"""Profile the generic memory sites left in the fast render.

Each 'dread'/'dwrite' that the optimizer could not resolve statically is
numbered in program order. The host C build logs (site, address) for every
execution over the recorded schedule and a randomized one, and each address
is classified with the original region chain (generate.py order, virtual
voice 0). The backend emits an inline check for the dominant region of a
site, when that region's address range is disjoint from every region earlier
in the chain; any other address still takes the complete original chain, so
hints only affect speed, never results.
"""
import os
import collections
import subprocess
from pathlib import Path

from tools.machines.registry import ROOT
from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_c import emit_c
from tools.machines.translation.fast_opt import Regions
from tools.machines.translation.fast_spec import random_schedule
from tools.machines.translation.verify import host_source

# per process: concurrent gate runs must not share profile logs
WORK = Path(f'/tmp/digimd/fastprofile-{os.getpid()}')


def sites_of(ops):
    out = []
    for o in ops:
        if o[0] == 'dread':
            out.append((o[2], False, o[3]))
        elif o[0] == 'dwrite':
            out.append((o[1], True, o[2]))
    return out


def classify(machine, space, write, address, raw=False):
    regions = Regions(machine)
    a = address & 0xffffff
    if raw and a >= 0xffff80:
        return 'ZERO'
    r = regions.write(space, a) if write else regions.read(space, a)
    return r[0] if r else 'FAULT'


def profile(m, machine, ops):
    """Per-site Counter of region names (specialized render, virtual voice 0)."""
    WORK.mkdir(parents=True, exist_ok=True)
    p = m['prefix']
    sites = sites_of(ops)
    if not sites:
        return []
    fn = emit_c(p + '_render_fast', m['voice_type'], ops, fallback=p + '_render_block_44k1', generic=p + '_render_block_44k1')
    lines = fn.split('\n')
    out = []
    k = 0
    for line in lines:
        if 'md_read_' in line or 'md_write_' in line:
            addr = sites[k][2]
            a = addr if isinstance(addr, str) else hex(addr)
            out.append(f'    fprintf(md_profile, "{k} %x\\n", (unsigned)({a}));')
            k += 1
        out.append(line)
    assert k == len(sites), (k, len(sites))
    fn = 'static FILE *md_profile;\n' + '\n'.join(out)
    src = host_source(m).replace(f'if (row[0]) {p}_render_block_44k1(&v, sine);', f'if (row[0]) {p}_render_fast(&v, sine);')
    log = WORK / (p + '.log')
    src = src.replace('    md_runtime_reset();', f'    md_profile = fopen("{log}", "w");\n    md_runtime_reset();', 1)
    marker = '#include <stdio.h>\nuint32_t md_source_words'
    src = src.replace(marker, fn + marker, 1)
    (WORK / f'{p}.c').write_text(src)
    exe = WORK / p
    subprocess.run(['clang', '-O1', '-w', '-o', str(exe), str(WORK / f'{p}.c'),
                    str(ROOT / 'tools/machines/generated/control_tables.c')], check=True)
    folder = F.OUT / m['name']
    counts = [collections.Counter() for _ in sites]
    spans = [dict() for _ in sites]
    rnd = WORK / f'{p}.random.raw'
    rnd.write_bytes(random_schedule(m))
    for schedule in (folder / 'schedule.raw', rnd):
        subprocess.run([str(exe), str(schedule), str(WORK / f'{p}.raw'), str(folder / 'sine.raw')],
                       stderr=subprocess.DEVNULL, timeout=300)
        for line in log.read_text().split('\n'):
            if not line:
                continue
            s, a = line.split()
            space, write, _ = sites[int(s)]
            raw = False
            name = classify(machine, space, write, int(a, 16))
            counts[int(s)][name] += 1
            a = int(a, 16) & 0xffffff
            lo, hi = spans[int(s)].get(name, (a, a))
            spans[int(s)][name] = (min(lo, a), max(hi, a))
    return counts, spans


def hints(m, machine, ops):
    """Per generic site: (dominant region, lowest, highest profiled address) or None."""
    if not sites_of(ops):
        return []
    counts, spans = profile(m, machine, ops)
    out = []
    for c, s in zip(counts, spans):
        if not c:
            out.append(None)
            continue
        name = c.most_common(1)[0][0]
        out.append((name,) + s[name])
    return out


if __name__ == '__main__':
    import sys
    from tools.machines.translation.dsp import decode
    ins = decode(F.OUT / 'decoded.tsv')
    img = F.image()
    for m in F.machines(sys.argv[1:] or None):
        machine, ops = F.lift(m, ins, img)
        counts, spans = profile(m, machine, ops)
        total = collections.Counter()
        for c in counts:
            total.update(c)
        print(m['name'], [dict(c) for c in counts], flush=True)
