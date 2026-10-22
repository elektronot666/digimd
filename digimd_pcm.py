"""Exact-domain PCM verification for the DigiMD original-synthesis port.

The primary reference is *raw Machinedrum DSP2 output*, BEFORE its mixer DSP.
Machinemodule's mdhost/mdvoice dump mono signed-24 samples sign-extended into
little-endian int32 at 44,100 Hz. DNII uses a different rate and float pipeline:
a resampled / post-AMP comparison is never evidence of bit-exact synthesis.

No numpy, resampling, amplitude matching, or automatic time alignment is used
for the exact verdict. A bounded offset search is diagnostic only.
"""
from __future__ import annotations

from array import array
from dataclasses import dataclass
import hashlib
import math
from pathlib import Path
import struct
import sys
import wave


MD_SAMPLE_RATE = 44_100
MD_BLOCK_FRAMES = 32
MD_MIN = -(1 << 23)
MD_MAX = (1 << 23) - 1
FORMATS = ('s24in32le', 's24le', 's16le', 'f32le', 'wav')


class PCMError(ValueError):
    """Malformed PCM or incompatible comparison domains."""


@dataclass(frozen=True)
class PCM:
    values: tuple[int | float, ...]
    sample_bits: tuple[int, ...]
    rate: int
    kind: str              # 'integer' or 'float'
    format: str
    bits: int
    digest: str

    @property
    def frames(self) -> int:
        return len(self.values)

    @property
    def nonzero(self) -> int:
        return sum(v != 0 for v in self.values)

    def normalized(self) -> list[float]:
        scale = float(1 << (self.bits - 1)) if self.kind == 'integer' else 1.0
        return [float(v) / scale for v in self.values]


def _int32_le(data: bytes) -> tuple[int, ...]:
    if len(data) % 4:
        raise PCMError('int32 PCM byte count is not a multiple of 4')
    result = array('i')
    if result.itemsize != 4:
        raise PCMError('host Python int array is not 32 bits')
    result.frombytes(data)
    if sys.byteorder != 'little':
        result.byteswap()
    return tuple(result)


def _int16_le(data: bytes) -> tuple[int, ...]:
    if len(data) % 2:
        raise PCMError('int16 PCM byte count is not a multiple of 2')
    result = array('h')
    if result.itemsize != 2:
        raise PCMError('host Python short array is not 16 bits')
    result.frombytes(data)
    if sys.byteorder != 'little':
        result.byteswap()
    return tuple(result)


def _int24_le(data: bytes) -> tuple[int, ...]:
    if len(data) % 3:
        raise PCMError('int24 PCM byte count is not a multiple of 3')
    result = []
    for i in range(0, len(data), 3):
        n = int.from_bytes(data[i:i + 3], 'little', signed=False)
        result.append(n - (1 << 24) if n & 0x800000 else n)
    return tuple(result)


def _decode(data: bytes, fmt: str) -> tuple[tuple[int | float, ...], tuple[int, ...], str, int]:
    if fmt == 's24in32le':
        values = _int32_le(data)
        if any(not (MD_MIN <= x <= MD_MAX) for x in values):
            raise PCMError('s24in32le contains non-sign-extended/out-of-range 24-bit sample')
        return values, tuple(int(v) for v in values), 'integer', 24
    if fmt == 's24le':
        values = _int24_le(data)
        return values, tuple(int(v) for v in values), 'integer', 24
    if fmt == 's16le':
        values = _int16_le(data)
        return values, tuple(int(v) for v in values), 'integer', 16
    if fmt == 'f32le':
        if len(data) % 4:
            raise PCMError('float32 PCM byte count is not a multiple of 4')
        values = struct.unpack(f'<{len(data) // 4}f', data)
        if not all(math.isfinite(x) for x in values):
            raise PCMError('float32 PCM contains NaN or infinity')
        codes = struct.unpack(f'<{len(data) // 4}I', data)
        return values, codes, 'float', 32
    raise PCMError(f'unsupported PCM format {fmt!r}')


def load_pcm(path: Path | str, fmt: str, rate: int | None = None) -> PCM:
    path = Path(path)
    if fmt not in FORMATS:
        raise PCMError(f'unknown format {fmt!r}; choose from {FORMATS}')
    blob = path.read_bytes()
    digest = hashlib.sha256(blob).hexdigest()
    if not blob:
        raise PCMError(f'{path} is empty')
    if fmt == 'wav':
        try:
            with wave.open(str(path), 'rb') as wav:
                if wav.getnchannels() != 1:
                    raise PCMError('WAV must be mono: tap the synthesis voice, not the stereo mix')
                if wav.getcomptype() != 'NONE':
                    raise PCMError('compressed WAV is not a valid exact PCM reference')
                wav_rate = wav.getframerate()
                nframes = wav.getnframes()
                width = wav.getsampwidth()
                data = wav.readframes(nframes)
        except (wave.Error, EOFError) as exc:
            raise PCMError(f'invalid PCM WAV: {exc}') from exc
        if width not in (2, 3, 4):
            raise PCMError('WAV must use signed 16-, 24- or 32-bit PCM')
        if rate is not None and rate != wav_rate:
            raise PCMError(f'WAV rate {wav_rate} contradicts specified rate {rate}')
        rate = wav_rate
        if width == 4:
            values = _int32_le(data)
            codes = tuple(values)
            kind, bits = 'integer', 32
        else:
            values, codes, kind, bits = _decode(data, {2: 's16le', 3: 's24le'}[width])
    else:
        if rate is None:
            raise PCMError('raw PCM needs an explicit sample rate')
        values, codes, kind, bits = _decode(blob, fmt)
    if rate is None or rate <= 0:
        raise PCMError('invalid PCM sample rate')
    if not values:
        raise PCMError('no audio samples')
    return PCM(values, codes, rate, kind, fmt, bits, digest)


def _error_metrics(a: PCM, b: PCM) -> dict:
    aa, bb = a.normalized(), b.normalized()
    overlap = min(len(aa), len(bb))
    diffs = [x - y for x, y in zip(aa, bb)]
    mean_square = math.fsum(v * v for v in diffs) / max(1, len(diffs))
    ref_power = math.fsum(v * v for v in aa[:overlap]) / max(1, overlap)
    peak_err = max((abs(v) for v in diffs), default=0.0)
    ref_mean = math.fsum(aa[:overlap]) / max(1, overlap)
    got_mean = math.fsum(bb[:overlap]) / max(1, overlap)
    covariance = math.fsum((x-ref_mean)*(y-got_mean) for x, y in zip(aa, bb))
    variance_a = math.fsum((x-ref_mean)**2 for x in aa[:overlap])
    variance_b = math.fsum((x-got_mean)**2 for x in bb[:overlap])
    def statistics(values):
        return {'dc_offset': math.fsum(values)/len(values),
                'peak': max(map(abs, values)),
                'clipped_samples': sum(abs(x) >= 1 for x in values)}
    return {
        'overlap_frames': overlap,
        'rms_error_normalized': math.sqrt(mean_square),
        'peak_error_normalized': peak_err,
        'reference_rms_normalized': math.sqrt(ref_power),
        'correlation': covariance/math.sqrt(variance_a*variance_b)
                       if variance_a > 0 and variance_b > 0 else None,
        'reference_statistics': statistics(aa),
        'candidate_statistics': statistics(bb),
        'alignment_samples': 0,
        'channels': 1,
        'snr_db': None if mean_square == 0 else (10 * math.log10(ref_power / mean_square)
                                                if ref_power > 0 else None),
    }


def _best_shift(a: PCM, b: PCM, maximum: int) -> dict | None:
    """Look for an accidental delay, but never use it to declare bit-exact."""
    if maximum <= 0 or a.rate != b.rate:
        return None
    a_val, b_val = a.normalized(), b.normalized()
    count = min(len(a_val), len(b_val))
    if count < 4 or maximum >= count // 2:
        raise PCMError('diagnostic shift must be less than half the compared frames')
    best = (math.inf, 0)
    for shift in range(-maximum, maximum + 1):
        a_start, b_start = max(-shift, 0), max(shift, 0)
        overlap = min(len(a_val) - a_start, len(b_val) - b_start)
        if overlap <= count // 2:
            continue
        mse = math.fsum((a_val[a_start + k] - b_val[b_start + k]) ** 2
                        for k in range(overlap)) / overlap
        if (mse, abs(shift)) < (best[0], abs(best[1])):
            best = (mse, shift)
    return {'candidate_offset_samples': best[1], 'rms_error_normalized': math.sqrt(best[0]),
            'diagnostic_only': True}


def compare_pcm(ref: PCM, candidate: PCM, *, allow_silence: bool = False,
                diagnostic_max_shift: int = 0) -> dict:
    """Only declare PASS if every unshifted native sample is identical.

    No silent/silent PASS by default. No rate conversion. Integer formats of
    different bit depth, and float-vs-integer comparisons, cannot be bit-exact
    even if their normalized amplitudes happen to match.
    """
    same_rate = ref.rate == candidate.rate
    same_repr = ref.kind == candidate.kind and ref.bits == candidate.bits
    same_frames = ref.frames == candidate.frames
    sample_mismatches = sum(x != y for x, y in zip(ref.sample_bits, candidate.sample_bits)) \
        if same_repr else None
    first_difference = (next((i for i, (x, y) in enumerate(zip(ref.sample_bits, candidate.sample_bits))
                              if x != y), None) if same_repr else None)
    reasons = []
    if not same_rate:
        reasons.append('sample_rate_mismatch: no implicit resampling allowed')
    if not same_repr:
        reasons.append('representation_mismatch: compare in the native synthesis domain')
    if not same_frames:
        reasons.append('length_mismatch')
    if not allow_silence and ref.nonzero == 0:
        reasons.append('silent_reference_cannot_prove_synthesis_accuracy')
    match = (not reasons and sample_mismatches == 0)
    result = {
        'verdict': 'PASS' if match else 'FAIL',
        'bit_exact': bool(match),
        'comparison_domain': 'unshifted/native-rate/native-sample-code',
        'reference': {'rate': ref.rate, 'format': ref.format, 'frames': ref.frames,
                      'nonzero': ref.nonzero, 'sha256': ref.digest},
        'candidate': {'rate': candidate.rate, 'format': candidate.format, 'frames': candidate.frames,
                      'nonzero': candidate.nonzero, 'sha256': candidate.digest},
        'different_sample_codes': sample_mismatches,
        'first_different_sample_index': first_difference,
        'reasons': reasons,
    }
    if same_rate:
        result['diagnostics'] = _error_metrics(ref, candidate)
        shifted = _best_shift(ref, candidate, diagnostic_max_shift)
        if shifted:
            result['diagnostics']['best_shift'] = shifted
    return result
