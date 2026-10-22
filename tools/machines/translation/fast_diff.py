"""Host differential: fast render (as C) against the reference generated engine.

Beyond fast_check (original recording, virtual voice 0), this runs a long
randomized schedule for virtual voices 0, 7 and 15 (private-memory engines)
and also forces the unguarded fallback path (`_g`), comparing every block's
output, Y state and private memory with the reference C renderer built from
the same source. The reference engine is itself verified against the original
firmware (fast_check / verify).
"""
import os
import argparse
import subprocess
from pathlib import Path

from tools.machines.registry import ROOT
from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_c import emit_c
from tools.machines.translation.fast_spec import random_schedule
from tools.machines.translation.verify import host_source

# per process: concurrent gate runs must not share profile logs
WORK = Path(f'/tmp/digimd/fastdiff-{os.getpid()}')


def harness(m, render=None, functions=''):
    p = m['prefix']
    src = host_source(m)
    if render:
        src = src.replace(f'if (row[0]) {p}_render_block_44k1(&v, sine);', f'if (row[0]) {render}(&v, sine);')
    if m.get('private_memory'):
        init = f'    {p}_voice_init(&v);\n'
        src = src.replace(init, init + '    v.virtual_voice = getenv("MD_VV") ? (uint32_t)atoi(getenv("MD_VV")) : 0u;\n', 1)
    marker = '#include <stdio.h>\nuint32_t md_source_words'
    return src.replace(marker, functions + marker, 1)


def build(m, name, src):
    WORK.mkdir(parents=True, exist_ok=True)
    path = WORK / f'{name}.c'
    path.write_text(src)
    exe = WORK / name
    subprocess.run(['clang', '-O1', '-w', '-o', str(exe), str(path), str(ROOT / 'tools/machines/generated/control_tables.c')],
                   check=True)
    return exe


def run(exe, schedule, vv, out):
    import os
    r = subprocess.run([str(exe), str(schedule), str(out), str(F.OUT / 'GND-SIN' / 'sine.raw')], timeout=600,
                       stderr=subprocess.DEVNULL, env=dict(os.environ, MD_VV=str(vv)))
    return r.returncode


def diff(m, instructions, img, blocks=4000):
    p = m['prefix']
    machine, ops = F.lift(m, instructions, img)
    ref = build(m, p + '_ref', harness(m))
    fns = ''
    if machine.generic_ops is not None:
        fns = emit_c(p + '_render_fast_g', m['voice_type'], machine.generic_ops, fallback=p + '_render_block_44k1')
    fast_fn = fns + emit_c(p + '_render_fast', m['voice_type'], ops, fallback=p + '_render_block_44k1',
                           generic=p + '_render_fast_g')
    fast = build(m, p + '_fast', harness(m, p + '_render_fast', fast_fn))
    variants = [('fast', fast)]
    if machine.generic_ops is not None:
        variants.append(('generic', build(m, p + '_gen', harness(m, p + '_render_fast_g', fns))))
    schedule = WORK / f'{p}.schedule'
    schedule.write_bytes(random_schedule(m, blocks))
    result = {}
    for vv in ((0, 7, 15) if m.get('private_memory') else (0,)):
        want = WORK / f'{p}.ref.{vv}'
        rc = run(ref, schedule, vv, want)
        for label, exe in variants:
            got = WORK / f'{p}.{label}.{vv}'
            rc2 = run(exe, schedule, vv, got)
            a, b = want.read_bytes(), got.read_bytes()
            result[f'{label}/vv{vv}'] = 'PASS' if a == b and rc == rc2 else f'FAIL rc={rc}/{rc2} bytes={len(a)}/{len(b)}'
    return result


if __name__ == '__main__':
    a = argparse.ArgumentParser()
    a.add_argument('--machines', nargs='*')
    a.add_argument('--blocks', type=int, default=4000)
    x = a.parse_args()
    from tools.machines.translation.dsp import decode
    ins = decode(F.OUT / 'decoded.tsv')
    img = F.image()
    bad = 0
    for m in F.machines(x.machines):
        r = diff(m, ins, img, x.blocks)
        bad += sum(v != 'PASS' for v in r.values())
        print(m['name'], r, flush=True)
    raise SystemExit(1 if bad else 0)
