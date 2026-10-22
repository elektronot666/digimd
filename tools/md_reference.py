#!/usr/bin/env python3
"""Capture a *real original-code* Machinedrum SYN-only PCM reference.

Uses sd88me/mpc-vst-machinedrum's locally built `mdhost` binary (original
68k coefficient functions in Musashi + original DSP2 in DSP56300 emulator).
It explicitly never uses the Machinedrum mixer DSP1. No fake waveform and no
approximate synthesis implementation is generated here.

Optional `mdvoice` captures DSP2 driven directly by DigiMD's 68k interpreter
coefficients for an independent comparison of the first trigger. A disagreement
is useful evidence, not automatically a failure of the original reference.

Example:
 python tools/md_reference.py --md /path/Elektron_SPS1-1UW_OS1.63.syx \\
   --mdhost /path/mpc-vst-machinedrum/build-proto/mdhost \\
   --machine TRX-BD --blocks 256 --out-dir out/pcm/trx_bd

Use `--plan-only` to write command and coefficient plan without invoking tools.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))

from digimd import load_firmware, MD_DEVICE, MD_SHA256, sha  # noqa: E402
from digimd_md68k import MD68K  # noqa: E402
from digimd_pcm import MD_SAMPLE_RATE, MD_BLOCK_FRAMES, PCMError, load_pcm, compare_pcm  # noqa: E402
from tools.extract_md_machines import machine_report  # noqa: E402


class ReferenceError(RuntimeError):
    pass


def sha_file(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1 << 20), b''):
            h.update(block)
    return h.hexdigest()


def plan(md_path: Path, name: str, syn: tuple[int, ...] | None, blocks: int,
         out_dir: Path, mdhost: Path | None = None, mdvoice: Path | None = None) -> dict:
    if blocks < 1 or blocks > 1_000_000:
        raise ReferenceError('blocks must be in 1..1,000,000')
    md = load_firmware(md_path, MD_DEVICE)
    if md.digest != MD_SHA256:
        raise ReferenceError('expected the known Machinedrum OS 1.63 image; SHA-256 differs')
    machines = machine_report(md, ROOT / 'data/machine_catalog.csv')['machines']
    find = [m for m in machines if m['display_name'].upper() == name.upper() or str(m['md_id']) == name]
    if len(find) != 1:
        raise ReferenceError('unknown or excluded synthesis machine: ' + name)
    m = find[0]
    if syn is None:
        syn = tuple(s['default_raw'] for s in m['syn'])
    if len(syn) != 8 or any(not 0 <= p <= 127 for p in syn):
        raise ReferenceError('SYN must contain eight integer values in 0..127')
    cpu = MD68K(md.section_bytes(0))
    count, words = cpu.coefficients(m, syn, trigger=True)
    if not 0 <= count <= 32:
        raise ReferenceError('unexpected coefficient word count')
    # Original DSP2 slot word 0 carries the machine selection on trigger.
    # The coefficient routine's out[0] is only a trigger flag, not the ID.
    slot = [int(m['md_id']) + 1]
    slot += [int(word) & 0xffffff for word in words[1:count]]
    outfile = (out_dir / 'mdhost.raw').resolve()
    mdhost_binary = (mdhost or Path('mdhost')).resolve()
    mdvoice_binary = (mdvoice or Path('mdvoice')).resolve()
    host_cmd = [str(mdhost_binary), str(md_path.resolve()), str(outfile),
                '0', str(m['md_id']), str(blocks)] + [str(p) for p in syn]
    voice_cmd = [str(mdvoice_binary), str(md_path.resolve()),
                 str((out_dir / 'mdvoice.raw').resolve()), str(blocks), '0'] + [f'{w:x}' for w in slot]
    return {
        'schema': 'digimd.reference-plan.v1',
        'provenance': 'original MD 1.63 ColdFire + DSP2; no MD DSP1 effects or mixer',
        'source_md_sha256': md.digest,
        'source_md_dsp2_sha256': sha(md.section_bytes(1)),
        'source_md_control_sha256': sha(md.section_bytes(0)),
        'machine': m['display_name'], 'machine_id': m['md_id'],
        'coefficient_function': m['coefficient_function_address'],
        'syn': list(syn), 'syn_labels': [p['label'] for p in m['syn']],
        'coefficient_executor': 'digimd bounded 68k VM; independent verification still required',
        'coefficient_word_count': count,
        'coefficient_words_24': [f'{w & 0xffffff:06x}' for w in words[:max(count, 1)]],
        'slot_words_24': [f'{w:06x}' for w in slot],
        'blocks': blocks, 'frames': blocks * MD_BLOCK_FRAMES,
        'sample_rate': MD_SAMPLE_RATE, 'pcm_format': 's24in32le',
        'mdhost_command': host_cmd,
        'mdvoice_command': voice_cmd,
    }


def capture(reference_plan: dict, *, mdhost: Path, mdvoice: Path | None,
            out_dir: Path, overwrite: bool = False, timeout: int = 600) -> dict:
    out_dir.mkdir(parents=True, exist_ok=True)
    outputs = [out_dir / 'mdhost.raw', out_dir / 'reference.json']
    if mdvoice is not None:
        outputs.append(out_dir / 'mdvoice.raw')
    if not overwrite and any(p.exists() for p in outputs):
        raise ReferenceError('output already exists; pass --overwrite or choose a new directory')
    if not mdhost.is_file():
        raise ReferenceError(f'mdhost executable not found: {mdhost}; build upstream tools/build_proto.sh first')
    if mdvoice is not None and not mdvoice.is_file():
        raise ReferenceError(f'mdvoice executable not found: {mdvoice}')
    env = dict(os.environ)
    for key in ('MDHOST_SET', 'MDHOST_LFO', 'MDHOST_LEVEL', 'MDHOST_VEL', 'MDHOST_SLOTS'):
        env.pop(key, None)

    def run(cmd):
        try:
            r = subprocess.run(cmd, env=env, capture_output=True, text=True, timeout=timeout,
                               check=True)
        except (subprocess.TimeoutExpired, subprocess.CalledProcessError, OSError) as e:
            raise ReferenceError(f'reference executable failed: {e}') from e
        return {'stdout': r.stdout[-4000:], 'stderr': r.stderr[-4000:]}

    host_run = run(reference_plan['mdhost_command'])
    host_audio = load_pcm(out_dir / 'mdhost.raw', 's24in32le', MD_SAMPLE_RATE)
    expected_frames = reference_plan['frames']
    if host_audio.frames != expected_frames:
        raise ReferenceError(f'mdhost rendered {host_audio.frames} frames, expected {expected_frames}')
    if host_audio.nonzero == 0:
        raise ReferenceError('mdhost output is silent: invalid reference for an audible machine')
    report = {key: value for key, value in reference_plan.items()
              if key not in ('mdhost_command', 'mdvoice_command')}
    report.update({
        'schema': 'digimd.reference-capture.v1',
        'status': 'recorded_original_dsp2_reference_NOT_verified_against_real_hardware',
        'reference_pcm': 'mdhost.raw',
        'reference_sha256': host_audio.digest,
        'reference_nonzero_frames': host_audio.nonzero,
        'reference_peak_abs_int24': max(abs(x) for x in host_audio.values),
        'mdhost_binary_sha256': sha_file(mdhost),
        'mdhost_log': host_run,
    })
    if mdvoice is not None:
        voice_run = run(reference_plan['mdvoice_command'])
        voice_audio = load_pcm(out_dir / 'mdvoice.raw', 's24in32le', MD_SAMPLE_RATE)
        if voice_audio.frames != expected_frames:
            raise ReferenceError('mdvoice rendered unexpected number of samples')
        report['mdvoice'] = {
            'pcm': 'mdvoice.raw', 'sha256': voice_audio.digest,
            'binary_sha256': sha_file(mdvoice), 'log': voice_run,
            'comparison': compare_pcm(host_audio, voice_audio),
            'warning': ('A fixed initial DSP slot vs mdhost periodic coefficient updates are not '
                        'necessarily the same stimulus; mismatches require examining control ticks.'),
        }
    (out_dir / 'reference.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def main(argv=None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--md', type=Path, required=True)
    parser.add_argument('--machine', default='TRX-BD')
    parser.add_argument('--syn', help='exactly eight comma-separated raw 0..127 values; default to OS defaults')
    parser.add_argument('--blocks', type=int, default=256)
    parser.add_argument('--out-dir', type=Path, default=Path('out/pcm/trx_bd'))
    parser.add_argument('--mdhost', type=Path, help='locally built upstream mdhost executable')
    parser.add_argument('--mdvoice', type=Path, help='optional DSP2-only executable for differential test')
    parser.add_argument('--plan-only', action='store_true')
    parser.add_argument('--overwrite', action='store_true')
    args = parser.parse_args(argv)
    try:
        syn = tuple(int(x) for x in args.syn.split(',')) if args.syn else None
        result = plan(args.md, args.machine, syn, args.blocks, args.out_dir,
                      mdhost=args.mdhost, mdvoice=args.mdvoice)
        if args.plan_only:
            args.out_dir.mkdir(parents=True, exist_ok=True)
            (args.out_dir / 'plan.json').write_text(json.dumps(result, indent=2) + '\n')
            print(f"PLAN ONLY: {result['machine']}, {result['frames']} DSP2 frames; no audio rendered")
            print(f"commands stored in {args.out_dir / 'plan.json'}")
        else:
            if args.mdhost is None:
                raise ReferenceError('provide --mdhost /path/to/build-proto/mdhost or use --plan-only')
            capture(result, mdhost=args.mdhost, mdvoice=args.mdvoice, out_dir=args.out_dir,
                    overwrite=args.overwrite)
            print(f"REFERENCE RECORDED: {args.out_dir / 'reference.json'}")
        return 0
    except (ReferenceError, ValueError, PCMError, OSError) as exc:
        print(f'md_reference: {exc}', file=sys.stderr)
        return 2


if __name__ == '__main__':
    sys.exit(main())
