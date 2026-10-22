#!/usr/bin/env python3
"""Compare native SHARC integer PCM and persistent state against DSP56303 translation."""
from __future__ import annotations
import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.trxbd_sharc.build_sharc import STATE_ADDR, VOICE_RESUME_SW
import fullblock
from digimd import MD_DEVICE, load_firmware
from digimd_trxbd_dsp import (TRXBDDSP56303Voice, from_firmware, build_dsp2_sine_table,
                              extract_trxbd_cf_tables, compute_trxbd_coefficients)
import vm as V
import vmrun

PATCHES = {
    'default': (64, 64, 0, 0, 0, 0, 0, 0),
    'all8': (40, 90, 80, 50, 70, 85, 60, 75),
    'min': (0,) * 8,
    'max': (127,) * 8,
    'noise': (64, 64, 0, 0, 0, 127, 0, 0),
    'harm_clip': (32, 80, 96, 48, 64, 32, 127, 127),
}


class HostVoice(C.Structure):
    _fields_ = [('y', C.c_int32 * 36), ('blk44', C.c_int32 * 32),
                ('prev_s24', C.c_int32), ('pos_int', C.c_uint32), ('pos_frac', C.c_uint32),
                ('last_p', C.c_uint32 * 8), ('active', C.c_uint32)]


def validate(section7: Path, build_report: dict, md: Path, blocks: int = 288) -> dict:
    # Exercise the real hook inside the stock block handler: direct C entry
    # calls do not reproduce its DAG/stack setup. Compare every persistent
    # voice byte and pre-AMP float, plus independently derived integer PCM.
    report_path = section7.parent/'kernel_parity.json'
    report_path.unlink(missing_ok=True)
    vm = V.VM()
    vm.load_dn2()
    stream = section7.read_bytes()
    if hashlib.sha256(stream).hexdigest() != build_report['spliced_section7_sha256']:
        raise ValueError('Section 7 does not match the build report')
    vmrun.load_stream(vm, stream)
    vm.call(vmrun.FXINIT)
    runner = vmrun.Runner(vm, {'PATTERN_VOL': 127})
    frame = runner.d.frame
    kernel = from_firmware(md)
    sine = build_dsp2_sine_table()
    tables = extract_trxbd_cf_tables(load_firmware(md, MD_DEVICE).section_bytes(0))
    cases = list(PATCHES.items()) + [('modulated_retrigger', PATCHES['all8'])]
    host_path = section7.parent / 'build/libtrxbd_parity.dylib'
    subprocess.run(['clang', '-O2', '-DTRXBD_HOST_TEST', '-dynamiclib', '-o', str(host_path),
                    str(ROOT / 'tools/trxbd_sharc/trxbd_sharc.c')], check=True)
    lib = C.CDLL(str(host_path.resolve()))
    hsine = (C.c_int32 * 32768)(*sine)
    voices = [HostVoice() for _ in cases]
    refs = [TRXBDDSP56303Voice(kernel, sine) for _ in cases]
    pcm = [[] for _ in cases]
    floats = [[] for _ in cases]
    params = [[x << 7 for x in syn] for _, syn in cases]
    rng = random.Random(0x56303)
    for t in range(16):
        struct.pack_into('<H', frame, 148 + 2*t, 5 if t < len(cases) else 4)
    for voice in voices:
        lib.trxbd_voice_init(C.byref(voice))
    expected = []
    def capture():
        for t, (hv, out) in enumerate(expected):
            addr = STATE_ADDR + 8 + t*320
            if vm.read(addr, 320) != bytes(hv):
                got = struct.unpack('<80I', vm.read(addr, 320))
                want = struct.unpack('<80I', bytes(hv))
                raise AssertionError(f'{cases[t][0]} block {b}: native state differs at {[(i, got[i], want[i]) for i in range(80) if got[i] != want[i]][:8]}')
            raw = vm.read(vm.rd32(0x254a60 + 4*t), 128)
            if raw != bytes(out):
                raise AssertionError(f'{cases[t][0]} block {b}: native 48k pre-AMP float differs')
            floats[t].extend(out[:])
    for b in range(blocks):
        expected.clear()
        trigger_mask = 0
        for t, ((name, _), hv, ref) in enumerate(zip(cases, voices, refs)):
            if name == 'modulated_retrigger' and b > 0 and b % 3 == 0:
                params[t] = [rng.randrange(0x3f81) for _ in range(8)]
            p = params[t]
            for off, val in zip((250, 252, 244, 248, 254, 256, 262, 260), p):
                struct.pack_into('<H', frame, off + 146*t, val << 1)
            trig = b == 0 or (name == 'modulated_retrigger' and b % 19 == 0)
            if list(hv.last_p) != p or trig:
                lib.trxbd_compute_coeffs((C.c_uint32 * 8)(*p), hv.y)
                hv.last_p[:] = p
                ref.load_coefficients(compute_trxbd_coefficients(p, *tables))
            if trig:
                trigger_mask |= 1 << t
                lib.trxbd_voice_trigger(C.byref(hv))
                ref.trigger()
                lib.trxbd_render_block_44k1(C.byref(hv), hsine)
                want = ref.render_block()
                if list(hv.blk44) != want:
                    raise AssertionError(f'{name} block {b}: host trigger PCM differs from DSP56303')
                pcm[t].extend(want)
                hv.prev_s24, hv.pos_int, hv.pos_frac = hv.blk44[0], 0, 0
            previous_pos = hv.pos_int
            out = (C.c_float * 32)()
            lib.trxbd_render_block_48k_float(C.byref(hv), hsine, out)
            if hv.pos_int < previous_pos:
                want = ref.render_block()
                if list(hv.blk44) != want:
                    raise AssertionError(f'{name} block {b}: host PCM differs from DSP56303')
                pcm[t].extend(want)
            if list(hv.y) != ref.y[:36]:
                raise AssertionError(f'{name} block {b}: host DSP state differs from DSP56303')
            expected.append((hv, out))
        for off in (34, 38, 40):
            struct.pack_into('<H', frame, off, trigger_mask)
        spi = vm.rd32(fullblock.SPI_HALF)
        vm.write(fullblock.RX_BASE + 0x1000*spi, bytes(frame))
        h = vm.rd32(fullblock.DMA_HALF)
        vm.write(fullblock.CODEC_IN + 0x100*h, b'\0'*256)
        runner._call_probed([(VOICE_RESUME_SW, capture)])
    results = []
    folders = dict(zip(PATCHES, ('trx_bd', 'trx_bd_all8', 'trx_bd_min', 'trx_bd_max', 'trx_bd_noise', 'trx_bd_harm_clip')))
    for (name, _), samples, output in zip(cases, pcm, floats):
        mdhost_samples = 0
        reference_sha256 = None
        if name in folders:
            reference = ROOT / 'out/pcm' / folders[name] / 'mdhost.raw'
            # Missing/stale independent evidence must never become a parity
            # PASS with zero compared samples.
            metadata = json.loads(reference.with_name('reference.json').read_text())
            raw = reference.read_bytes()
            reference_sha256 = hashlib.sha256(raw).hexdigest()
            if (not raw or len(raw) % 4 or reference_sha256 != metadata['reference_sha256']
                    or metadata['source_md_sha256'] != hashlib.sha256(md.read_bytes()).hexdigest()
                    or metadata['syn'] != list(PATCHES[name]) or metadata['sample_rate'] != 44100):
                raise AssertionError(f'{name}: independent reference is stale or incompatible')
            mdhost_samples = min(len(samples), len(raw) // 4)
            if struct.pack(f'<{mdhost_samples}i', *samples[:mdhost_samples]) != raw[:mdhost_samples*4]:
                raise AssertionError(f'{name}: native PCM differs from original MD DSP2 capture')
        result = {'case': name, 'internal_44k1_samples': len(samples),
                  'pre_amp_48k_samples': len(output), 'original_mdhost_samples_compared': mdhost_samples,
                  'original_mdhost_sha256': reference_sha256,
                  'pcm_and_state_bit_exact': True,
                  'pre_amp_float_bit_exact': True,
                  'pcm24_sha256': hashlib.sha256(struct.pack(f'<{len(samples)}i', *samples)).hexdigest()}
        results.append(result)
        print(json.dumps(result), flush=True)
    report = {'section7_sha256': hashlib.sha256(stream).hexdigest(), 'blocks': blocks, 'cases': results,
              'reference': 'Native whole-handler state/float vs host C; every internal integer block/state vs DSP56303 translation, independently verified against original MD DSP2'}
    report_path.write_text(json.dumps(report, indent=2)+'\n')
    return report


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--dir', type=Path, default=ROOT / 'out/trxbd_sharc')
    ap.add_argument('--md', type=Path, default=ROOT / 'firmware/Elektron_SPS1-1UW_OS1.63.syx')
    ap.add_argument('--blocks', type=int, default=288)
    args = ap.parse_args()
    result = validate(args.dir / 'section7_trxbd.bin', json.loads((args.dir / 'build_report.json').read_text()), args.md, args.blocks)
    (args.dir / 'kernel_parity.json').write_text(json.dumps(result, indent=2) + '\n')


if __name__ == '__main__':
    main()
