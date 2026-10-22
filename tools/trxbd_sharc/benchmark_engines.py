#!/usr/bin/env python3
"""Compare native DigiMD TRX-BD, stock FM Tone and stock FM Drum under sharcvm.

All engines execute inside the SAME patched Digitone II section-7 image and use
identical trigger timing, pitch/velocity, native AMP/filter defaults and disabled
sends. Results are SHARC INTERPRETER INSTRUCTION COUNTS, not hardware cycles or
proof of physical headroom. The stock engines retain their own native SYN
presets; TRX-BD uses the original Machinedrum default SYN preset.

Run from the digimd repository root (after the documented sharcvm setup):
  python3 tools/trxbd_sharc/benchmark_engines.py \
      --section7 out/trxbd_sharc/section7_trxbd.bin \
      --controls out/audio-qualification/stock-controls.json \
      --out out/benchmarks/dn2-engine-instructions.json
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import statistics
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.source_identity import source_revision

# Match DigiMD's existing SHARC benchmark toolchain import setup. The native
# VM and runner live in the ignored out/external/dn2-cfw checkout, not site-packages.
DN2_CFW = ROOT / "out/external/dn2-cfw"
for helper in (DN2_CFW / "tools/sharcvm", DN2_CFW / "tools/dspsplice"):
    if str(helper) not in sys.path:
        sys.path.insert(0, str(helper))

ENGINE_TYPES = {"FM Tone": 0, "FM Drum": 2, "TRX-BD": 5}
VOICE_COUNTS = (1, 4, 8, 16)
TRX_DEFAULT_SYN = (64, 64, 0, 0, 0, 0, 0, 0)
TRX_PARAM_OFFSETS = (250, 252, 244, 248, 254, 256, 262, 260)
FIXED_VOICE_FRAME_OFFSETS = (
    (302, 0x7f00), (306, 0x4000), (322, 0x7f00),
    (332, 0x7f00), (334, 0x7f00), (336, 0x7f00), (338, 0x7f00),
    (346, 0x4000), (348, 0x6400), (352, 0x0100),
)


def sha256_bytes(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def percentile(values: list[int], p: float) -> int:
    if not values:
        raise ValueError('Cannot summarize zero blocks')
    ordered = sorted(values)
    return ordered[min(len(ordered) - 1, max(0, math.ceil(p * len(ordered)) - 1))]


def stats(values: list[int]) -> dict:
    if not values:
        raise ValueError('Cannot summarize zero blocks')
    return {
        "min": min(values), "median": statistics.median(values),
        "mean": statistics.fmean(values), "p95": percentile(values, .95),
        "p99": percentile(values, .99), "max": max(values),
    }


def relative_stats(active: list[int], baseline: list[int]) -> dict:
    """Paired per-block subtraction, not subtraction of unrelated percentiles."""
    if len(active) != len(baseline) or not active:
        raise ValueError('Require aligned, nonempty baseline and workload')
    return stats([x - y for x, y in zip(active, baseline)])


def configure_engine(runner, controls: dict, machine: int, count: int,
                     trx_syn=TRX_DEFAULT_SYN) -> None:
    """Reuse digimd's already verified stock frame/AMP/filter construction."""
    from tools.trxbd_sharc.integration import configure
    if count < 0 or count > 16:
        raise ValueError('Voice count must be 0..16')
    frame = runner.d.frame
    configure(frame, controls, [machine] * count)
    for voice in range(16):
        # The stock integration fixture defines note 48+3*voice. Keep that
        # exact proven pitch schedule for every engine and every voice count.
        struct.pack_into('<H', frame, 2 + 2 * voice, 48 + 3 * voice)
        for off, val in FIXED_VOICE_FRAME_OFFSETS:
            struct.pack_into('<H', frame, off + 146 * voice, val)
        for off in (340, 342, 344):  # chorus/delay/reverb sends
            struct.pack_into('<H', frame, off + 146 * voice, 0)
        if machine == 5 and voice < count:
            for off, value in zip(TRX_PARAM_OFFSETS, trx_syn):
                struct.pack_into('<H', frame, off + 146 * voice, value << 8)
    # Explicitly zero all return/master FX and external input levels, even if
    # an upstream helper's defaults change. PATTERN_VOL is held constant.
    runner.set(CHR_VOL=0, DEL_VOL=0, REV_VOL=0,
               IN_L=0, IN_R=0, IN_CHR_L=0, IN_CHR_R=0,
               IN_DEL_L=0, IN_DEL_R=0, IN_REV_L=0, IN_REV_R=0,
               PATTERN_VOL=127)


def measure_case(section7: bytes, controls: dict, *, name: str, machine: int,
                 voices: int, blocks: int, warmup: int, retrigger_every: int,
                 control_events: bool = True, trx_syn=TRX_DEFAULT_SYN) -> dict:
    """Execute the entire patched stock DSP block handler with real voice PCM.

    'MIDI event baseline' uses machine type 4 with matching note/filter/AMP
    triggers (not a reference audio engine). 'Idle' sends no triggers.
    """
    import vm as V
    import vmrun
    import fullblock

    vm = V.VM()
    vm.load_dn2()
    vmrun.load_stream(vm, section7)
    vm.call(vmrun.FXINIT, max_insns=10**9)
    runner = vmrun.Runner(vm, {"PATTERN_VOL": 127})
    configure_engine(runner, controls, machine, voices, trx_syn)
    frame = runner.d.frame
    peak_per_voice = [0.0] * voices
    peak_master = 0.0
    recorded: list[int] = []
    block_types: list[str] = []
    amp_silent_blocks = 0
    inputs = [0.0] * 32
    for b in range(blocks + warmup):
        trigger_mask = ((1 << voices) - 1) if control_events and b % retrigger_every == 0 else 0
        for off in (34, 38, 40):
            struct.pack_into('<H', frame, off, trigger_mask)
        left, right = runner.block(inputs, inputs, capture=False)
        if not all(math.isfinite(x) for x in left + right):
            raise AssertionError(f'{name} {voices} voices: non-finite master audio in block {b}')
        if b < warmup:
            continue
        is_trigger = (b % retrigger_every == 0)
        block_types.append('trigger' if is_trigger else 'body')
        recorded.append(runner.per_block[-1])
        peak_master = max(peak_master, *(abs(x) for x in left + right))
        any_active_audio = False
        for t in range(voices):
            buf = vm.rd32(0x254a60 + 4 * t)
            if not buf:
                raise AssertionError(f'{name} voice {t} lacks a buffer pointer')
            samples = vm.rdf(buf, 32)
            if not all(math.isfinite(x) for x in samples):
                raise AssertionError(f'{name} voice {t} emitted non-finite PCM in block {b}')
            peak = max(map(abs, samples))
            peak_per_voice[t] = max(peak_per_voice[t], peak)
            any_active_audio = any_active_audio or peak > 0
        if not any_active_audio:
            amp_silent_blocks += 1
    if machine in ENGINE_TYPES.values():
        silent = [t for t, peak in enumerate(peak_per_voice) if peak <= 1e-12]
        if silent or peak_master <= 1e-12:
            raise AssertionError(f'{name} {voices} voices produced silent outputs: '
                                 f'silent_voices={silent}, master_peak={peak_master}')
    elif any(peak > 1e-12 for peak in peak_per_voice):
        raise AssertionError(f'Control {name} unexpectedly generated audio')
    trigger_cost = [n for n, phase in zip(recorded, block_types) if phase == 'trigger']
    body_cost = [n for n, phase in zip(recorded, block_types) if phase == 'body']
    return {
        "name": name, "machine_type": machine, "voices": voices,
        "trx_syn": list(trx_syn) if machine == 5 else None,
        "blocks_measured": blocks, "warmup_blocks": warmup,
        "retrigger_every_blocks": retrigger_every, "recorded_trigger_blocks": len(trigger_cost),
        "measured_total": stats(recorded),
        "measured_trigger": stats(trigger_cost) if trigger_cost else None,
        "measured_body": stats(body_cost) if body_cost else None,
        "voice_pcm_peaks": peak_per_voice, "master_peak": peak_master,
        "all_voices_non_silent": all(x > 1e-12 for x in peak_per_voice),
        "blocks_with_no_voice_pcm": amp_silent_blocks,
        "per_block_instructions": recorded,
    }


def make_comparison(cases: list[dict], engine_types=None) -> dict:
    engine_types = ENGINE_TYPES if engine_types is None else engine_types
    rows = []
    for voices in VOICE_COUNTS:
        group = {case["name"]: case for case in cases if case["voices"] == voices}
        baseline = group['MIDI event control']["per_block_instructions"]
        for name in engine_types:
            result = group[name]
            total = result['measured_total']
            delta = relative_stats(result['per_block_instructions'], baseline)
            rows.append({"machine": name, "voices": voices,
                         "total_median": total['median'], "total_max": total['max'],
                         "total_p99": total['p99'],
                         "paired_minus_midi_control_median": delta['median'],
                         "paired_minus_midi_control_max": delta['max'],
                         "paired_minus_midi_control_p99": delta['p99'],
                         "all_voices_non_silent": result['all_voices_non_silent'],
                         "master_peak": result['master_peak']})
    for voices in VOICE_COUNTS:
        by_machine = {r['machine']: r for r in rows if r['voices'] == voices}
        fm_tone = by_machine['FM Tone']['paired_minus_midi_control_median']
        for r in rows:
            if r['voices'] == voices:
                r['relative_delta_vs_fm_tone_pct'] = (
                    100 * r['paired_minus_midi_control_median'] / fm_tone
                    if fm_tone > 0 else None
                )
    return {"rows": rows}


def run(section7: Path, controls_path: Path, *, blocks: int = 96,
        warmup: int = 32, retrigger_every: int = 32,
        expected_section7_sha256: str | None = None,
        trx_patches=('default',)) -> dict:
    if blocks < 64 or warmup < 0 or retrigger_every < 2:
        raise ValueError('Require blocks>=64, warmup>=0, retrigger_every>=2')
    if blocks // retrigger_every < 2:
        raise ValueError('At least two retriggers needed in recorded window')
    if warmup % retrigger_every or blocks % retrigger_every:
        raise ValueError('Warmup and blocks must be multiples of retrigger interval')
    from tools.trxbd_sharc.validate_kernel import PATCHES
    if not trx_patches or len(set(trx_patches)) != len(trx_patches):
        raise ValueError('Require nonempty, unique TRX-BD patch names')
    presets = {('TRX-BD' if patch == 'default' else f'TRX-BD {patch}'): PATCHES[patch]
               for patch in trx_patches}
    engine_types = {'FM Tone': 0, 'FM Drum': 2, **{name: 5 for name in presets}}
    sec = section7.read_bytes()
    section_hash = sha256_bytes(sec)
    if expected_section7_sha256 and section_hash != expected_section7_sha256:
        raise ValueError(f'Section7 SHA mismatch: {section_hash} != {expected_section7_sha256}')
    controls_bytes = controls_path.read_bytes()
    controls = json.loads(controls_bytes)
    if controls['global_frame']['master_tuning_tenths_hz'] != 4400:
        raise ValueError('Unexpected stock master tuning; fixture could render silence')
    if len(controls['machines']) < 4:
        raise ValueError('Stock control fixture missing FM machine descriptors')
    cases = []
    # No-voice reference for absolute whole-handler cost.
    cases.append(measure_case(sec, controls, name='MIDI idle', machine=4, voices=0,
                              blocks=blocks, warmup=warmup, retrigger_every=retrigger_every,
                              control_events=False))
    for voices in VOICE_COUNTS:
        case_list = [('MIDI event control', 4)] + list(engine_types.items())
        for name, machine in case_list:
            item = measure_case(sec, controls, name=name, machine=machine, voices=voices,
                                blocks=blocks, warmup=warmup, retrigger_every=retrigger_every,
                                trx_syn=presets.get(name, TRX_DEFAULT_SYN))
            cases.append(item)
            print(f'{name:18s} {voices:2d} voices: median={item["measured_total"]["median"]:,.0f} '
                  f'p99={item["measured_total"]["p99"]:,} '
                  f'peak={item["measured_total"]["max"]:,} '
                  f'master_peak={item["master_peak"]:.6f}', flush=True)
    return {
        "schema": "digimd.native-engine-instruction-comparison.v1",
        "measurement": "Executed SHARC interpreter instructions, whole block handler",
        "not_measurement": ["Physical SHARC cycles", "DSP deadline or available headroom",
                            "CPU load", "Comparable DSP arithmetic primitives per sample"],
        "scope": "Identical native patched section7, pitch/velocity/AMP/filter/FX and note events; original stock SYN defaults versus listed authentic TRX-BD SYN presets; no IRQ/DMA/cache timing",
        "source_commit": source_revision(),
        "section7_path": str(section7), "section7_sha256": section_hash,
        "controls_path": str(controls_path), "controls_sha256": sha256_bytes(controls_bytes),
        "blocks": blocks, "warmup": warmup, "retrigger_every": retrigger_every,
        "voice_counts": list(VOICE_COUNTS), "engines": engine_types,
        "trx_syn_presets": {name: list(syn) for name, syn in presets.items()},
        "idle_reference": cases[0]['measured_total'],
        "comparison": make_comparison(cases, engine_types),
        "cases": cases,
    }


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--section7', type=Path, default=ROOT / 'out/trxbd_sharc/section7_trxbd.bin')
    p.add_argument('--controls', type=Path, default=ROOT / 'out/audio-qualification/stock-controls.json')
    p.add_argument('--expected-section7-sha256', default=None,
                   help='Refuse mismatched/stale SHARC builds')
    p.add_argument('--blocks', type=int, default=96)
    p.add_argument('--warmup', type=int, default=32)
    p.add_argument('--retrigger-every', type=int, default=32)
    p.add_argument('--trx-patches', nargs='+', default=['default'],
                   choices=['default', 'all8', 'harm_clip', 'noise', 'min', 'max'])
    p.add_argument('--out', type=Path, default=ROOT / 'out/benchmarks/dn2-engine-instructions.json')
    args = p.parse_args()
    result = run(args.section7, args.controls, blocks=args.blocks,
                 warmup=args.warmup, retrigger_every=args.retrigger_every,
                 expected_section7_sha256=args.expected_section7_sha256,
                 trx_patches=args.trx_patches)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(result, indent=2) + '\n')
    print('\nWHOLE DSP HANDLER / INSTRUCTIONS PER BLOCK:')
    print(f'{"Engine":<12s} {"Voices":>6s} {"Median":>11s} {"P99":>11s} {"Max":>11s} '
          f'{"Delta vs MIDI":>15s} {"vs FM Tone":>10s}')
    for row in result['comparison']['rows']:
        pct = row['relative_delta_vs_fm_tone_pct']
        print(f'{row["machine"]:<12s} {row["voices"]:6d} {row["total_median"]:11,.0f} '
              f'{row["total_p99"]:11,d} {row["total_max"]:11,d} '
              f'{row["paired_minus_midi_control_median"]:15,.0f} '
              f'{pct:9.1f}%' if pct is not None else f'{row["machine"]:<12s} {row["voices"]:6d} '
              f'{row["total_median"]:11,.0f} {row["total_p99"]:11,d} {row["total_max"]:11,d} '
              f'{row["paired_minus_midi_control_median"]:15,.0f} {"N/A":>10s}')
    print(f'\nFull reproducibility report: {args.out}')
    print('Note: deltas include differences in stock voice paths; they are not isolated synthesis-only cycles.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
