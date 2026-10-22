"""Pairing class of every machine, decided by the pair gate (fast_pair_gate).

For each machine the pair render is gated in plain mode, normally and with
every data branch forced through the per-lane fallback
(FAST_PAIR_FORCE_DIVERGE): both must be bit-exact. If not, the same in merge
mode (lane B's scratch writes tracked word by word). A machine that passes
neither is unpairable: voice B's render depends on what voice A's render
wrote in the same block (shared scratch, md_shared_y20), which a pair cannot
reproduce. The result (class, costs, divergence rate) goes to
out/machines/fast_pair/classes.json; fast_pair.PAIR_MERGE and
PAIR_UNPAIRABLE are set from it.

    python3 -m tools.machines.translation.fast_pair_classify [--machines ...] [--blocks 240]
"""
import argparse
import json
import os
from pathlib import Path

from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_pair_gate import gate
from tools.machines.registry import ROOT

OUT = ROOT / 'out/machines/fast_pair/classes.json'


def run(m, ins, img, work, blocks, merge, forced):
    if forced:
        os.environ['FAST_PAIR_FORCE_DIVERGE'] = '1'
    else:
        os.environ.pop('FAST_PAIR_FORCE_DIVERGE', None)
    try:
        r = gate(m, ins, img, work, blocks=blocks, merge=merge)
    except Exception as e:          # a crash counts as a failure
        r = dict(status='ERROR', error=f'{type(e).__name__}: {e}'[:300])
    finally:
        os.environ.pop('FAST_PAIR_FORCE_DIVERGE', None)
    r.pop('diverged_at', None)
    if r.get('mismatch'):
        r['mismatch'] = {k: r['mismatch'][k] for k in ('block', 'differs', 'first')}
    return r


def classify(m, ins, img, work, blocks):
    tried = {}
    for mode, merge in (('plain', False), ('merge', True)):
        normal = run(m, ins, img, work, blocks, merge, False)
        forced = run(m, ins, img, work, max(40, blocks // 4), merge, True) if normal['status'] == 'PASS' else None
        tried[mode] = dict(normal=normal, forced=forced)
        if normal['status'] == 'PASS' and forced['status'] == 'PASS':
            return dict(cls=mode, **{k: normal.get(k) for k in (
                'per_voice_pair', 'per_voice_scalar', 'speedup', 'pair_mean', 'two_scalar_mean', 'diverged_blocks',
                'paired_blocks')}), tried
    return dict(cls='unpairable'), tried


def main():
    a = argparse.ArgumentParser()
    a.add_argument('--machines', nargs='*')
    a.add_argument('--blocks', type=int, default=240)
    a.add_argument('--work', type=Path, default=Path('/tmp/digimd/fastpair'))
    x = a.parse_args()
    from tools.machines.translation.dsp import decode
    ins = decode(F.OUT / 'decoded.tsv'); img = F.image()
    out = json.loads(OUT.read_text()) if OUT.exists() else {}
    for m in F.machines(x.machines):
        res, tried = classify(m, ins, img, x.work, x.blocks)
        out[m['name']] = dict(res, tried=tried)
        print(m['name'], json.dumps(res), flush=True)
        OUT.parent.mkdir(parents=True, exist_ok=True)
        OUT.write_text(json.dumps(out, indent=1, sort_keys=True))
    return out


if __name__ == '__main__':
    main()
