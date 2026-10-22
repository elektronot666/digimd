"""Rebuild a native49 profile after a dispatcher-only change.

Every engine, resampler, table and helper object is reused byte-for-byte from
an existing source-bound profile (default: the qualified Full49 pacing
profile). Only the dispatcher is recompiled and the md_cycles/audio pacing
leaves assembled;
the link and every post-link step (stock stream patches, region reservations,
asset payload, history/route fills, aPLib bundle, build proof) are the same
code as `tools.machines.resident.native`. Use the full build when any engine
source changes.

    python3 -m tools.machines.resident.relink --base out/audio-fix-full49-pacing \
        --out out/audio-fix-full49-routing
"""
import argparse
import json
from pathlib import Path
import re
import shutil
import subprocess

from tools.machines.registry import ROOT
from tools.machines.compact.build import sha
from tools.machines.resident import native as N
from tools.trxbd_sharc import build_sharc as B

HERE = Path(__file__).resolve().parent
LINKER = ROOT / 'out/machines/resident49/toolchain/target/release/seld'


def layout_constants(dispatch_c):
    text = Path(dispatch_c).read_text()
    m = re.search(r'static const uint8_t md_history_words\[49\]=\{[^}]*\};\n#define MD_PRIVATE_LOW 0x[0-9a-f]{8}u\n#define MD_PRIVATE_HIGH 0x[0-9a-f]{8}u\n', text)
    if not m:
        raise SystemExit('Base dispatcher lacks generated layout constants')
    return m.group(0)


def relink(base, output, linker=LINKER, verify_reproduction=False):
    base = Path(base).resolve(); output = Path(output).resolve()
    old_proof = json.loads((base / 'build-proof.json').read_text())
    old_objects = json.loads((base / 'objects.json').read_text())
    output.mkdir(parents=True, exist_ok=True)
    for name in ('dispatch.h', 'descriptors.h', 'engine_slot.inc', 'linker.json', 'profile.ldf'):
        shutil.copyfile(base / name, output / name)
    constants = layout_constants(base / 'dispatch.c')
    template = (HERE / 'dispatch49.c').read_text()
    if template.count('@LAYOUT_CONSTANTS@') != 1:
        raise SystemExit('dispatch49.c template marker missing')
    (output / 'dispatch.c').write_text(template.replace('@LAYOUT_CONSTANTS@', constants))
    actual = dict(old_proof['actual_compilation'])
    log = (output / 'build.log').open('w')

    def run(cmd):
        # selcc can report an unsupported construct and still exit 0 with a
        # truncated assembly file: treat any diagnostic as a failure
        r = subprocess.run([str(B.SELACHE_BIN / cmd[0]), *map(str, cmd[1:])], check=True, capture_output=True, text=True)
        log.write(r.stdout + r.stderr)
        if (r.stdout + r.stderr).strip():
            log.flush()
            raise SystemExit(f'{cmd[0]} reported: {(r.stdout + r.stderr).strip()[:300]}')
    asm = output / 'dispatch.s'; obj = output / 'dispatch.doj'
    run(['selcc', '-proc', 'ADSP-21569', '-char-size-8', '-O1', '-I' + str(output), '-S', '-o', asm, output / 'dispatch.c'])
    run(['selas', '-proc', 'ADSP-21569', '-o', obj, asm])
    actual['dispatch'] = dict(source_sha256=sha(output / 'dispatch.c'), assembly_sha256=sha(asm), object_sha256=sha(obj))
    cycles = output / 'md_cycles.doj'
    run(['selas', '-proc', 'ADSP-21569', '-o', cycles, HERE / 'md_cycles.s'])
    pacing = output / 'audio_pacing.doj'
    run(['selas', '-proc', 'ADSP-21569', '-o', pacing, HERE / 'audio_pacing.s'])
    objects = []
    for rel, digest in old_objects.items():
        path = ROOT / rel
        if sha(path) != digest:
            raise SystemExit(f'Reused object changed: {rel}')
        if path.name == 'dispatch.doj' and path.parent == base:
            objects.append(obj)
            continue
        if path.name == 'audio_pacing.doj' and path.parent == base:
            objects.extend((pacing, cycles))
            continue
        objects.append(path)
    if obj not in objects or cycles not in objects:
        raise SystemExit('Base object list lacks dispatch/audio pacing objects')
    subprocess.run([str(linker), '-proc', 'ADSP-21569', '-T', str(output / 'profile.ldf'), '-o', str(output / 'profile.dxe'),
                    *map(str, objects)], check=True, stdout=log, stderr=log)
    log.close()
    (output / 'objects.json').write_text(json.dumps({str(p.relative_to(ROOT)): sha(p) for p in objects}, indent=2) + '\n')
    sources = dict(old_proof['sources'])
    for p in (HERE / 'dispatch49.c', HERE / 'md_cycles.s', HERE / 'audio_pacing.s', HERE / 'native.py', HERE / 'relink.py'):
        sources[str(p.relative_to(ROOT))] = sha(p)
    original_inputs = N.inputs
    N.inputs = lambda: sources
    try:
        proof = N.finish(output, sources, old_proof['optimized_shared_kernels'], actual, linker)
    finally:
        N.inputs = original_inputs
    proof = json.loads((output / 'build-proof.json').read_text())
    proof.update(build_route='dispatcher relink over reused engine objects', base_profile=str(base.relative_to(ROOT)),
                 base_stream_sha256=old_proof['stream_sha256'])
    (output / 'build-proof.json').write_text(json.dumps(proof, indent=2) + '\n')
    return proof


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--base', type=Path, default=ROOT / 'out/audio-fix-full49-pacing')
    p.add_argument('--out', type=Path, default=ROOT / 'out/audio-fix-full49-routing')
    p.add_argument('--linker', type=Path, default=LINKER)
    a = p.parse_args()
    r = relink(a.base, a.out, a.linker)
    print('Relinked native49:', r['stream_sha256'])
