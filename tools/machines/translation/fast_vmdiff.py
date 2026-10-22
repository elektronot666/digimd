"""sharcvm differential of the assembly renders against the selcc-compiled
reference C render linked in the same image (built by fast_native).

Runs a randomized schedule for virtual voices 0/7/15 (private-memory
engines) through: the specialized render, the unguarded fallback `_g` (when
present), and the reference `_render_block_44k1_c`; every block's 44.1k output,
Y state and private memory must agree. Each run starts from a fresh VM so the
shared scratch memory evolves identically.
"""
import argparse
import struct
from pathlib import Path

from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_native import TEST_SOURCE_BASE, build
from tools.machines.translation.fast_spec import random_schedule
import vm as V


def run(m, folder, symbol, schedule, vv):
    vm = V.VM()
    vm.load_dn2()
    elf = vm.load_elf(folder / 'engine.dxe')
    stack_top = 0x26f7f0
    vm.c_runtime_state(stack_base=stack_top - 4000, stack_top=stack_top, l7=0, mode1=vm.reg('MODE1'))
    sy = {k.rstrip('.'): v for k, v in elf.symbols.items()}
    p = m['prefix']
    voice, params, coeff, sine = 0x300000, 0x302000, 0x302100, 0x80550000
    words = m['original_slot_words']
    vm.write(sine, (F.OUT / m['name'] / 'sine.raw').read_bytes())
    vm.write(TEST_SOURCE_BASE, (F.OUT / 'P.u32').read_bytes()[0x80000:0x800000])
    vm.call(sy['md_runtime_reset'])
    vm.call(sy[p + '_voice_init'], args=(voice,))
    if m.get('private_memory'):
        vm.wr32(voice + words * 4 + 180 + 256 + 6144, vv)
    out = bytearray()
    for block in range(len(schedule) // 40):
        row = struct.unpack_from('<10I', schedule, 40 * block)
        vm.write(params, struct.pack('<8I', *row[2:]))
        vm.write(coeff, struct.pack('<I', row[1]) + bytes(63 * 4))
        vm.call(sy[p + '_compute_coeffs'], args=(params, coeff))
        vm.call(sy[p + '_update_coeffs'], args=(params, voice, row[1]))
        if row[1]:
            vm.call(sy[p + '_voice_trigger'], args=(voice,))
        if row[0]:
            vm.call(sy[symbol], args=(voice, sine))
        out += vm.read(voice + m['stream_byte_offset'], 128)
        out += vm.read(voice, 4 * words)
        if m.get('private_memory'):
            out += vm.read(voice + words * 4 + 180, 6400)
        out += vm.read(voice + m.get('memory_fault_byte_offset', words * 4 + 176), 4)
    return bytes(out)


def vmdiff(m, instructions, img, work, blocks):
    folder = work / m['name']
    if not (folder / 'engine.dxe').exists():
        build(m, folder, instructions, img)
    schedule = random_schedule(m, blocks)
    p = m['prefix']
    import vm as _v
    elf_syms = None
    result = {}
    for vv in ((0, 7, 15) if m.get('private_memory') else (0,)):
        want = run(m, folder, p + '_render_block_44k1_c', schedule, vv)
        names = [p + '_render_block_44k1']
        if (folder / 'fast1.s').exists():
            names.append(p + '_render_block_44k1_g')
        for name in names:
            got = run(m, folder, name, schedule, vv)
            result[f'{name[len(p) + 1:]}/vv{vv}'] = 'PASS' if got == want else 'FAIL'
    return result


if __name__ == '__main__':
    a = argparse.ArgumentParser()
    a.add_argument('--machines', nargs='*')
    a.add_argument('--blocks', type=int, default=200)
    a.add_argument('--work', type=Path, default=Path('/tmp/digimd/fastvmdiff'))
    x = a.parse_args()
    from tools.machines.translation.dsp import decode
    ins = decode(F.OUT / 'decoded.tsv')
    img = F.image()
    bad = 0
    for m in F.machines(x.machines):
        r = vmdiff(m, ins, img, x.work, x.blocks)
        bad += sum(v != 'PASS' for v in r.values())
        print(m['name'], r, flush=True)
    raise SystemExit(1 if bad else 0)
