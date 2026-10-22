"""Run every fast-render gate for all 47 translated engines and record them.

    python3 -m tools.machines.translation.fast_gates [--part host|vm|all]

host: fast_check (original recordings) and fast_diff (reference engine,
randomized schedules, virtual voices, fallbacks); vm: fast_native (sharcvm at
44.1 and 48 kHz against the original recordings) and fast_vmdiff (sharcvm
against the selcc reference render). Writes out/machines/translation/
fast-gates.json (parts merge), status PASS only when every case of every part
passed.
"""
import argparse
import hashlib
import json
from pathlib import Path

from tools.machines.translation import fast_check as F

OUT = F.OUT / 'fast-gates.json'
SOURCES = sorted(Path(__file__).parent.glob('fast_*.py'))


def digest():
    return {p.name: hashlib.sha256(p.read_bytes()).hexdigest() for p in SOURCES}


def host(ins, img):
    from tools.machines.translation.fast_diff import diff
    check = F.check()
    out = {}
    for m in F.machines():
        out[m['name']] = dict(original=check[m['name']]['status'], reference=diff(m, ins, img, 3000))
        print('host', m['name'], out[m['name']], flush=True)
    ok = all(v['original'] == 'PASS' and all(x == 'PASS' for x in v['reference'].values()) for v in out.values())
    return dict(status='PASS' if ok else 'FAIL', machines=out)


def vm(ins, img, work=Path('/tmp/digimd/fastgates')):
    from tools.machines.translation.fast_native import run_machine
    from tools.machines.translation.fast_vmdiff import vmdiff
    out = {}
    for m in F.machines():
        try:
            native = run_machine(m, ins, img, work)
        except Exception as exc:
            native = dict(status='ERROR', error=repr(exc)[:300])
        ref = vmdiff(m, ins, img, work, 300) if native.get('status') == 'PASS' else {}
        out[m['name']] = dict(original=native, reference=ref)
        print('vm', m['name'], out[m['name']], flush=True)
    ok = all(v['original'].get('status') == 'PASS' and v['reference'] and all(x == 'PASS' for x in v['reference'].values())
             for v in out.values())
    return dict(status='PASS' if ok else 'FAIL', machines=out)


def main():
    a = argparse.ArgumentParser()
    a.add_argument('--part', choices=('host', 'vm', 'all'), default='all')
    x = a.parse_args()
    from tools.machines.translation.dsp import decode
    ins = decode(F.OUT / 'decoded.tsv')
    img = F.image()
    result = json.loads(OUT.read_text()) if OUT.exists() else {}
    src = digest()
    if result.get('sources') != src:
        result = dict(sources=src)
    if x.part in ('host', 'all'):
        result['host'] = host(ins, img)
    if x.part in ('vm', 'all'):
        result['vm'] = vm(ins, img)
    # another part may have finished meanwhile: merge with the file as it is now
    latest = json.loads(OUT.read_text()) if OUT.exists() else {}
    if latest.get('sources') == src:
        for part in ('host', 'vm'):
            if part not in result and part in latest:
                result[part] = latest[part]
    parts = [result.get('host', {}).get('status'), result.get('vm', {}).get('status')]
    result['status'] = 'PASS' if parts == ['PASS', 'PASS'] else 'INCOMPLETE' if None in parts else 'FAIL'
    OUT.write_text(json.dumps(result, indent=1) + '\n')
    print('fast gates', result['status'])


if __name__ == '__main__':
    main()
