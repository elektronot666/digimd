"""Per-line dynamic hit profile of a fast render built by fast_native (sharcvm)."""
import collections
import struct
import sys
from pathlib import Path

from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_native import TEST_SOURCE_BASE
import vm as V
import decode as D


def hot(name, work=Path('/tmp/digimd/fn2'), blocks=64):
    m = next(q for q in F.machines([name]))
    folder = work / name
    reference = F.OUT / name
    vm = V.VM()
    vm.load_dn2()
    elf = vm.load_elf(folder / 'engine.dxe')
    stack_top = 0x26f7f0
    vm.c_runtime_state(stack_base=stack_top - 4000, stack_top=stack_top, l7=0, mode1=vm.reg('MODE1'))
    symbols = {k.rstrip('.'): v for k, v in elf.symbols.items()}
    p = m['prefix']
    voice, params, coeff, sine = 0x300000, 0x302000, 0x302100, 0x80550000
    vm.write(sine, (reference / 'sine.raw').read_bytes())
    vm.write(TEST_SOURCE_BASE, (F.OUT / 'P.u32').read_bytes()[0x80000:0x800000])
    vm.call(symbols['md_runtime_reset'])
    vm.call(symbols[p + '_voice_init'], args=(voice,))
    schedule = (reference / 'schedule.raw').read_bytes()
    vm.lib.vm_set_profile(vm.h, 1)
    for block in range(min(blocks, len(schedule) // 40)):
        row = struct.unpack_from('<10I', schedule, 40 * block)
        vm.write(params, struct.pack('<8I', *row[2:]))
        vm.write(coeff, struct.pack('<I', row[1]) + bytes(63 * 4))
        vm.call(symbols[p + '_compute_coeffs'], args=(params, coeff))
        vm.call(symbols[p + '_update_coeffs'], args=(params, voice, row[1]))
        if row[1]:
            vm.call(symbols[p + '_voice_trigger'], args=(voice,))
        if row[0]:
            vm.call(symbols[p + '_render_block_44k1'], args=(voice, sine))
    lines = (folder / 'fast0.s').read_text().split('\n')
    pc = symbols[p + '_render_block_44k1']
    start = next(i for i, l in enumerate(lines) if l.startswith(p + '_render_block_44k1.:'))
    hits = {}
    for i in range(start + 1, len(lines)):
        l = lines[i].strip()
        if not l or l.endswith(':') or l.startswith('.') or l.startswith('//'):
            continue
        d = D.decode_at(vm.read, pc)
        slot = vm.pages.get(pc >> 12)
        h = slot[pc & 0xfff].hits if slot is not None else 0
        hits[i] = h
        pc += d.parcels
    return lines, hits


if __name__ == '__main__':
    lines, hits = hot(sys.argv[1], blocks=int(sys.argv[2]) if len(sys.argv) > 2 else 64)
    total = sum(hits.values())
    print('total', total)
    # per label region
    region = collections.Counter()
    cur = 'entry'
    for i, l in enumerate(lines):
        if l.strip().endswith(':'):
            cur = l.strip()
        if i in hits:
            region[cur] += hits[i]
    for k, v in region.most_common(25):
        print(f'{v:10d} {100*v/total:5.1f}% {k}')


def dump(name, blocks, lo, hi, out):
    lines, hits = hot(name, blocks=blocks)
    with open(out, 'w') as f:
        for i, l in enumerate(lines):
            f.write(f'{hits.get(i, ""):>8} {l}\n')
