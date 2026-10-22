"""Entry guards for the fast render: profile-chosen ranges of voice Y words.

Some engines keep buffer pointers in their Y state words and reload them at
the start of each render. When such a word is known to lie in a small range,
the optimizer can map every access through it to a fixed memory region. The
specialized render checks each assumed word on entry; if any word is outside
its range it calls the generic fast render instead, so results never depend
on the profile -- only speed does.

Ranges come from running the reference C renderer (the generated engine) over
the recorded schedule plus a long randomized schedule, and widening each
word's observed values to the aligned power-of-two block that contains them
(a modulo buffer's natural extent).
"""
import os
import random
import struct
import subprocess
from pathlib import Path

from tools.machines.registry import ROOT, machines as registry_machines
from tools.machines.translation.verify import host_source
from tools.machines.translation import fast_opt

# per process: concurrent gate runs must not share profile logs
WORK = Path(f'/tmp/digimd/fastspec-{os.getpid()}')
MAX_BLOCK = 4096


def random_schedule(m, blocks=3000, seed=0):
    rng = random.Random(0x5eed + m['md_id'] + 0x10000 * seed)
    original = next(q for q in registry_machines() if q['md_id'] == m['md_id'])
    params = [p['default_raw'] << 7 for p in original['syn']]
    rows = []
    for b in range(blocks):
        if b % 200 == 100:
            params = [rng.choice((0, 0x3f80, rng.randrange(0x3f81))) for _ in range(8)]
        elif rng.random() < 0.08:
            params[rng.randrange(8)] = rng.randrange(0x3f81)
        trig = int(b == 0 or rng.random() < 1 / 12)
        rows.append(struct.pack('<10I', 1, trig, *params))
    return b''.join(rows)


VOICES = (0, 7, 15)


def profile_entry_words(m):
    """Observed raw Y words at every render entry: {virtual voice: [set per word]}."""
    WORK.mkdir(parents=True, exist_ok=True)
    p = m['prefix']
    private = bool(m.get('private_memory'))
    words = 64 if private else m['original_slot_words']
    log = WORK / f'{p}.words'
    src = host_source(m)
    call = f'if (row[0]) {p}_render_block_44k1(&v, sine);'
    assert call in src
    src = src.replace(call, f'if (row[0]) {{ fwrite(v.y, 4, {words}, md_prof); fflush(md_prof); {p}_render_block_44k1(&v, sine); }}')
    src = src.replace('    md_runtime_reset();', f'    FILE *md_prof = fopen("{log}", "wb");\n    md_runtime_reset();', 1)
    if private:
        init = f'    {p}_voice_init(&v);\n'
        assert init in src
        src = src.replace(init, init + '    v.virtual_voice = getenv("MD_VV") ? (uint32_t)atoi(getenv("MD_VV")) : 0u;\n', 1)
    path = WORK / f'{p}.c'
    path.write_text(src)
    exe = WORK / p
    subprocess.run(['clang', '-O1', '-w', '-o', str(exe), str(path), str(ROOT / 'tools/machines/generated/control_tables.c')],
                   check=True)
    folder = ROOT / 'out/machines/translation' / m['name']
    result = {}
    for vv in (VOICES if private else (0,)):
        seen = [set() for _ in range(words)]
        for schedule in (folder / 'schedule.raw', None):
            if schedule is None:
                schedule = WORK / f'{p}.random.raw'
                schedule.write_bytes(random_schedule(m))
            out = WORK / f'{p}.out'
            # a memory fault on random controls ends the run early; what was
            # logged up to that point is still valid
            subprocess.run([str(exe), str(schedule), str(out), str(folder / 'sine.raw')], timeout=120,
                           stderr=subprocess.DEVNULL, env=dict(os.environ, MD_VV=str(vv)))
            data = log.read_bytes()
            for n in range(len(data) // (4 * words)):
                row = struct.unpack_from(f'<{words}I', data, 4 * words * n)
                for k, x in enumerate(row):
                    seen[k].add(x)
        result[vv] = seen
    return result


def candidates(seen, ops):
    """(k, lo, hi, s) for loaded words w with w - s*VV in one small aligned block."""
    loaded = {o[3] for o in ops if o[0] == 'load' and o[2] == 'Y' and isinstance(o[3], int)}
    words = len(next(iter(seen.values())))
    out = []
    for k in range(words):
        if k not in loaded:
            continue
        best = None
        for s in ((0, 64, 1536) if len(seen) > 1 else (0,)):
            values = {x - s * vv for vv, sets in seen.items() for x in sets[k]}
            if not values:
                continue
            lo, hi = min(values), max(values)
            if lo < 0 or hi > 0xffffff:
                continue
            size = 1 << (lo ^ hi).bit_length()
            if size > MAX_BLOCK:
                continue
            base = lo & ~(size - 1)
            if best is None or size < best[0]:
                best = (size, (k, base, base + size - 1, s))
        if best:
            out.append(best[1])
    return out


def weighted_unresolved(ops, machine):
    """Generic accesses left after optimization, weighted 16x per loop level."""
    ranges, succ, pred = fast_opt.blocks(ops)
    loops = fast_opt.find_loops(ops, ranges, pred)
    depth = [0] * len(ops)
    for h, (latch, c, body) in loops.items():
        for b in body:
            for i in range(*ranges[b]):
                depth[i] += 1
    return sum(16 ** depth[i] for i, o in enumerate(ops) if o[0] in ('dread', 'dwrite'))


def choose(m, machine, lifted, optimize):
    """Guards worth having: (guards, specialized ops or None, generic ops)."""
    head = [o for o in lifted[:1] if o[0] == 'guard_vv']
    rest = lifted[len(head):]
    base = optimize(list(lifted), machine)
    base_score = weighted_unresolved(base, machine)
    if base_score == 0:
        return [], None, base
    seen = profile_entry_words(m)
    cands = candidates(seen, base)
    if not cands:
        return [], None, base

    def build(gs):
        ops = optimize(head + [('guard_y',) + g for g in gs] + rest, machine)
        return ops, weighted_unresolved(ops, machine)
    best_ops, best = build(cands)
    if best >= base_score:
        return [], None, base
    chosen = list(cands)
    for g in list(cands):
        trial = [x for x in chosen if x != g]
        ops, score = build(trial)
        if score <= best:
            chosen, best_ops, best = trial, ops, score
    if not chosen:
        return [], None, base
    return chosen, best_ops, base
