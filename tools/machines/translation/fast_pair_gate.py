"""sharcvm gate for the two-voice SIMD render (fast_pair).

Builds the fast_native image of an engine plus `<prefix>_render_pair`, then
drives two voices of the machine block by block: voice A on the engine's
original-firmware schedule (or a randomized one), voice B on a randomized
schedule (seeded), each with its own virtual voice. Per block, after the engine's own C coefficient/trigger code:

* reference: the scalar fast render of A, then of B (shared scratch and the
  shared y20 word carried from A to B, as on the unit);
* pair: `<prefix>_render_pair(A, B, scratch)` on the same two voice structs
  (each lane addresses its own).

Both voice structs (output block, Y state, private memory, fault word), the
shared scratch X/Y and md_shared_y20 must be bit-identical, and the pair must
return 0. Before each paired block the shared scratch and md_shared_y20 are
randomized (other tracks write them between renders on the unit). Blocks where only one voice renders use the scalar render. Reports
instructions of the pair against the two scalar renders.

    python3 -m tools.machines.translation.fast_pair_gate --machines P-I-SD
"""
import argparse
import json
from collections import defaultdict
import random
import statistics
import struct
import subprocess
from pathlib import Path

from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_native import TEST_SOURCE_BASE, build, layout
from tools.machines.translation.fast_pair import compile_pair
from tools.machines.translation.fast_spec import random_schedule
from tools.trxbd_sharc.build_sharc import SELACHE_BIN
import vm as V

VA, VB, VS = 0x300000, 0x304000, 0x30e000      # VS: private scratch (twice that in merge mode)
PARAMS, COEFF = 0x302f00, 0x302f80


def build_pair(m, folder, instructions, img, merge=None):
    e = build(m, folder, instructions, img)
    machine, ops = F.lift(m, instructions, img)
    from tools.machines.translation.fast_native import TEST_SOURCE_BASE as base
    from tools.machines.translation.fast_profile import hints
    from tools.machines.translation.fast_pair import PAIR_MERGE
    text, pe = compile_pair(m['prefix'] + '_render_pair', machine, ops, layout(m), base, hints=hints(m, machine, ops),
                            merge=m['name'] in PAIR_MERGE if merge is None else merge)
    (folder / 'pair.s').write_text(text)
    from tools.machines.translation.fast_toolchain import selas
    r = subprocess.run([str(selas()), '-proc', 'ADSP-21569', '-o', str(folder / 'pair.doj'), str(folder / 'pair.s')],
                       capture_output=True, text=True)
    if r.returncode or (r.stdout + r.stderr).strip():
        raise SystemExit('selas pair.s: ' + (r.stdout + r.stderr)[-800:])
    fast = sorted(folder.glob('fast[0-9].doj'))
    cmd = [str(SELACHE_BIN / 'seld'), '-proc', 'ADSP-21569', '-T', str(folder / 'engine.ldf'), '-o', str(folder / 'engine.dxe'),
           str(folder / 'engine.doj'), str(folder / 'helpers.doj'), str(folder / 'control_tables.doj'),
           *map(str, fast), str(folder / 'pair.doj')]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        raise SystemExit('seld: ' + (r.stdout + r.stderr)[-800:])
    return machine


def interleave(a, b):
    wa = struct.unpack(f'<{len(a) // 4}I', a); wb = struct.unpack(f'<{len(b) // 4}I', b)
    return struct.pack(f'<{2 * len(wa)}I', *[x for pair in zip(wa, wb) for x in pair])


def deinterleave(p):
    w = struct.unpack(f'<{len(p) // 4}I', p)
    return struct.pack(f'<{len(w) // 2}I', *w[0::2]), struct.pack(f'<{len(w) // 2}I', *w[1::2])


def gate(m, instructions, img, work, blocks=240, seed=1, same_schedule=False, vv=(0, 1), random_a=False, scramble=True,
         merge=None):
    folder = work / m['name']
    machine = build_pair(m, folder, instructions, img, merge)
    vm = V.VM()
    vm.load_dn2()
    elf = vm.load_elf(folder / 'engine.dxe')
    top = 0x26f7f0
    vm.c_runtime_state(stack_base=top - 4000, stack_top=top, l7=0, mode1=vm.reg('MODE1'))
    sy = {k.rstrip('.'): v for k, v in elf.symbols.items()}
    p = m['prefix']
    sine = 0x80550000
    vm.write(sine, (F.OUT / m['name'] / 'sine.raw').read_bytes())
    vm.write(TEST_SOURCE_BASE, (F.OUT / 'P.u32').read_bytes()[0x80000:0x800000])
    sb = m['state_bytes']
    assert sb <= VB - VA and VB + sb <= VS, sb
    nx, ny = machine.scratch_x_words, machine.scratch_y_words
    sx, sy_ = sy['md_shared_scratch_x'], sy['md_shared_scratch_y']

    def read_scratch():
        return vm.read(sx, 4 * nx) + vm.read(sy_, 4 * ny)

    def write_scratch(data):
        vm.write(sx, data[:4 * nx]); vm.write(sy_, data[4 * nx:])
    scr_bytes = 4 * (nx + ny)
    y20 = sy.get('md_shared_y20')
    vm.call(sy['md_runtime_reset'])
    for v, w in zip((VA, VB), vv):
        vm.call(sy[p + '_voice_init'], args=(v,))
        if m.get('private_memory'):
            vm.wr32(v + layout(m)['virtual_voice_byte'], w)
    rnd = random.Random(seed)
    write_scratch(bytes(rnd.getrandbits(8) for _ in range(scr_bytes)))
    sched_a = random_schedule(m, blocks, seed=2 * seed) if random_a else (F.OUT / m['name'] / 'schedule.raw').read_bytes()
    sched_b = sched_a if same_schedule else random_schedule(m, blocks, seed=2 * seed + 1)
    n = min(blocks, len(sched_a) // 40, len(sched_b) // 40)
    pair_cost, scalar_cost, paired, mism = [], [], 0, None
    # blocks where the lanes disagreed at a branch (the per-lane fallback ran)
    laneb = sy.get(p + '_render_pair_laneb')
    diverged, last_div, div_blocks, declined = 0, None, [], 0
    if laneb is not None:
        vm.lib.vm_set_profile(vm.h, 1)

    def hits(pc=None):
        pc = laneb if pc is None else pc
        page = vm.pages.get(pc >> 12)
        return page[pc & 0xfff].hits if page is not None else 0
    divs = sorted((int(k.rsplit('_div', 1)[1]), v) for k, v in sy.items() if k.startswith(p + '_render_pair_div'))
    div_cost = defaultdict(list)       # per branch: pair cost of the blocks that diverged there

    def snapshot():
        return (vm.read(VA, sb), vm.read(VB, sb), read_scratch(), vm.read(y20, 4) if y20 else b'')

    def restore(s):
        vm.write(VA, s[0]); vm.write(VB, s[1]); write_scratch(s[2])
        if y20:
            vm.write(y20, s[3])
    for block in range(n):
        rows = [struct.unpack_from('<10I', sched_a, 40 * block), struct.unpack_from('<10I', sched_b, 40 * block)]
        for v, row in zip((VA, VB), rows):
            vm.write(PARAMS, struct.pack('<8I', *row[2:]))
            vm.write(COEFF, struct.pack('<I', row[1]) + bytes(63 * 4))
            vm.call(sy[p + '_compute_coeffs'], args=(PARAMS, COEFF))
            vm.call(sy[p + '_update_coeffs'], args=(PARAMS, v, row[1]))
            if row[1]:
                vm.call(sy[p + '_voice_trigger'], args=(v,))
        if not (rows[0][0] and rows[1][0]):
            for v, row in zip((VA, VB), rows):
                if row[0]:
                    vm.call(sy[p + '_render_block_44k1'], args=(v, sine))
            continue
        if scramble:
            # other tracks write the shared scratch (and y20) between renders
            write_scratch(bytes(rnd.getrandbits(8) for _ in range(scr_bytes)))
            if y20:
                vm.wr32(y20, rnd.getrandbits(24))
        before = snapshot()
        n0 = vm.icount
        vm.call(sy[p + '_render_block_44k1'], args=(VA, sine))
        vm.call(sy[p + '_render_block_44k1'], args=(VB, sine))
        scalar_cost.append(vm.icount - n0)
        want = snapshot()
        restore(before)
        n0 = vm.icount
        h0 = hits() if laneb is not None else 0
        d0 = [hits(pc) for _, pc in divs]
        ret = vm.call(sy[p + '_render_pair'], args=(VA, VB, VS))
        pair_cost.append(vm.icount - n0)
        for (k, pc), h in zip(divs, d0):
            if hits(pc) != h:
                div_cost[k].append(pair_cost[-1])
        if laneb is not None and hits() != h0:
            diverged += 1
            last_div = block
            div_blocks.append(block)
        got = snapshot()
        paired += 1
        if ret == 1 and got == before:
            # the pair declined (an entry guard, or lanes that disagree
            # before anything is written): the caller renders scalar
            declined += 1
            n0 = vm.icount
            vm.call(sy[p + '_render_block_44k1'], args=(VA, sine))
            vm.call(sy[p + '_render_block_44k1'], args=(VB, sine))
            pair_cost[-1] += vm.icount - n0
            got, ret = snapshot(), 0
        if ret != 0 or got != want:
            names = ('voice A', 'voice B', 'shared scratch', 'md_shared_y20')
            which = [names[i] for i in range(4) if got[i] != want[i]]
            first = None
            for i in range(4):
                if got[i] != want[i]:
                    k = next(j for j in range(0, len(got[i]), 4) if got[i][j:j + 4] != want[i][j:j + 4])
                    first = dict(region=names[i], byte=k, got=got[i][k:k + 4].hex(), want=want[i][k:k + 4].hex())
                    break
            words = {}
            for i in range(4):
                if got[i] != want[i]:
                    words[names[i]] = [(j // 4, got[i][j:j + 4].hex(), want[i][j:j + 4].hex())
                                       for j in range(0, len(got[i]), 4) if got[i][j:j + 4] != want[i][j:j + 4]][:24]
            mism = dict(block=block, ret=ret, differs=which, first=first, words=words, diverged=last_div == block)
            break
        restore(want)
    ok = mism is None and paired > 0
    res = dict(status='PASS' if ok else 'FAIL', blocks=n, paired_blocks=paired, diverged_blocks=diverged, declined_blocks=declined, diverged_at=div_blocks,
               diverged_by_branch={k: (len(v), round(statistics.mean(v))) for k, v in sorted(div_cost.items())}, mismatch=mism)
    if pair_cost:
        res.update(pair_median=statistics.median(pair_cost), two_scalar_median=statistics.median(scalar_cost),
                   pair_mean=round(statistics.mean(pair_cost)), two_scalar_mean=round(statistics.mean(scalar_cost)),
                   per_voice_pair=statistics.median(pair_cost) / 2, per_voice_scalar=statistics.median(scalar_cost) / 2,
                   speedup=round(statistics.median(scalar_cost) / statistics.median(pair_cost), 3))
    return res


def main():
    a = argparse.ArgumentParser()
    a.add_argument('--machines', nargs='*', default=['P-I-SD'])
    a.add_argument('--blocks', type=int, default=240)
    a.add_argument('--work', type=Path, default=Path('/tmp/digimd/fastpair'))
    x = a.parse_args()
    from tools.machines.translation.dsp import decode
    ins = decode(F.OUT / 'decoded.tsv'); img = F.image()
    out = {}
    for m in F.machines(x.machines):
        out[m['name']] = gate(m, ins, img, x.work, x.blocks)
        print(m['name'], json.dumps(out[m['name']]), flush=True)
    return out


if __name__ == '__main__':
    main()
