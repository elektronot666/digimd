"""Deterministic frame-level stock-chain and machine-replacement regressions.

The actual compiled hook and entire stock DSP handler run in sharcvm. A second
handler receives independently generated dry PCM at the same synthesis seam.
Matching FX history makes exact comparisons possible even during tails.
ColdFire frame production, organic pattern/project loading, IRQs and DMA timing
are explicitly outside this harness; no full-device integration PASS is implied.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import struct
import sys
import wave

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.trxbd_sharc.build_sharc import (
    VOICE_RESUME_SW,
)
from tools.trxbd_sharc.integrity import trace_hook
from digimd import MD_DEVICE, load_firmware
from digimd_trxbd_dsp import (TRXBDDSP56303Voice, from_firmware, build_dsp2_sine_table,
                              extract_trxbd_cf_tables, compute_trxbd_coefficients)
import fullblock
import vm as V
import vmrun

PARAM_OFFSETS = (250, 252, 244, 248, 254, 256, 262, 260)
# Original initialized CF setting 0x42432c11, serialized at frame0xa58.
# Stock SHARC 0x1c2888..0x1c2897 converts unsigned16 *0.1 to Hz.
DEFAULT_TUNING_TENTHS_HZ = 4400


def slot_frame_offset(slot, voice=0):
    """OS 1.12 serialization omits portamento slots 93 and 94."""
    if not 0 <= voice < 16:
        raise ValueError('Voice must be in 0..15')
    if 25 <= slot <= 92:
        return 218+2*(slot-25)+146*voice
    if 95 <= slot <= 99:
        return 354+2*(slot-95)+146*voice
    raise ValueError(f'Slot {slot} is not serialized in a voice frame')


def f32(value):
    return struct.unpack('<f', struct.pack('<f', value))[0]


def require_nonzero(peak, label):
    if not math.isfinite(peak) or peak <= 0:
        raise AssertionError(f'{label} is silent/non-finite; equality cannot qualify audio')


class ReferenceVoice:
    """Python DSP56303 semantics + independent rational/f32 resampling."""
    def __init__(self, kernel, sine, tables):
        self.voice = TRXBDDSP56303Voice(kernel, sine)
        self.tables = tables
        self.parameters = None
        self.active = False
        self.block = [0]*32
        self.previous = 0
        self.position = 32
        self.fraction = 0

    def render(self, parameters, trigger):
        if parameters != self.parameters or trigger:
            self.voice.load_coefficients(compute_trxbd_coefficients(parameters, *self.tables))
            self.parameters = list(parameters)
        if trigger:
            self.voice.trigger()
            self.block = self.voice.render_block()
            self.previous = self.block[0]
            self.position = self.fraction = 0
            self.active = True
        if not self.active:
            return [0.0]*32
        result = []
        for _ in range(32):
            if self.position >= 32:
                self.previous = self.block[-1]
                self.block = self.voice.render_block()
                self.position -= 32
            s0 = self.block[self.position-1] if self.position else self.previous
            s1 = self.block[self.position]
            a = f32(s0/8388608)
            b = f32(f32(f32(s1-s0)*f32(self.fraction))*f32(1/(8388608*160)))
            result.append(f32(a+b))
            self.fraction += 147
            if self.fraction >= 160:
                self.fraction -= 160
                self.position += 1
        return result


def configure(frame, controls, machines):
    tuning=controls['global_frame']['master_tuning_tenths_hz']
    if tuning != DEFAULT_TUNING_TENTHS_HZ:
        raise ValueError('Fixture must use the verified initialized stock master tuning')
    struct.pack_into('<H',frame,0xa58,tuning)
    for t in range(16):
        machine = machines[t] if t < len(machines) else 4
        struct.pack_into('<H', frame, 148+2*t, machine)
        struct.pack_into('<H', frame, 2+2*t, 48+3*t)
        struct.pack_into('<H', frame, 0xa5a+2*t, t)
        struct.pack_into('<H', frame, 116+2*t, 0x6400)
        if machine < 4:
            for row in controls['machines'][machine]['slots']:
                if row['slot'] not in (93,94):
                    struct.pack_into('<H', frame, slot_frame_offset(row['slot'],t), row['default'] & 0xffff)
        for off, value in ((302,0x7f00),(306,0x4000),(322,0x7f00),
                           (332,0x7f00),(334,0x7f00),(336,0x7f00),(338,0x7f00),
                           (346,0x4000),(348,0x6400),(352,0x0100)):
            struct.pack_into('<H', frame, off+146*t, value)


def create_runner(section7):
    vm = V.VM()
    vm.load_dn2()
    if section7 is not None:
        vmrun.load_stream(vm, section7)
    vm.call(vmrun.FXINIT)
    return vm, vmrun.Runner(vm, {'PATTERN_VOL': 127})


def save_audio(path, channels):
    interleaved = [sample for pair in zip(*channels) for sample in pair]
    raw = struct.pack(f'<{len(interleaved)}f', *interleaved)
    path.with_suffix('.f32').write_bytes(raw)
    # The float PCM is the lossless comparison artifact; WAV is for listening.
    pcm24 = b''.join(max(-8388608, min(8388607, round(x*8388608))).to_bytes(3, 'little', signed=True)
                     for x in interleaved)
    with wave.open(str(path.with_suffix('.wav')), 'wb') as handle:
        handle.setnchannels(2)
        handle.setsampwidth(3)
        handle.setframerate(48000)
        handle.writeframes(pcm24)
    return dict(sha256=hashlib.sha256(raw).hexdigest(), frames=len(channels[0]),
                rate=48000, channels=2, peak=max(map(abs, interleaved)),
                dc_offset=sum(interleaved)/len(interleaved),
                clipped_samples=sum(abs(x)>=1 for x in interleaved))


def validate(section7: Path, controls: Path, output: Path, blocks=128, *, fx=False, control_frames=None, trace_every=1):
    if blocks < 128:
        raise ValueError('At least 128 blocks are required for the transition schedule')
    if trace_every < 1:
        raise ValueError('trace_every must be positive')
    output.mkdir(parents=True, exist_ok=True)
    (output/'integration.json').unlink(missing_ok=True)
    trace_path = output/'trace.jsonl'
    trace_path.write_text('')
    stream = section7.read_bytes()
    settings = json.loads(controls.read_text())
    serialized = None
    control_report = None
    if control_frames is not None:
        if fx:
            raise ValueError('Serialized-control fixture currently supplies dry FX controls')
        serialized = control_frames.read_bytes()
        control_report = json.loads(control_frames.with_suffix('.json').read_text())
        if (hashlib.sha256(serialized).hexdigest() != control_report['frames_sha256']
                or len(serialized) != blocks*fullblock.FRAME_LEN
                or control_report['controls_sha256'] != hashlib.sha256(controls.read_bytes()).hexdigest()):
            raise ValueError('Serialized control frames are stale, truncated or from different controls')
    stock_vm, stock = create_runner(None)
    native_vm, native = create_runner(stream)
    traces = []
    # First compare untouched stock firmware with the candidate, all four
    # stock synth types audible together, exactly identical control/input.
    for runner in (stock, native):
        configure(runner.d.frame, settings, (0,1,2,3))
        if fx:
            runner.set(CHR_DPTH=64, CHR_SPD=48, CHR_HPF=0, CHR_WDTH=127, CHR_VOL=64,
                       DEL_TIME=1, DEL_X=1, DEL_WID=127, DEL_FDBK=32, DEL_HPF=0, DEL_LPF=127,
                       DEL_REV=32, DEL_VOL=64, REV_PRE=0, REV_DEC=40, REV_FREQ=64, REV_GAIN=64,
                       REV_HPF=0, REV_LPF=127, REV_VOL=64)
            for t in range(4):
                for off, value in ((340,40),(342,48),(344,56)):
                    struct.pack_into('<H', runner.d.frame, off+146*t, value<<8)
    stock_voice_peaks=[0.0]*4
    stock_pcm = ([], [])
    candidate_pcm = ([], [])
    for block in range(32):
        for runner in (stock, native):
            for offset in (34,38,40):
                struct.pack_into('<H', runner.d.frame, offset, 15 if block % 16 == 0 else 0)
        a = stock.block([0.0]*32, [0.0]*32, capture=False)
        b = native.block([0.0]*32, [0.0]*32, capture=False)
        if a != b:
            raise AssertionError(f'Stock-only output changed at block {block}')
        for t in range(4):
            a_voice=stock_vm.rdf(stock_vm.rd32(0x254a60+4*t),32)
            b_voice=native_vm.rdf(native_vm.rd32(0x254a60+4*t),32)
            if a_voice!=b_voice or not all(math.isfinite(x) for x in a_voice+b_voice):
                raise AssertionError(f'Stock machine {t} mono output differs')
            stock_voice_peaks[t]=max(stock_voice_peaks[t],max(map(abs,a_voice)))
        for channel in range(2):
            stock_pcm[channel].extend(a[channel])
            candidate_pcm[channel].extend(b[channel])
    for machine,peak in enumerate(stock_voice_peaks):
        require_nonzero(peak,f'Stock machine {machine}')
    stock_report = save_audio(output/'stock-reference', stock_pcm)
    stock_report['per_machine_peaks']=stock_voice_peaks
    save_audio(output/'stock-candidate', candidate_pcm)
    if stock_report['peak'] == 0:
        raise AssertionError('Silent stock fixture cannot demonstrate stock preservation')

    # Turn the second VM into the independent dry-injection reference. Both
    # retain identical stock processing history from the initial phase.
    # The machine lookup is the only additional change to the stock reference.
    stock_vm.wr32(0x25d75c, 5)
    md = ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'
    kernel, sine = from_firmware(md), build_dsp2_sine_table()
    tables = extract_trxbd_cf_tables(load_firmware(md, MD_DEVICE).section_bytes(0))
    reference = ReferenceVoice(kernel, sine, tables)
    reference_pcm, native_pcm = ([], []), ([], [])
    stages = []
    fx_peaks = dict(chorus=0.0, delay=0.0, reverb=0.0)
    custom_pre_peak = 0.0
    last_machine = 1
    for block in range(blocks):
        phase = block % 128
        # Includes replacement with no note-on, reselect without triggering,
        # stock retriggers after recovery, another no-custom frame/pattern,
        # and repeated replacements. Other stock voices continue throughout.
        custom = 16 <= phase < 64 or 96 <= phase < 112
        machine = 5 if custom else 1
        trigger = phase in (0,16,32,48,65,80,97,113) or (phase > 113 and phase % 4 == 0)
        if machine != last_machine:
            stages.append(dict(block=block, machine=machine, trigger=trigger))
            if machine != 5:
                reference = ReferenceVoice(kernel, sine, tables)
        for runner in (stock, native):
            if serialized is not None:
                runner.d.frame[:] = serialized[block*fullblock.FRAME_LEN:(block+1)*fullblock.FRAME_LEN]
                if struct.unpack_from('<H',runner.d.frame,148)[0] != machine:
                    raise ValueError(f'Serialized schedule differs at block {block}')
            else:
                struct.pack_into('<H', runner.d.frame, 148, machine)
                for offset in (34,38,40):
                    struct.pack_into('<H', runner.d.frame, offset, (1 if trigger else 0) | (2 if phase % 23 == 0 else 0))
        parameters = [x<<7 for x in (40,90,80,50,70,85,60,75)]
        if custom:
            # Fractional control changes while sounding.
            parameters[2] = (parameters[2]+block*37) % 0x3f81
            for runner in (stock, native):
                for offset, parameter in zip(PARAM_OFFSETS, parameters):
                    if serialized is None:
                        struct.pack_into('<H', runner.d.frame, offset, parameter<<1)
                    elif struct.unpack_from('<H',runner.d.frame,offset)[0] != parameter<<1:
                        raise ValueError(f'Serialized parameter differs at block {block}, offset {offset}')
        dry = reference.render(parameters, trigger) if custom else None
        def inject():
            if dry is not None:
                stock_vm.wrf(stock_vm.rd32(0x254a60), dry)
        spi = stock_vm.rd32(fullblock.SPI_HALF)
        stock_vm.write(fullblock.RX_BASE+0x1000*spi, bytes(stock.d.frame))
        stock_vm.write(fullblock.CODEC_IN+0x100*stock_vm.rd32(fullblock.DMA_HALF), bytes(256))
        stock._call_probed([(VOICE_RESUME_SW, inject)])
        # Watch every custom callback, including leaving/reselecting a slot.
        def capture_pre():
            nonlocal custom_pre_peak
            if dry is not None:
                actual = native_vm.read(native_vm.rd32(0x254a60),128)
                if actual != struct.pack('<32f',*dry):
                    raise AssertionError(f'Compiled pre-filter PCM differs at block {block}')
                custom_pre_peak = max(custom_pre_peak,max(map(abs,dry)))
        # Long runs may sample the expensive store/register trace explicitly.
        # PCM/final output/FX comparisons still execute on every block.
        if block < 128 or block % trace_every == 0 or machine != last_machine:
            trace = trace_hook(native_vm, native, [0] if custom else [], capture=capture_pre)
            trace['block'] = block
            traces.append(trace)
            with trace_path.open('a') as handle:
                handle.write(json.dumps(trace)+'\n')
        else:
            native_vm.write(fullblock.RX_BASE+0x1000*native_vm.rd32(fullblock.SPI_HALF),bytes(native.d.frame))
            native_vm.write(fullblock.CODEC_IN+0x100*native_vm.rd32(fullblock.DMA_HALF),bytes(256))
            native._call_probed([(VOICE_RESUME_SW,capture_pre)])
        for channel in range(2):
            a = stock_vm.rdf(fullblock.OUT_FLOATS+128*channel, 32)
            b = native_vm.rdf(fullblock.OUT_FLOATS+128*channel, 32)
            if not all(math.isfinite(x) for x in a+b) or a != b:
                differences = [(i,x,y) for i,(x,y) in enumerate(zip(a,b)) if x != y]
                raise AssertionError(f'Integrated stock chain differs block={block} channel={channel}: {differences[:4]}')
            reference_pcm[channel].extend(a)
            native_pcm[channel].extend(b)
        if fx:
            for name, pointer in (('chorus',0x25b668),('delay',0x25b068),('reverb',0x25b368)):
                a, b = stock_vm.rdf(pointer,64), native_vm.rdf(pointer,64)
                if a != b or not all(math.isfinite(x) for x in a+b):
                    raise AssertionError(f'{name} output differs at block {block}')
                fx_peaks[name] = max(fx_peaks[name],max(map(abs,b)))
        last_machine = machine
    if fx and any(peak == 0 for peak in fx_peaks.values()):
        raise AssertionError(f'FX fixture did not exercise every stock effect: {fx_peaks}')
    reference_report = save_audio(output/'transition-reference', reference_pcm)
    native_report = save_audio(output/'transition-candidate', native_pcm)
    require_nonzero(custom_pre_peak,'TRX-BD pre-filter output')
    require_nonzero(reference_report['peak'],'Independent full-output reference')
    require_nonzero(native_report['peak'],'Compiled full output')
    result = dict(schema='digimd.frame-integration.v1', section7_sha256=hashlib.sha256(stream).hexdigest(),
                  control_fixture_sha256=hashlib.sha256(controls.read_bytes()).hexdigest(),
                  harness_source_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
                  stock_regression=stock_report, transitions=stages,
                  reference=reference_report, candidate=native_report,
                  exact_match=True, compared_samples=blocks*32*2, max_absolute_error=0, rms_error=0,
                  latency_samples=0, memory_traces=traces,
                  memory_trace_every=trace_every, memory_traced_blocks=len(traces),
                  every_block_audio_compared=True,
                  fx_enabled=fx, fx_output_peaks=fx_peaks,
                  custom_pre_filter_peak=custom_pre_peak,
                  serialized_control_provenance=control_report,
                  boundary='Full stock DSP handler; deterministic frames; independent Python dry voice/resampler',
                  pending=['ColdFire frame producer and UI', 'organic sequencer/pattern/project transitions',
                           'IRQs and DMA', 'physical timing and audio'] +
                           ([] if fx else ['enabled FX sends/mixed FX qualification']))
    (output/'integration.json').write_text(json.dumps(result, indent=2)+'\n')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--section7', type=Path, default=ROOT/'out/trxbd_sharc/section7_trxbd.bin')
    parser.add_argument('--controls', type=Path, default=ROOT/'out/audio-qualification/stock-controls.json')
    parser.add_argument('--out', type=Path, default=ROOT/'out/audio-qualification/integration')
    parser.add_argument('--blocks', type=int, default=128)
    parser.add_argument('--trace-every',type=int,default=1,help='Trace stores/registers every N blocks, plus first128 and all machine changes; audio compared every block')
    parser.add_argument('--fx', action='store_true', help='Enable and assert nonzero stock chorus/delay/reverb outputs')
    parser.add_argument('--control-frames',type=Path,help='Frames emitted by serialize_control.py; require matching JSON provenance')
    args = parser.parse_args()
    result = validate(args.section7, args.controls, args.out, args.blocks, fx=args.fx, control_frames=args.control_frames, trace_every=args.trace_every)
    print(json.dumps({k:v for k,v in result.items() if k != 'memory_traces'}, indent=2))


if __name__ == '__main__':
    main()
