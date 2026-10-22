"""Host check: lifted IR (via fast_c) against the original-firmware recordings.

For each generated machine, the render routine is lifted and emitted as C,
compiled into the generated engine's translation unit, and run over the same
schedule as the original oracle recording (out/machines/translation/<NAME>/
original.raw). The output (coefficients, 44.1k block, all 64 slot words and
private memory) must be byte-identical.
"""
import argparse
import json
import subprocess
from pathlib import Path

from tools.machines.registry import ROOT
from tools.machines.translation.dsp import decode
from tools.machines.translation.verify import host_source
from tools.machines.translation.fast_ir import Lifter, Machine
from tools.machines.translation import fast_opt
from tools.machines.translation.fast_c import emit_c

OUT = ROOT / 'out/machines/translation'


def machines(names=None):
    batch = json.loads((OUT / 'batch.json').read_text())
    for m in batch['machines']:
        if m.get('status') != 'generated-unverified':
            continue
        if names and m['name'] not in names:
            continue
        yield m


def image():
    from digimd import load_firmware, MD_DEVICE
    from digimd_dsp56300 import parse_dsp_image
    fw = load_firmware(ROOT / 'firmware/Elektron_SPS1-1UW_OS1.63.syx', MD_DEVICE)
    return parse_dsp_image(fw.section_bytes(1))


CACHE = Path('/tmp/digimd/ircache')


def _cache_key(m, optimize):
    import hashlib
    h = hashlib.sha256()
    for f in ('fast_ir.py', 'fast_opt.py', 'dsp.py', 'fast_spec.py'):
        h.update((Path(__file__).parent / f).read_bytes())
    h.update(json.dumps(m, sort_keys=True).encode())
    h.update(b'1' if optimize else b'0')
    return h.hexdigest()[:24]


def lift(m, instructions, img, optimize=True):
    import pickle
    CACHE.mkdir(parents=True, exist_ok=True)
    path = CACHE / f"{m['name']}-{_cache_key(m, optimize)}.pkl"
    if path.exists():
        return pickle.loads(path.read_bytes())
    machine, ops = _lift(m, instructions, img, optimize)
    path.write_bytes(pickle.dumps((machine, ops)))
    return machine, ops


def _lift(m, instructions, img, optimize=True):
    """(machine, ops): the specialized render; machine.generic_ops is the
    unguarded fast render it falls back to (None when there are no guards)."""
    from tools.machines.translation import fast_spec
    machine = Machine(m)
    lifted = Lifter(instructions, machine, img).function(int(m['entries']['render'], 16))
    ops = lifted['ops']
    if machine.private_memory:
        # virtual voices 0..15 take the specialized path; any other value
        # falls back to the reference renderer (see fast_c/fast_sharc)
        ops = [('guard_vv', 16)] + ops
    machine.generic_ops = None
    machine.guards = []
    if optimize:
        guards, special, generic = fast_spec.choose(m, machine, ops, fast_opt.optimize)
        if guards:
            machine.generic_ops = generic
            machine.guards = guards
            ops = special
        else:
            ops = generic
    return machine, ops


def check(names=None, optimize=True, work=Path('/tmp/digimd/fastcheck')):
    work.mkdir(parents=True, exist_ok=True)
    instructions = decode(OUT / 'decoded.tsv')
    img = image()
    results = {}
    for m in machines(names):
        p = m['prefix']
        machine, ops = lift(m, instructions, img, optimize)
        fn = ''
        if machine.generic_ops is not None:
            fn = emit_c(p + '_render_fast_g', m['voice_type'], machine.generic_ops, fallback=p + '_render_block_44k1')
        fn += emit_c(p + '_render_fast', m['voice_type'], ops, fallback=p + '_render_block_44k1', generic=p + '_render_fast_g')
        src = host_source(m).replace(f'if (row[0]) {p}_render_block_44k1(&v, sine);', f'if (row[0]) {p}_render_fast(&v, sine);')
        marker = '#include <stdio.h>\nuint32_t md_source_words'
        src = src.replace(marker, fn + marker, 1)
        path = work / f'{p}.c'
        path.write_text(src)
        exe = work / p
        subprocess.run(['clang', '-O1', '-w', '-o', str(exe), str(path), str(ROOT / 'tools/machines/generated/control_tables.c')], check=True)
        folder = OUT / m['name']
        actual = work / f'{p}.raw'
        r = subprocess.run([str(exe), str(folder / 'schedule.raw'), str(actual), str(folder / 'sine.raw')], timeout=60)
        same = r.returncode == 0 and actual.read_bytes() == (folder / 'original.raw').read_bytes()
        results[m['name']] = dict(status='PASS' if same else 'FAIL', ops=len(ops), returncode=r.returncode)
        print(m['name'], results[m['name']], flush=True)
    return results


if __name__ == '__main__':
    a = argparse.ArgumentParser()
    a.add_argument('--machines', nargs='*')
    a.add_argument('--no-opt', action='store_true')
    x = a.parse_args()
    r = check(x.machines, not x.no_opt)
    raise SystemExit(0 if all(v['status'] == 'PASS' for v in r.values()) else 1)
