#!/usr/bin/env python3
"""Build the *real original-code* Machinedrum DSP2 reference and capture PCM.

No Docker, hardware flashing, CI, firmware approximation or Gearmulator GUI.
The pinned upstream Machinemodule checkout supplies genuine DSP56303 emulation,
original MD firmware coefficient functions, and its DSP2-only voice tap.

A first-time build needs network access to obtain public Git submodules. Use
--source with an existing complete checkout on an offline machine. --plan-only
prints the exact commands without cloning, compiling or generating fake audio.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from tools.md_reference import plan, capture, ReferenceError

UPSTREAM = 'https://github.com/sd88me/mpc-vst-machinedrum.git'
PIN = '6ea7a4adfcfacd1dc62fcac1cfea31b488f96d8f'


def command_plan(source: Path, build: Path, *, clone: bool) -> list[list[str]]:
    source, build = source.resolve(), build.resolve()
    steps: list[list[str]] = []
    if clone:
        steps.append(['git', 'clone', '--no-checkout', UPSTREAM, str(source)])
        steps.append(['git', '-C', str(source), 'checkout', '--detach', PIN])
    steps += [
        ['git', '-C', str(source), 'submodule', 'update', '--init', '--recursive', '--jobs', '4'],
        ['cmake', '-S', str(ROOT / 'tools/reference_host'), '-B', str(build), '-G', 'Ninja',
         '-DCMAKE_BUILD_TYPE=Release', '-DCMAKE_CXX_FLAGS=-DDSP56K_NO_JIT_RUNTIME',
         f'-DDIGIMD_MACHINEMODULE_ROOT={source}'],
        ['cmake', '--build', str(build), '--parallel', '4', '--target', 'digimd_mdhost', 'digimd_mdvoice', 'digimd_mdsequence'],
    ]
    return steps


def invoke(cmd: list[str], timeout: int | None = None) -> None:
    print('RUN:', ' '.join(cmd), flush=True)
    subprocess.run(cmd, check=True, timeout=timeout)


def build_and_capture(md: Path, source: Path, build: Path, out: Path, *,
                      machine: str, syn: tuple[int, ...] | None,
                      blocks: int, plan_only: bool,
                      skip_voice: bool, timeout: int) -> dict:
    # Validate the exact proprietary input before attempting a clone or build.
    reference = plan(md, machine, syn, blocks, out)
    clone = not source.exists()
    commands = command_plan(source, build, clone=clone)
    report: dict = {
        'schema': 'digimd.reference-build.v1',
        'upstream': UPSTREAM,
        'upstream_revision_required': PIN,
        'source_md_sha256': reference['source_md_sha256'],
        'md_machine': reference['machine'],
        'original_dsp2_only': True,
        'network_needed_for_clone': clone,
        'commands': commands,
        'status': 'PLAN ONLY: no original PCM produced',
    }
    if plan_only:
        return report
    for exe in ('git', 'cmake', 'ninja', 'g++'):
        if shutil.which(exe) is None:
            raise ReferenceError(f'missing local executable: {exe}')
    if clone:
        source.parent.mkdir(parents=True, exist_ok=True)
        invoke(commands[0], timeout)
        invoke(commands[1], timeout)
    if not (source / '.git').exists():
        raise ReferenceError(f'not a Git checkout: {source}')
    head = subprocess.check_output(['git', '-C', str(source), 'rev-parse', 'HEAD'], text=True).strip()
    if head != PIN:
        raise ReferenceError(f'upstream revision mismatch: got {head}, expected {PIN}; refusing unpinned build')
    # The first two steps, if present, were the clone and pinned checkout.
    for cmd in commands[2 if clone else 0:]:
        invoke(cmd, timeout)
    host = build / 'digimd_mdhost'
    voice = build / 'digimd_mdvoice'
    if not host.is_file() or (not skip_voice and not voice.is_file()):
        raise ReferenceError('CMake completed without producing expected original-code renderers')
    # A legitimate reference requires actual non-silent 24-bit DSP2 samples.
    full_plan = plan(md, machine, syn, blocks, out, mdhost=host,
                     mdvoice=None if skip_voice else voice)
    result = capture(full_plan, mdhost=host, mdvoice=None if skip_voice else voice,
                     out_dir=out, timeout=timeout)
    report['status'] = 'ORIGINAL DSP2 PCM CAPTURED (NOT DNII firmware)'
    report['upstream_revision_actual'] = head
    report['reference_sha256'] = result['reference_sha256']
    report['reference_frames'] = result['frames']
    report['reference_nonzero_frames'] = result['reference_nonzero_frames']
    if 'mdvoice' in result:
        report['mdvoice_comparison'] = result['mdvoice']['comparison']
    return report


def main(argv=None) -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--md', type=Path, required=True)
    p.add_argument('--source', type=Path, default=ROOT / 'out/external/mpc-vst-machinedrum')
    p.add_argument('--build', type=Path, default=ROOT / 'out/build/md-reference')
    p.add_argument('--out-dir', type=Path, default=ROOT / 'out/pcm/trx_bd')
    p.add_argument('--machine', default='TRX-BD')
    p.add_argument('--syn', help='eight comma-separated raw SYN values, 0..127; otherwise MD defaults')
    p.add_argument('--blocks', type=int, default=256)
    p.add_argument('--plan-only', action='store_true')
    p.add_argument('--skip-voice', action='store_true',
                   help='skip independent mdvoice comparison, which has different control-tick stimulus')
    p.add_argument('--timeout', type=int, default=1800, help='per subprocess timeout (seconds)')
    args = p.parse_args(argv)
    try:
        syn = tuple(int(v) for v in args.syn.split(',')) if args.syn else None
        report = build_and_capture(args.md, args.source, args.build, args.out_dir,
                                   machine=args.machine, syn=syn, blocks=args.blocks,
                                   plan_only=args.plan_only,
                                   skip_voice=args.skip_voice, timeout=args.timeout)
        args.out_dir.mkdir(parents=True, exist_ok=True)
        dest = args.out_dir / 'build_report.json'
        dest.write_text(json.dumps(report, indent=2) + '\n', encoding='utf-8')
        print(f"{report['status']} | {dest}")
        return 0
    except (ReferenceError, OSError, ValueError, subprocess.CalledProcessError,
            subprocess.TimeoutExpired) as exc:
        print(f'build_md_reference: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    raise SystemExit(main())
