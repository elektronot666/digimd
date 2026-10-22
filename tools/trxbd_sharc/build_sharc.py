#!/usr/bin/env python3
"""Compile and splice the native SHARC+ TRX-BD voice engine into Digitone II OS 1.12 section 7."""
from __future__ import annotations

import argparse
import ctypes as C
import hashlib
import json
import math
from pathlib import Path
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
DN2_CFW = ROOT / 'out/external/dn2-cfw'
DIGIKIT = ROOT / 'out/external/digikit'
SELACHE_BIN = ROOT / 'out/external/selache/target/release'

for p in (
    ROOT,
    DN2_CFW / 'tools/dspsplice',
    DN2_CFW / 'tools/sharcvm',
    DN2_CFW / 'tools/brickcheck',
    DN2_CFW / 'scripts',
    DIGIKIT / 'tools',
    DIGIKIT,
):
    if str(p) not in sys.path:
        sys.path.insert(0, str(p))

from digimd import DNII_DEVICE, DNII_SHA256, MD_DEVICE, MD_SHA256, load_firmware  # noqa: E402
from digimd_trxbd_dsp import build_dsp2_sine_table, render_trxbd_pcm24  # noqa: E402
import bc_dsp  # noqa: E402
import build as splice_build  # noqa: E402
import elf as E  # noqa: E402
import fullblock  # noqa: E402
import ldr  # noqa: E402
import stub  # noqa: E402
import vm as V  # noqa: E402
import vmrun  # noqa: E402
from tools.md_runtime import build as runtime_build  # noqa: E402

CODE_RANGE = (0x002DD600, 0x002E6000)   # L1 Block 1 free tail (below 0x2E8000 DM cache)
SAVE_ADDR = 0x002E6800                  # Hook register save area (108 bytes in L1 Block 1)
STATE_ADDR = 0x00300000                 # L1 Block 2 (below 0x318000 PM cache)
STATE_BYTES = 0x00004000                # 16 KB zeroed FILL for TrxBdState
DATA_RANGE = (0x80540000, 0x80550000)   # DDR data tables (above stock DDR 0x80530000)
SINE_ADDR = 0x80550000                  # DDR 32,768-word DSP2 sine table
SINE_BYTES = 0x00020000                 # 128 KB zeroed FILL

CHORUS_INIT_CALL_SW = 0x1C1695
CHORUS_INIT_SW = 0xB84F51
VOICE_HOOK_SW = 0x1C9448
VOICE_RESUME_SW = 0x1C944C
VOICE_HOOK_STOCK = bytes.fromhex('089ce80a089c5e05')
MACHINE_LOOKUP_5_DM = 0x0025D75C        # 0x25d748[5]

LDF_TEMPLATE = """ARCHITECTURE(ADSP-21569)
$OBJECTS = $COMMAND_LINE_OBJECTS;
MEMORY
{{
   mem_trxbd_code {{ TYPE(BW RAM) START(0x{c0:08x}) END(0x{c1:08x}) WIDTH(8) }}
   mem_trxbd_data {{ TYPE(BW RAM) START(0x{d0:08x}) END(0x{d1:08x}) WIDTH(8) }}
}}
PROCESSOR p0
{{
   OUTPUT($COMMAND_LINE_OUTPUT_FILE)
   ENTRY(trxbd_voice_stub.)
   SECTIONS
   {{
      trxbd_code SW
      {{
         INPUT_SECTION_ALIGN(2)
         FILL(0x1)
         INPUT_SECTIONS( $OBJECTS(seg_swco seg_pmco) )
      }} > mem_trxbd_code
      trxbd_data BW
      {{
         INPUT_SECTION_ALIGN(4)
         INPUT_SECTIONS( $OBJECTS(seg_dmda seg_l2) )
      }} > mem_trxbd_data
      trxbd_bss NO_INIT BW
      {{
         INPUT_SECTION_ALIGN(4)
         INPUT_SECTIONS( $OBJECTS(seg_l1_bss seg_bss) )
      }} > mem_trxbd_data
   }}
}}
"""


def generate_stubs_asm(save_addr: int, dual_engine: bool = False) -> str:
    """Generate SHARC+ assembly stubs for trxbd_init_stub and trxbd_voice_stub."""
    init_stub = stub.gen(
        'trxbd_init_stub',
        [('c', 'md_dual_init' if dual_engine else 'trxbd_init', []), ('tail', CHORUS_INIT_SW)],
        save_addr,
    )
    voice_raw = stub.gen(
        'trxbd_voice_stub',
        [('c', 'md_dual_process' if dual_engine else 'trxbd_process', []), ('tail', VOICE_RESUME_SW)],
        save_addr,
    )
    tail_jmp = f'    JUMP 0x{VOICE_RESUME_SW:X};'
    displaced_seq = '\n'.join([
        '    I5 = DM(-0x18, I6);',
        '    R10 = DM(-0x22, I6);',
        tail_jmp,
    ])
    if tail_jmp not in voice_raw:
        raise RuntimeError('stub.gen tail jump did not match expected template')
    voice_stub = voice_raw.replace(tail_jmp, displaced_seq)
    return '\n'.join(['.SECTION/SW seg_swco;', '', init_stub, voice_stub, '.ENDSEG;', ''])


def jump_nop_bytes(target_sw: int) -> bytes:
    """Encode 48-bit Type 8a `JUMP target;` + 16-bit Type 1a `NOP;` (8 bytes total)."""
    raw = (0x063E << 32) | (target_sw & 0xFFFFFF)
    parcels = [(raw >> 32) & 0xFFFF, (raw >> 16) & 0xFFFF, raw & 0xFFFF, 0x0001]
    return b''.join(struct.pack('<H', p) for p in parcels)


def build_section7(
    dn2_syx: Path,
    out_dir: Path,
    *,
    diagnostic_silence: bool = False,
    dual_engine: bool = False,
    generic_machines: bool = False,
    layout: str = 'l1',
    experimental_source: Path | None = None,
    experimental_math_source: Path | None = None,
) -> dict:
    out_dir = out_dir.resolve()
    if generic_machines:
        if not dual_engine or diagnostic_silence or experimental_source or experimental_math_source:
            raise ValueError('Generic build requires tracked dual engines')
        from tools.machines.build import prepare, validate_layout
        code_range = validate_layout(layout)
    else:
        if layout != 'l1':
            raise ValueError('Explicit layouts are supported by the generic builder')
        code_range = CODE_RANGE
    if dual_engine and (diagnostic_silence or experimental_source or experimental_math_source):
        raise ValueError("Dual production engines require original tracked sources")
    fw = load_firmware(dn2_syx, DNII_DEVICE)
    if fw.digest != DNII_SHA256:
        raise ValueError(f'unexpected DNII SHA-256: {fw.digest}')
    stock7 = fw.section_bytes(7)

    bdir = out_dir / 'build'
    bdir.mkdir(parents=True, exist_ok=True)
    (out_dir / 'build_report.json').unlink(missing_ok=True)
    src_c = ROOT / 'tools/trxbd_sharc/trxbd_sharc.c'
    diagnostic_c = ROOT / 'tools/trxbd_sharc/diagnostic_silence.c'
    if experimental_source is not None:
        if diagnostic_silence:
            raise ValueError('Cannot combine silence and experimental source builds')
        experimental_source = experimental_source.resolve(strict=True)
        if not experimental_source.is_relative_to(ROOT / 'out'):
            raise ValueError('Experimental sources must reside under ignored out/, never replace production C')
        if experimental_source.suffix != '.c':
            raise ValueError('Experimental renderer source must be a C file')
    compile_c = (diagnostic_c if diagnostic_silence
                 else experimental_source if experimental_source is not None else src_c)
    if generic_machines:
        prepare(bdir)
        compile_c = bdir / 'trxbd_generic.c'
    inc_dir = ROOT / 'tools/trxbd_sharc'

    subprocess.run(
        [
            str(SELACHE_BIN / 'selcc'),
            '-proc', 'ADSP-21569',
            '-char-size-8',
            '-O1',
            f'-I{inc_dir}',
            '-S',
            '-o', str(bdir / 'trxbd_sharc.s'),
            str(compile_c),
        ],
        check=True,
    )
    if dual_engine:
        compiled = bdir / 'trxbd_sharc.s'
        text = compiled.read_text()
        marker = '.VAR trxbd_t_decay. ='
        if text.count(marker) != 1:
            raise ValueError('Missing original shared decay table')
        text = text.replace('trxbd_t_decay.', 'md_decay_table.')
        compiled.write_text(text.replace('.VAR md_decay_table. =', '.GLOBAL md_decay_table.;\n.VAR md_decay_table. ='))
    subprocess.run(
        [
            str(SELACHE_BIN / 'selas'),
            '-proc', 'ADSP-21569',
            '-o', str(bdir / 'trxbd_sharc.doj'),
            str(bdir / 'trxbd_sharc.s'),
        ],
        check=True,
    )

    extra_objects = []
    extra_sources = []
    compiled_names = []
    if dual_engine:
        compile_units = [(name, ROOT / 'tools/trxsd' / (name + '.c')) for name in ('trxsd', 'dual')]
        if generic_machines:
            from tools.machines.registry import machines
            compile_units = [(m['native']['prefix'], bdir/(m['native']['prefix']+'_generic.c'))
                             for m in machines() if m['native']['enabled'] and m['native']['prefix'] != 'trxbd']
            compile_units.append(('dispatch', bdir/'dispatch_generic.c'))
        for name, source in compile_units:
            asm = bdir / (name + '.s')
            obj = bdir / (name + '.doj')
            subprocess.run([str(SELACHE_BIN / 'selcc'), '-proc', 'ADSP-21569',
                            '-char-size-8', '-O1', '-S', '-o', str(asm), str(source)], check=True)
            subprocess.run([str(SELACHE_BIN / 'selas'), '-proc', 'ADSP-21569',
                            '-o', str(obj), str(asm)], check=True)
            extra_objects.append(str(obj))
            extra_sources.append(source)
            compiled_names.append(name)
        extra_sources.extend([ROOT / 'tools/trxsd/trxsd_tables.h', ROOT / 'tools/trxsd/filter.s'])

    math_src = ROOT / 'tools/trxbd_sharc/math_leaf.s' 
    if experimental_math_source is not None:
        experimental_math_source = experimental_math_source.resolve(strict=True)
        if not experimental_math_source.is_relative_to(ROOT / 'out'):
            raise ValueError('Experimental math helpers must reside under ignored out/')
        if experimental_math_source.suffix != '.s':
            raise ValueError('Experimental SHARC helper source must be .s assembly')
    assemble_math_src = experimental_math_source if experimental_math_source is not None else math_src
    if experimental_math_source is None:
        # Specialize layout at build time and assemble shared leaves exactly once.
        assemble_math_src = bdir / 'native_helpers.s'
        boundary = '.GLOBAL trxbd_osc_shape.;'
        engine_math = math_src.read_text()
        if engine_math.count(boundary) != 1:
            raise ValueError('TRX-BD assembly layout boundary missing or duplicated')
        before, after = engine_math.split(boundary)
        if generic_machines:
            active = [m['native'] for m in machines() if m['native']['enabled']]
            assemble_math_src.write_text(runtime_build.assembly(before, boundary+after))
            for n in active:
                units = [(n['prefix']+'_resample', runtime_build.resampler_asm(n['resampler_symbol'], n['stream_byte_offset']))]
                units += [(n['prefix']+'_helper_'+str(i), (ROOT/p).read_text())
                          for i,p in enumerate(n['assembly_sources']) if n['prefix']!='trxbd']
                for name, body in units:
                    assembly_path = bdir/(name+'.s')
                    assembly_path.write_text('.SECTION/SW seg_swco;\n'+body.replace('.SECTION/SW seg_swco;','').replace('.ENDSEG;','')+'\n.ENDSEG;\n')
                    obj = bdir/(name+'.doj')
                    subprocess.run([str(SELACHE_BIN/'selas'),'-proc','ADSP-21569','-o',str(obj),str(assembly_path)],check=True)
                    extra_objects.append(str(obj))
                    compiled_names.append(name)
        else:
            assemble_math_src.write_text(runtime_build.assembly(
                before, runtime_build.resampler_asm('trxbd_resample_run', 144), boundary + after,
                runtime_build.resampler_asm('trxsd_resample_run', 120) + '\n' + (ROOT / 'tools/trxsd/filter.s').read_text() if dual_engine else ''))
    subprocess.run([str(SELACHE_BIN / 'selas'), '-proc', 'ADSP-21569',
                    '-o', str(bdir / 'math_leaf.doj'), str(assemble_math_src)], check=True)
    stubs_s = generate_stubs_asm(SAVE_ADDR, True) if dual_engine else generate_stubs_asm(SAVE_ADDR)
    if generic_machines:
        from tools.machines.build import lookup_stub
        stubs_s = stubs_s.replace('md_dual_init.', 'md_generic_init.').replace('md_dual_process.', 'md_generic_process.')
        stubs_s += lookup_stub()
    (bdir / 'stubs.s').write_text(stubs_s)
    subprocess.run(
        [
            str(SELACHE_BIN / 'selas'),
            '-proc', 'ADSP-21569',
            '-o', str(bdir / 'stubs.doj'),
            str(bdir / 'stubs.s'),
        ],
        check=True,
    )

    ldf = LDF_TEMPLATE.format(
        c0=code_range[0],
        c1=code_range[1] - 1,
        d0=DATA_RANGE[0],
        d1=DATA_RANGE[1] - 1,
    )
    (bdir / 'trxbd.ldf').write_text(ldf)
    dxe_path = bdir / 'trxbd.dxe'
    subprocess.run(
        [
            str(SELACHE_BIN / 'seld'),
            '-proc', 'ADSP-21569',
            '-T', str(bdir / 'trxbd.ldf'),
            *(['-Map', str(bdir / 'linker.map')] if generic_machines else []),
            '-o', str(dxe_path),
            str(bdir / 'stubs.doj'),
            str(bdir / 'trxbd_sharc.doj'),
            str(bdir / 'math_leaf.doj'),
            *extra_objects,
        ],
        check=True,
    )

    # Check register census
    asm_text = (bdir / 'trxbd_sharc.s').read_text()
    rc = splice_build.register_census(asm_text + '\n' + assemble_math_src.read_text() +
        ''.join((bdir / (name + '.s')).read_text() for name in compiled_names) if dual_engine else asm_text + '\n' + assemble_math_src.read_text())
    disallowed = set(rc) - splice_build.CENSUS_ALLOWED
    if disallowed:
        raise RuntimeError(f'selcc assembly writes disallowed registers: {sorted(disallowed)}')

    elf = E.read_elf(dxe_path)
    syms = {k.rstrip('.'): v for k, v in elf.symbols.items()}
    init_stub_sw = syms['trxbd_init_stub']
    voice_stub_sw = syms['trxbd_voice_stub']

    stream = ldr.Stream(stock7)
    # 1. Enable machine type 5 in unpack lookup table 0x25d748[5]
    if generic_machines:
        # Replace the lookup boundary itself; never extend the stock table.
        stream.patch(E.sw_to_byte(0x1c2958), bytes.fromhex('150f250048d73e703f92'),
                     jump_nop_bytes(syms['md_safe_lookup']) + b'\x01\x00',
                     'bounded lookup: verified 0..6 unchanged; unavailable/invalid raw identities use silent custom sentinel')
    else:
        stream.patch(
        MACHINE_LOOKUP_5_DM,
        struct.pack('<I', 0),
        struct.pack('<I', 5),
        'enable machine type 5 in 0x25d748[5]',
    )
    if dual_engine and not generic_machines:
        stream.patch(MACHINE_LOOKUP_5_DM + 4, struct.pack('<I', 0),
                     struct.pack('<I', 6), 'enable machine type 6 in 0x25d748[6]')
    # 2. Hook effect init 0x1c1695 -> trxbd_init_stub (tail-jumps to stock chorus init 0xb84f51)
    stream.patch(
        E.sw_to_byte(CHORUS_INIT_CALL_SW),
        splice_build.cjump_bytes(CHORUS_INIT_SW),
        splice_build.cjump_bytes(init_stub_sw),
        'hook 0x1c1695 -> trxbd_init_stub',
    )
    # 3. Hook voice dispatch 0x1c9448 -> trxbd_voice_stub (resumes at 0x1c944c)
    stream.patch(
        E.sw_to_byte(VOICE_HOOK_SW),
        VOICE_HOOK_STOCK,
        jump_nop_bytes(voice_stub_sw),
        'hook 0x1c9448 -> trxbd_voice_stub',
    )

    sections_info = []
    for addr, n, sec in splice_build.ranges_of(elf):
        if not n:
            continue
        if sec.kind == 'bss':
            stream.add_fill(addr, (n + 3) // 4 * 4, f'bss {sec.name}')
        else:
            stream.add_payload(addr, sec.data, f'{sec.kind} {sec.name}')
        sections_info.append({'name': sec.name, 'kind': sec.kind, 'core_addr': f'0x{addr:08x}', 'bytes': n})

    save_bytes = (stub.SAVE_BYTES + 3) // 4 * 4
    stream.add_fill(SAVE_ADDR, save_bytes, 'hook register save area')
    stream.add_fill(STATE_ADDR, 5384 if generic_machines else STATE_BYTES,
                    'single guarded MD 16-slot pool' if generic_machines else 'TRX-BD 16-voice state in L1 block 2')
    if dual_engine and not generic_machines:
        stream.add_fill(0x00304000, 0x4000, 'TRX-SD 16-slot private state')
    stream.add_fill(SINE_ADDR, SINE_BYTES, 'DSP2 32,768-word sine table in DDR')

    out_bytes = stream.to_bytes(fix_first=True)
    if generic_machines and len(out_bytes) > 0xfffff:
        raise ValueError('Generic SHARC bootstream exceeds the OS 1.12 decompressed loader cap')
    ok_ldr, bad_ldr, _ = ldr.check_stream(out_bytes)
    if not ok_ldr:
        raise RuntimeError(f'spliced boot stream failed ldr check: {bad_ldr}')

    # Run bc_dsp static checks (simulated stored7 <= 1 MiB)
    sb = bc_dsp.scan_sizes_and_blocks(out_bytes, out_bytes)
    if not (sb['parse_ok'] and sb['final'] and not sb['cache_hits'] and not sb['region_miss']):
        raise RuntimeError(f'bc_dsp static check failed: {sb}')

    cand_img, blocks, _, _ = bc_dsp.final_image(out_bytes)
    stock_img, _, _, _ = bc_dsp.final_image(stock7)
    bad_shifts = bc_dsp.scan_bad_shifts(cand_img, stock_img, blocks)
    if bad_shifts:
        raise RuntimeError(f'bc_dsp scan_bad_shifts found bad shift encodings: {bad_shifts}')

    sec7_out = out_dir / 'section7_trxbd.bin'
    sec7_out.write_bytes(out_bytes)

    report = {
        'schema': 'digimd.sharc-trxbd-splice.v1',
        'dual_engine': dual_engine,
        'generic_machines': generic_machines,
        'layout': layout,
        'diagnostic_only': diagnostic_silence or experimental_source is not None or experimental_math_source is not None,
        'diagnostic_mode': ('silence-custom-renderer' if diagnostic_silence
                            else 'experimental-source-benchmark' if experimental_source is not None
                            else 'experimental-math-benchmark' if experimental_math_source is not None
                            else None),
        'experimental_source_override': str(experimental_source.relative_to(ROOT))
                                        if experimental_source is not None else None,
        'experimental_math_override': str(experimental_math_source.relative_to(ROOT))
                                      if experimental_math_source is not None else None,
        'source_sha256': {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest()
                          for p in (Path(__file__), src_c, inc_dir/'trxbd_tables.h', math_src,
                                    *runtime_build.sources(), *extra_sources,
                                    *([diagnostic_c] if diagnostic_silence else []),
                                    *([experimental_source] if experimental_source is not None else []),
                                    *([experimental_math_source] if experimental_math_source is not None else []))},
        'compiled_helpers_sha256': hashlib.sha256(assemble_math_src.read_bytes()).hexdigest(),
        'stock_dn2_sha256': fw.digest,
        'stock_section7_sha256': hashlib.sha256(stock7).hexdigest(),
        'spliced_section7_sha256': hashlib.sha256(out_bytes).hexdigest(),
        'spliced_section7_bytes': len(out_bytes),
        'symbols': {k: f'0x{v:08x}' for k, v in sorted(syms.items())},
        'sections': sections_info,
        'patches': stream.patches,
        'added_blocks': stream.added,
        'static_checks': {
            'ldr_check_ok': ok_ldr,
            'cache_hits': sb['cache_hits'],
            'region_miss': sb['region_miss'],
            'bad_shifts': bad_shifts,
        },
    }
    if generic_machines:
        from tools.machines.build import finish
        finish(report, bdir, elf, stock7, code_range)
    (out_dir / 'build_report.json').write_text(json.dumps(report, indent=2))
    return report


def set_frame_u16(frame: bytearray, byte_off: int, val: int) -> None:
    """Write a 16-bit parameter into fullblock.FullDSP's raw SPI frame (which is 16-bit word-swapped inside each 32-bit word)."""
    # Note: the SHARC unpacker sw 0x1c2712 copies the SPI RX buffer (where each 32-bit word has its
    # two 16-bit halves swapped relative to the ColdFire's big-endian wire) into 0x25c48c.
    # Let us check how FullDSP.frame maps to 0x25c48c!
    struct.pack_into('<H', frame, byte_off, val & 0xFFFF)


def verify_in_sharcvm(
    md_syx: Path,
    sec7_path: Path,
    out_dir: Path,
    blocks_n: int = 32,
) -> dict:
    stream = sec7_path.read_bytes()
    (out_dir / 'sharcvm_verify.json').unlink(missing_ok=True)
    vm = V.VM()
    vm.load_dn2()
    vmrun.load_stream(vm, stream)

    # 1. Run FXINIT (0x1c1445), which triggers trxbd_init_stub -> trxbd_init
    vm.call(vmrun.FXINIT, max_insns=10**9)

    magic = vm.rd32(STATE_ADDR)
    if magic != 0x54525842:
        raise RuntimeError(f'expected TRXBD_MAGIC 0x54525842 at 0x300000 after FXINIT, got 0x{magic:08x}')

    # 2. Verify all 32,768 words of the DSP2 sine table at 0x80550000 in SHARC DDR
    sine_bytes = vm.read(SINE_ADDR, 32768 * 4)
    sharc_sine = struct.unpack('<32768i', sine_bytes)
    ref_sine = build_dsp2_sine_table()
    if sharc_sine != ref_sine:
        diffs = sum(1 for a, b in zip(sharc_sine, ref_sine) if a != b)
        raise RuntimeError(f'SHARC DDR sine table mismatch: {diffs} words differ')

    # 3. sw 0x1c2712 copies RX_BASE (d.frame) to 0x25c48c verbatim (672 32-bit words).
    def write_frame_halfword(frame: bytearray, unpacked_off: int, val16: int) -> None:
        struct.pack_into('<H', frame, unpacked_off, val16 & 0xFFFF)

    runner = vmrun.Runner(vm, {'PATTERN_VOL': 127})
    frame = runner.d.frame

    # Configure Track 0 as machine type 5 (TRX-BD), filter type 0 (Multimode, open cutoff 0x7f00),
    # track level 100 (0x6400), AMP volume 100 (0x6400), AMP decay 127 (0x7f00).
    # Set Tracks 1..15 to machine type 4 (MIDI, silent) so Track 0 is isolated.
    write_frame_halfword(frame, 148 + 2 * 0, 5)
    for t in range(1, 16):
        write_frame_halfword(frame, 148 + 2 * t, 4)

    # Set default TRX-BD SYN knobs on Track 0: PTCH=64, DEC=64, RAMP=0, RDEC=0, STRT=0, NOIS=0, HARM=0, CLIP=0
    param_offsets = (250, 252, 244, 248, 254, 256, 262, 260)
    default_syn = (64, 64, 0, 0, 0, 0, 0, 0)
    for off, val in zip(param_offsets, default_syn):
        write_frame_halfword(frame, off, val << 8)

    # Also open Track 0's filter and AMP in the frame so post-AMP output is audible:
    # Track level at 116: 0x6400; FREQ (67 -> 302): 0x7f00; F.ENV (69 -> 306): 0x4000;
    # WIDTH (77 -> 322): 0x7f00; AMP HOLD (82 -> 332): 0x7f00; AMP DEC (83 -> 334): 0x7f00;
    # AMP SUS (84 -> 336): 0x7f00; AMP REL (85 -> 338): 0x7f00; PAN (89 -> 346): 0x4000;
    # VOL (90 -> 348): 0x6400; AMP RST (92 -> 352): 0x0100.
    write_frame_halfword(frame, 116, 0x6400)
    write_frame_halfword(frame, 302, 0x7F00)
    write_frame_halfword(frame, 306, 0x4000)
    write_frame_halfword(frame, 322, 0x7F00)
    write_frame_halfword(frame, 332, 0x7F00)
    write_frame_halfword(frame, 334, 0x7F00)
    write_frame_halfword(frame, 336, 0x7F00)
    write_frame_halfword(frame, 338, 0x7F00)
    write_frame_halfword(frame, 346, 0x4000)
    write_frame_halfword(frame, 348, 0x6400)
    write_frame_halfword(frame, 352, 0x0100)

    # Trigger Track 0 on block 0 (note + filter + AMP trigger masks at offsets 34, 38, 40)
    for trig_off in (34, 38, 40):
        write_frame_halfword(frame, trig_off, 0x0001)

    pre_amp_f32: list[float] = []
    post_amp_L: list[float] = []
    post_amp_R: list[float] = []
    blk44_captured: list[int] = []
    last_pos_int = -1

    # Probe right at 0x1c944c (immediately after trxbd_voice_stub finishes writing Track 0's
    # pre-filter/pre-AMP buffer at DM(0x254a60) = 0x804acf90, BEFORE the DNII filter & AMP run!)
    def capture_pre_amp() -> None:
        buf_ptr = vm.rd32(0x00254A60)
        pre_amp_f32.extend(vm.rdf(buf_ptr, 32))
        # Also read Track 0's internal 44.1 kHz DSP56303 block from TrxBdVoice at STATE_ADDR + 8
        # TrxBdVoice layout: y[36] (144 B), blk44[32] (128 B at +144), prev_s24 (+272), pos_int (+276)
        v_addr = STATE_ADDR + 8
        pos_int = vm.rd32(v_addr + 276)
        nonlocal last_pos_int
        if pos_int < last_pos_int or last_pos_int < 0:
            raw_blk = vm.read(v_addr + 144, 32 * 4)
            blk44_captured.extend(struct.unpack('<32i', raw_blk))
        last_pos_int = pos_int

    for b_idx in range(blocks_n):
        if b_idx == 1:
            # Clear trigger bits after block 0
            for trig_off in (34, 38, 40):
                write_frame_halfword(frame, trig_off, 0x0000)
        spi = vm.rd32(fullblock.SPI_HALF)
        vm.write(fullblock.RX_BASE + 0x1000 * spi, bytes(frame))
        h = vm.rd32(fullblock.DMA_HALF)
        vm.write(fullblock.CODEC_IN + 0x100 * h, b'\x00' * 256)
        n0 = vm.icount
        runner._call_probed([(VOICE_RESUME_SW, capture_pre_amp)])
        runner.per_block.append(vm.icount - n0)
        post_amp_L.extend(vm.rdf(fullblock.OUT_FLOATS, 32))
        post_amp_R.extend(vm.rdf(fullblock.OUT_FLOATS + 0x80, 32))

    # Compare captured 44.1 kHz 24-bit blocks from SHARC L1 against original MD DSP2 reference!
    ref_44k = render_trxbd_pcm24(md_syx, syn=default_syn, blocks=len(blk44_captured) // 32)
    if blk44_captured != ref_44k:
        diffs = sum(1 for a, b in zip(blk44_captured, ref_44k) if a != b)
        raise RuntimeError(f'SHARC internal 44.1 kHz 24-bit PCM differed from MD reference on {diffs} samples')

    # Compare captured 48 kHz pre-AMP float buffer against host C reference!
    host_lib_path = out_dir / 'build/libtrxbd_host.dylib'
    subprocess.run(
        ['clang', '-O2', '-DTRXBD_HOST_TEST', '-dynamiclib', '-o', str(host_lib_path), str(ROOT / 'tools/trxbd_sharc/trxbd_sharc.c')],
        check=True,
    )
    hlib = C.CDLL(str(host_lib_path))

    class HostVoice(C.Structure):
        _fields_ = [
            ('y', C.c_int32 * 36),
            ('blk44', C.c_int32 * 32),
            ('prev_s24', C.c_int32),
            ('pos_int', C.c_uint32),
            ('pos_frac', C.c_uint32),
            ('last_p', C.c_uint32 * 8),
            ('active', C.c_uint32),
        ]

    hsine = (C.c_int32 * 32768)(*ref_sine)
    hv = HostVoice()
    hlib.trxbd_voice_init(C.byref(hv))
    hp = (C.c_uint32 * 8)(*[x << 7 for x in default_syn])
    hlib.trxbd_compute_coeffs(hp, hv.y)
    hlib.trxbd_voice_trigger(C.byref(hv))
    hlib.trxbd_render_block_44k1(C.byref(hv), hsine)
    hv.prev_s24 = hv.blk44[0]
    hv.pos_int = 0
    hv.pos_frac = 0
    host_48k: list[float] = []
    out_f = (C.c_float * 32)()
    for _ in range(blocks_n):
        hlib.trxbd_render_block_48k_float(C.byref(hv), hsine, out_f)
        host_48k.extend(out_f[:])

    max_err_48k = max(abs(a - b) for a, b in zip(pre_amp_f32, host_48k))
    if max_err_48k > 1e-6:
        raise RuntimeError(f'SHARC 48 kHz pre-AMP float differed from host reference (max_err={max_err_48k})')

    pre_peak = max(abs(x) for x in pre_amp_f32)
    pre_nonzero = sum(1 for x in pre_amp_f32 if x != 0.0)
    post_peak = max(abs(x) for x in post_amp_L + post_amp_R)
    nan_count = sum(1 for x in pre_amp_f32 + post_amp_L + post_amp_R if math.isnan(x) or math.isinf(x))
    if nan_count != 0 or pre_peak <= 0.1:
        raise RuntimeError(f'invalid SHARC audio: nan_count={nan_count}, pre_peak={pre_peak}')

    pre_raw = struct.pack(f'<{len(pre_amp_f32)}f', *pre_amp_f32)
    (out_dir / 'sharc_pre_amp_48k_f32.raw').write_bytes(pre_raw)

    verify_rep = {
        'section7_sha256': hashlib.sha256(stream).hexdigest(),
        'blocks_run': blocks_n,
        'sine_table_32768_bit_exact': True,
        'internal_44k1_samples_compared': len(blk44_captured),
        'internal_44k1_bit_exact_with_md_dsp2': True,
        'pre_amp_48k_samples': len(pre_amp_f32),
        'pre_amp_48k_sha256': hashlib.sha256(pre_raw).hexdigest(),
        'pre_amp_48k_max_error_vs_host_c': max_err_48k,
        'pre_amp_peak': round(pre_peak, 6),
        'pre_amp_nonzero': pre_nonzero,
        'post_amp_master_peak': round(post_peak, 6),
        'nan_or_inf_samples': nan_count,
        'insns_per_block_min': min(runner.per_block),
        'insns_per_block_median': sorted(runner.per_block)[len(runner.per_block) // 2],
        'insns_per_block_max': max(runner.per_block),
        'stock_insns_per_block_baseline': bc_dsp.STOCK_INSN_PER_BLOCK,
        'instruction_ceiling_666667_ok': max(runner.per_block) < 666667,
        'measurement': 'Interpreter instructions; no cache/DDR/IRQ timing model',
        'physical_cycles_measured': False,
    }
    (out_dir / 'sharcvm_verify.json').write_text(json.dumps(verify_rep, indent=2))
    return verify_rep


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--dn2', type=Path, default=ROOT / 'firmware/Digitone_II_OS1.12.syx')
    ap.add_argument('--md', type=Path, default=ROOT / 'firmware/Elektron_SPS1-1UW_OS1.63.syx')
    ap.add_argument('--out-dir', type=Path, default=ROOT / 'out/trxbd_sharc')
    ap.add_argument('--verify', action='store_true')
    ap.add_argument('--blocks', type=int, default=32)
    args = ap.parse_args()

    args.out_dir.mkdir(parents=True, exist_ok=True)
    rep = build_section7(args.dn2, args.out_dir)
    print(f"Built {args.out_dir / 'section7_trxbd.bin'} ({rep['spliced_section7_bytes']} B, sha256={rep['spliced_section7_sha256']})")
    if args.verify:
        vrep = verify_in_sharcvm(args.md, args.out_dir / 'section7_trxbd.bin', args.out_dir, blocks_n=args.blocks)
        print('SHARCVM VERIFIED:', json.dumps(vrep, indent=2))
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
