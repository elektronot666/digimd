"""Native gate for the SHARC+ fast render: executed in sharcvm against the
original-firmware recordings at 44.1 kHz and through the 48 kHz resampler.

The engine's generated C keeps init/trigger/coefficients; its C render
definition is renamed and the assembly render is linked in its place, so the
48 kHz path (render48.inc -> MD_RENDER44) also executes the new code.
"""
import argparse
import json
import statistics
import struct
import subprocess
from pathlib import Path

from tools.machines.registry import ROOT
from tools.machines.translation import fast_check as F
from tools.machines.translation import native as N
from tools.machines.translation.fast_sharc import compile_function
from tools.md_runtime.build import assembly, resampler_asm
from tools.trxbd_sharc.build_sharc import SELACHE_BIN, LDF_TEMPLATE, DATA_RANGE, splice_build
import vm as V

OUT = F.OUT
TEST_SOURCE_BASE = 0x80600000


def layout(m):
    words = m['original_slot_words']
    stream = m['stream_byte_offset']
    lay = dict(blk_byte=stream, fault_byte=m.get('memory_fault_byte_offset', words * 4 + 176))
    if m.get('private_memory'):
        base = words * 4 + 180
        lay.update(x_private_byte=base, external_byte=base + 256, virtual_voice_byte=base + 256 + 6144,
                   scratch_y20_history_byte=base + 256 + 6144 + 4)
    return lay


def fast_source(m, instructions, img, source_base):
    """Assembly for the specialized render (and its unguarded fast fallback)."""
    machine, ops = F.lift(m, instructions, img)
    p = m['prefix']
    texts = []
    generic = None
    if machine.generic_ops is not None:
        generic = p + '_render_block_44k1_g'
        g, _ = compile_function(generic, machine, machine.generic_ops, layout(m), source_base,
                                fallback=p + '_render_block_44k1_c')
        texts.append(g)
    from tools.machines.translation.fast_profile import hints
    text, e = compile_function(p + '_render_block_44k1', machine, ops, layout(m), source_base,
                               fallback=p + '_render_block_44k1_c', generic=generic, hints=hints(m, machine, ops))
    texts.insert(0, text)
    fast_source.tags = e.tags
    return texts, e, machine, ops


def renamed_engine(m):
    text = (ROOT / m['source']).read_text()
    old = f'void {m["prefix"]}_render_block_44k1({m["voice_type"]} *v, const int32_t *sine) {{'
    assert text.count(old) == 1, m['name']
    text = text.replace(old, f'extern void {m["prefix"]}_render_block_44k1({m["voice_type"]} *v, const int32_t *sine);\n'
                        f'void {m["prefix"]}_render_block_44k1_c({m["voice_type"]} *v, const int32_t *sine) {{')
    for rel in ('../md_runtime_compat.h', '../../md_runtime/render48.inc', '../engine_slot.inc'):
        text = text.replace(f'"{rel}"', '"' + str((ROOT / m['source']).parent.joinpath(rel).resolve()) + '"')
    return text


def build(m, folder, instructions, img):
    p = m['prefix']
    folder.mkdir(parents=True, exist_ok=True)
    (folder / 'engine_src.c').write_text(renamed_engine(m))
    (folder / 'engine.c').write_text(f'#define MD_SOURCE_WORDS_BASE 0x{TEST_SOURCE_BASE:x}u\n#include "{folder / "engine_src.c"}"\n'
                                     f'const uint32_t translated_voice_bytes = sizeof({m["voice_type"]});\n')
    asms, e, machine, ops = fast_source(m, instructions, img, TEST_SOURCE_BASE)
    fast = []
    (folder / 'fast0.tags.json').write_text(json.dumps(e.tags))
    for n, asm in enumerate(asms):
        (folder / f'fast{n}.s').write_text(asm)
        fast.append(folder / f'fast{n}')
    (folder / 'helpers.s').write_text(assembly(resampler_asm(p + '_resample_run', m['stream_byte_offset'])))
    ldf = LDF_TEMPLATE.format(c0=0x20040000, c1=0x200f8fff, d0=DATA_RANGE[0], d1=DATA_RANGE[1] - 1).replace(
        'ENTRY(trxbd_voice_stub.)', f'ENTRY({p}_voice_init.)')
    (folder / 'engine.ldf').write_text(ldf)
    commands = [['selcc', '-proc', 'ADSP-21569', '-char-size-8', '-O1', '-S', '-o', folder / 'engine.s', folder / 'engine.c'],
                ['selcc', '-proc', 'ADSP-21569', '-char-size-8', '-O1', '-S', '-o', folder / 'control_tables.s',
                 ROOT / 'tools/machines/generated/control_tables.c'],
                ['selas', '-proc', 'ADSP-21569', '-o', folder / 'control_tables.doj', folder / 'control_tables.s'],
                ['selas', '-proc', 'ADSP-21569', '-o', folder / 'engine.doj', folder / 'engine.s'],
                ['selas', '-proc', 'ADSP-21569', '-o', folder / 'helpers.doj', folder / 'helpers.s'],
                *[['selas', '-proc', 'ADSP-21569', '-o', f.with_suffix('.doj'), f.with_suffix('.s')] for f in fast],
                ['seld', '-proc', 'ADSP-21569', '-T', folder / 'engine.ldf', '-o', folder / 'engine.dxe', folder / 'engine.doj',
                 folder / 'helpers.doj', folder / 'control_tables.doj', *[f.with_suffix('.doj') for f in fast]]]
    from tools.machines.translation.fast_toolchain import selas
    fixed = selas()
    with (folder / 'native-build.log').open('w') as log:
        for cmd in commands:
            tool = fixed if cmd[0] == 'selas' and str(cmd[-1]).endswith(tuple(f'fast{n}.s' for n in range(4))) \
                else SELACHE_BIN / cmd[0]
            subprocess.run([str(tool), *map(str, cmd[1:])], stdout=log, stderr=log, check=True)
    return e


def run_machine(m, instructions, img, work):
    folder = work / m['name']
    e = build(m, folder, instructions, img)
    reference = OUT / m['name']
    vm = V.VM()
    vm.load_dn2()
    elf = vm.load_elf(folder / 'engine.dxe')
    stack_top = 0x26f7f0
    vm.c_runtime_state(stack_base=stack_top - 4000, stack_top=stack_top, l7=0, mode1=vm.reg('MODE1'))
    symbols = {k.rstrip('.'): v for k, v in elf.symbols.items()}
    p = m['prefix']
    voice, params, coeff, sine = 0x300000, 0x302000, 0x302100, 0x80550000
    statebytes = m['state_bytes']
    words = m['original_slot_words']
    vm.write(sine, (reference / 'sine.raw').read_bytes())
    vm.write(TEST_SOURCE_BASE, (OUT / 'P.u32').read_bytes()[0x80000:0x800000])
    vm.write(voice - 16, b'\xa5' * (statebytes + 32))
    vm.call(symbols['md_runtime_reset'])
    vm.call(symbols[p + '_voice_init'], args=(voice,))
    counts = []
    actual = bytearray()
    schedule = (reference / 'schedule.raw').read_bytes()
    original = (reference / 'original.raw').read_bytes()
    width = (1729 + int(m.get('buffered_output', False))) if m.get('private_memory') else 129
    for block in range(len(schedule) // 40):
        row = struct.unpack_from('<10I', schedule, 40 * block)
        vm.write(params, struct.pack('<8I', *row[2:]))
        vm.write(coeff, struct.pack('<I', row[1]) + bytes(63 * 4))
        n = vm.call(symbols[p + '_compute_coeffs'], args=(params, coeff))
        vm.write(coeff, struct.pack('<I', row[1]))
        actual += struct.pack('<I', n) + vm.read(coeff, 128)
        vm.call(symbols[p + '_update_coeffs'], args=(params, voice, row[1]))
        if row[1]:
            vm.call(symbols[p + '_voice_trigger'], args=(voice,))
        before = vm.icount
        if row[0]:
            vm.call(symbols[p + '_render_block_44k1'], args=(voice, sine))
        counts.append(vm.icount - before)
        fault = vm.rd32(voice + m.get('memory_fault_byte_offset', statebytes - 4))
        actual += vm.read(voice + m['stream_byte_offset'], 128)
        state = struct.unpack(f'<{words}I', vm.read(voice, 4 * words))
        actual += struct.pack('<64I', *(x & 0xffffff for x in state), *([0] * (64 - words)))
        if m.get('private_memory'):
            extra = struct.unpack('<1600I', vm.read(voice + words * 4 + 180, 6400))
            actual += struct.pack('<1600I', *(x & 0xffffff for x in extra))
            if m.get('buffered_output'):
                actual += struct.pack('<I', vm.rd32(voice + m['state_bytes'] - 4) & 0xffffff)
        assert vm.read(voice - 16, 16) == b'\xa5' * 16 and vm.read(voice + statebytes, 16) == b'\xa5' * 16, 'guards'
        here = bytes(actual[-width * 4:])
        want = original[block * width * 4:(block + 1) * width * 4]
        if here != want or fault:
            i = next((i for i in range(width) if here[4 * i:4 * i + 4] != want[4 * i:4 * i + 4]), None)
            return dict(status='FAIL', block=block, word=i, fault=fault,
                        actual=here[4 * i:4 * i + 4].hex() if i is not None else None,
                        original=want[4 * i:4 * i + 4].hex() if i is not None else None)
    global VERIFY48
    if VERIFY48 is None:
        VERIFY48 = _verify48_without_oracle()
    try:
        rate48 = VERIFY48(m, folder_for_48(m, folder), vm, symbols, voice, params, sine)
    except AssertionError as exc:
        return dict(status='FAIL48', error=repr(exc)[:300])
    return dict(status='PASS', blocks=len(counts), render44_median=statistics.median(counts), render44_max=max(counts),
                rate48_median=rate48['emulator_instructions']['median'], rate48_max=rate48['emulator_instructions']['maximum'])


def _verify48_without_oracle():
    import inspect
    src = inspect.getsource(N.verify48)
    start = src.index("    with (folder/'oracle48.log')")
    end = src.index("    raw=reference.read_bytes()")
    src = src[:start] + "    assert schedule.read_bytes()==(OUT/m['name']/'schedule48.raw').read_bytes(),'48k schedule changed'\n" + src[end:]
    scope = dict(N.__dict__)
    scope['OUT'] = OUT
    exec(src, scope)
    return scope['verify48']


VERIFY48 = None


def folder_for_48(m, folder):
    """verify48 reads host-proof.json for the block count and writes its
    reference/actual files into the folder; reuse the recorded 48k oracle."""
    src = OUT / m['name']
    for name in ('host-proof.json', 'schedule48.raw', 'original48.raw'):
        (folder / name).write_bytes((src / name).read_bytes())
    return folder


def main():
    a = argparse.ArgumentParser()
    a.add_argument('--machines', nargs='*')
    a.add_argument('--work', type=Path, default=Path('/tmp/digimd/fastnative'))
    x = a.parse_args()
    from tools.machines.translation.dsp import decode
    instructions = decode(OUT / 'decoded.tsv')
    img = F.image()
    results = {}
    for m in F.machines(x.machines):
        try:
            r = run_machine(m, instructions, img, x.work)
        except Exception as exc:  # report and continue
            r = dict(status='ERROR', error=repr(exc)[:400])
        results[m['name']] = r
        print(m['name'], r, flush=True)
    (x.work / 'results.json').write_text(json.dumps(results, indent=2))


if __name__ == '__main__':
    main()
