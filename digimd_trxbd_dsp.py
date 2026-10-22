"""Provenance-checked extraction of *original* Machinedrum TRX-BD DSP2 P-memory.

This is data extraction, NOT a Motorola DSP interpreter, SHARC translation,
DNII firmware patch, audio renderer, or completeness proof for memory accesses.
The MD OS 1.63 executable is the sole source of all emitted instructions.

P:$145AF5, $145BB6, $145C77 are the firmware's original machine-identity
lookup arrays used for per-voice init, trigger and render respectively.
There are 193 entries in each (machine-code zero is the empty/idle entry).
The literal P-memory segment between TRX-BD's init and TRX-SD's init also
contains TRX-BD's two 288-word lookup blocks.
"""
from __future__ import annotations

from dataclasses import dataclass
import functools
import hashlib
from pathlib import Path
import struct
from typing import Literal, Sequence

from digimd import MD_DEVICE, MD_SHA256, load_firmware
from digimd_dsp56300 import DspImage, parse_dsp_image

KIND = Literal['init', 'trigger', 'render']
MD_ID = 16
NEXT_MD_ID = 17  # TRX-SD; marks the end of this original DSP2 P-memory segment.
TABLE_P = {'init': 0x145AF5, 'trigger': 0x145BB6, 'render': 0x145C77}
TRX_P_BEGIN = 0x1003DA
TABLE_A_BEGIN = 0x10050C
TABLE_B_BEGIN = 0x10062C
TRX_P_END = 0x10074C
EXPECTED_ENTRY_P = {'init': 0x1003DA, 'trigger': 0x1003E1, 'render': 0x100408}
EXPECTED_CODE_WORDS = TABLE_A_BEGIN - TRX_P_BEGIN  # 306
EXPECTED_TABLE_WORDS = 288
EXPECTED_BUNDLE_WORDS = TRX_P_END - TRX_P_BEGIN  # 882


class TRXBDKernelError(ValueError):
    pass


def _expect(ok: bool, why: str) -> None:
    if not ok:
        raise TRXBDKernelError(why)


def pack24(words: tuple[int, ...]) -> bytes:
    """Original DSP56300 24-bit *little-endian word serialization*.

    Not PCM. Do not interpret these words as 24-bit audio samples.
    """
    if any(not (0 <= word <= 0xFFFFFF) for word in words):
        raise ValueError('DSP56300 word outside 24-bit range')
    return b''.join(word.to_bytes(3, 'little') for word in words)


@dataclass(frozen=True)
class OriginalTRXBDKernel:
    entries: dict[str, int]
    code: tuple[int, ...]
    table_a: tuple[int, ...]
    table_b: tuple[int, ...]
    md_firmware_sha256: str
    original_dsp2_sha256: str

    def segments(self) -> dict[str, tuple[int, tuple[int, ...]]]:
        return {'code': (TRX_P_BEGIN, self.code),
                'table_a': (TABLE_A_BEGIN, self.table_a),
                'table_b': (TABLE_B_BEGIN, self.table_b)}

    def manifest(self) -> dict:
        def record(label: str, addr: int, words: tuple[int, ...]) -> dict:
            b = pack24(words)
            return {'name': label, 'space': 'P', 'start': f'0x{addr:06x}',
                    'end_exclusive': f'0x{addr+len(words):06x}',
                    'words': len(words), 'bytes': len(b),
                    'sha256': hashlib.sha256(b).hexdigest()}
        return {
            'format': 'digimd.trxbd.original_dsp2_kernel.v1',
            'firmware_sha256': self.md_firmware_sha256,
            'original_voice_dsp2_sha256': self.original_dsp2_sha256,
            'original_md_machine_id': MD_ID,
            'original_md_machine_code': MD_ID + 1,
            'entry_points': {k: f'P:0x{v:06x}' for k,v in self.entries.items()},
            'original_dispatch_tables': {k: f'P:0x{v:06x}' for k,v in TABLE_P.items()},
            'sections': [record(k, *v) for k,v in self.segments().items()],
            'total_p_words': sum(len(w) for _,w in self.segments().values()),
            'required_host_contract': {
                'dsp2_voice_slot_space': 'Y',
                'voice_slots': 16,
                'slot_base': '0x000800',
                'slot_stride_words': 64,
                'slot_0': 'MD trigger+machine_code, on trigger only',
                'slot_1_through_12': 'MD native coefficient words from original ColdFire routine',
                'block_frames': 32,
                'original_sample_rate_hz': 44100,
            },
            'not_proven': [
                'kernel call graph and all X/Y/P runtime dependencies',
                'DSP56303 register/reset/interrupt/hardware state',
                'actual original DSP2 PCM rendering',
                'bit-exact equivalence against independent reference',
                'SHARC+ translation and Digitone II AMP input hookup',
                'firmware boot, real-time DSP headroom, hardware safety',
            ],
            'flashable': False,
        }


def extract_kernel(image: DspImage, firmware_sha256: str,
                   dsp2_sha256: str) -> OriginalTRXBDKernel:
    """Refuse a different ROM or dispatch map; never guess boundaries."""
    _expect(firmware_sha256 == MD_SHA256, 'unsupported Machinedrum firmware; require stock OS 1.63')
    entries: dict[str, int] = {}
    for kind, table in TABLE_P.items():
        entry = image.word('P', table + MD_ID + 1)
        neighbor = image.word('P', table + NEXT_MD_ID + 1)
        _expect(entry == EXPECTED_ENTRY_P[kind], f'wrong TRX-BD {kind} entry: {entry:#x}')
        _expect(TRX_P_END <= neighbor < TRX_P_END + 0x100,
                f'TRX-SD {kind} entry invalid: {neighbor:#x}')
        entries[kind] = entry
    _expect(image.word('P', TABLE_P['init'] + NEXT_MD_ID + 1) == TRX_P_END,
            'next original TRX-SD program does not start where expected')
    # These immediates are the source program's two original table references.
    _expect(image.word('P', 0x100446) == TABLE_A_BEGIN, 'TRX-BD first table reference changed')
    _expect(image.word('P', 0x100448) == TABLE_B_BEGIN, 'TRX-BD second table reference changed')
    _expect(image.word('P', TABLE_A_BEGIN-1) == 0x00000C,
            'TRX-BD executable region does not end with expected RTS word')
    code = image.words('P', TRX_P_BEGIN, EXPECTED_CODE_WORDS)
    table_a = image.words('P', TABLE_A_BEGIN, EXPECTED_TABLE_WORDS)
    table_b = image.words('P', TABLE_B_BEGIN, EXPECTED_TABLE_WORDS)
    _expect(len(code) + len(table_a) + len(table_b) == EXPECTED_BUNDLE_WORDS,
            'original TRX-BD region not contiguous')
    return OriginalTRXBDKernel(entries, code, table_a, table_b,
                               firmware_sha256, dsp2_sha256)


def from_firmware(path: Path) -> OriginalTRXBDKernel:
    firmware = load_firmware(path, MD_DEVICE)
    original = firmware.section_bytes(1)
    return extract_kernel(parse_dsp_image(original), firmware.digest,
                          hashlib.sha256(original).hexdigest())


def sx24(word: int) -> int:
    word &= 0xFFFFFF
    return word - 0x1000000 if (word & 0x800000) else word


def u24(word: int) -> int:
    return word & 0xFFFFFF


def wrap56(acc_val: int) -> int:
    acc_val &= 0xFFFFFFFFFFFFFF
    return acc_val - 0x100000000000000 if (acc_val & 0x80000000000000) else acc_val


def acc(word24: int) -> int:
    return sx24(word24) << 24


def acc48(hi24: int, lo24: int) -> int:
    return (sx24(hi24) << 24) + (lo24 & 0xFFFFFF)


def lim(acc_val: int) -> int:
    if acc_val > 0x7FFFFFFFFFFF:
        return 0x7FFFFF
    if acc_val < -0x800000000000:
        return -0x800000
    return sx24(acc_val >> 24)


def lim48(acc_val: int) -> tuple[int, int]:
    if acc_val > 0x7FFFFFFFFFFF:
        return 0x7FFFFF, 0xFFFFFF
    if acc_val < -0x800000000000:
        return -0x800000, 0
    return sx24(acc_val >> 24), acc_val & 0xFFFFFF


def a1(acc_val: int) -> int:
    return sx24(acc_val >> 24)


def a0(acc_val: int) -> int:
    return acc_val & 0xFFFFFF


def mpy(s1: int, s2: int) -> int:
    return wrap56(sx24(s1) * sx24(s2) * 2)


def mpysu(s1: int, s2: int) -> int:
    return wrap56(sx24(s1) * u24(s2) * 2)


def mpyuu(s1: int, s2: int) -> int:
    return wrap56(u24(s1) * u24(s2) * 2)


def dmac(dst: int, prod: int) -> int:
    return wrap56((dst >> 24) + prod)


def add(lhs: int, rhs: int) -> int:
    return wrap56(lhs + rhs)


def sub(lhs: int, rhs: int) -> int:
    return wrap56(lhs - rhs)


def asl(acc_val: int, shift: int = 1) -> int:
    return wrap56(acc_val << shift)


def asr(acc_val: int, shift: int = 1) -> int:
    return acc_val >> shift


@functools.lru_cache(maxsize=1)
def build_dsp2_sine_table() -> tuple[int, ...]:
    """Execute DSP2 boot routine P:$100068..$10008D to build X/Y:$148000..$14FFFF."""
    sine = [0] * 32768
    sine[0] = 0
    sine[1] = 0x000648
    hi = [0, 0x000648]
    lo = [0, 0x7ED46B]
    c_hi = 0x7FFFFF
    c_lo = sx24(0xD88586)
    r1 = 1
    for k in range(2, 32768):
        y1 = hi[r1]
        y0 = sx24(lo[r1])
        r1 ^= 1
        a = mpyuu(c_lo, y0)
        a = dmac(a, mpysu(c_hi, y0))
        a = add(a, mpysu(y1, c_lo))
        a = dmac(a, mpy(c_hi, y1))
        a = asl(a, 1)
        a = sub(a, acc48(hi[r1], lo[r1]))
        hi[r1], lo[r1] = lim48(a)
        sine[k] = lim(a)
    return tuple(sine)


def extract_trxbd_cf_tables(control_section: bytes) -> tuple[tuple[int, ...], tuple[int, ...]]:
    """Extract the two ColdFire lookup tables read by 0x20122A..0x2013A5."""
    t_pitch = struct.unpack('>128I', control_section[0x4BD94:0x4BF94])
    t_decay = struct.unpack('>438I', control_section[0x4BF94:0x4BF94 + 438 * 4])
    return t_pitch, t_decay


def compute_trxbd_coefficients(
    params_14bit: Sequence[int],
    t_pitch: Sequence[int],
    t_decay: Sequence[int],
    *,
    trigger: bool = False,
) -> tuple[int, ...]:
    """Exact translation of Machinedrum OS 1.63 ColdFire routine 0x20122A..0x2013A5."""
    if len(params_14bit) < 8:
        raise ValueError('TRX-BD requires 8 parameters')
    p = [int(v) & 0xFFFF for v in params_14bit[:8]]
    out = [0] * 14
    out[0] = 1 if trigger else 0
    d0 = (p[0] >> 6) + 0x40
    d1 = d0 >> 7
    d0 &= 0x7F
    out[3] = ((t_pitch[d0] << d1) & 0xFFFFFFFF) >> 1
    out[1] = t_decay[((p[1] * 0x50) >> 12) + 120]
    d0 = (p[2] * p[2]) >> 18
    out[4] = ((d0 * p[2]) >> 14) + 3
    out[5] = t_decay[(p[3] * 0x46) >> 12]
    out[2] = (p[4] << 9) & 0x01FFFE00
    if p[5] <= 0x1FFF:
        out[11] = (p[5] << 8) & 0xFFFFFFFF
        out[12] = t_decay[0]
    else:
        out[11] = 0x00200000
        out[12] = t_decay[((p[5] * 0x28 - 0x50000) & 0xFFFFFFFF) >> 12]
    out[13] = (p[6] << 9) & 0x01FFFE00
    d2 = p[7]
    d0 = (d2 * 125) & 0xFFFFFFFF
    out[8] = (0x001FFFFF - d0) & 0xFFFFFFFF
    out[9] = (d0 + 0xFFE00000) & 0xFFFFFFFF
    d0 = (d2 * d2) >> 14
    out[10] = (d0 * 511) & 0xFFFFFFFF
    if d2 <= 0x1FFF:
        out[6] = 0x007FFFFF
        out[7] = 0xFF800000
    else:
        d0 = (d2 * 100) & 0xFFFFFFFF
        out[6] = (0x001C7FFF - d0) & 0xFFFFFFFF
        out[7] = (d0 + 0xFFE38000) & 0xFFFFFFFF
    return tuple(out)


class TRXBDDSP56303Voice:
    """Bit-exact execution of Machinedrum OS 1.63 DSP2 TRX-BD (P:$1003DA..$10050B)."""

    def __init__(self, kernel: OriginalTRXBDKernel, sine_table: Sequence[int] | None = None):
        self.kernel = kernel
        self.table_a = tuple(sx24(w) for w in kernel.table_a)
        self.table_b = tuple(sx24(w) for w in kernel.table_b)
        self.sine = tuple(sine_table) if sine_table is not None else build_dsp2_sine_table()
        self.y = [0] * 64
        self.init_voice()

    def init_voice(self) -> None:
        """Execute P:$1003DA..$1003E0."""
        self.y[0x1C] = 0
        self.y[0x1D] = 1

    def load_coefficients(self, words: Sequence[int]) -> None:
        """Write coefficient words 1..13 into Y:(r6+$1..$d)."""
        limit = min(len(words), 14)
        for i in range(1, limit):
            self.y[i] = sx24(words[i])

    def trigger(self) -> None:
        """Execute P:$1003E1..$100407."""
        y = self.y
        b = add(acc(0x50C335), acc(y[0x11]))
        y[0x12] = 0x0A
        y[0x13] = 0x7FFCB9
        y[0x11] = lim(b)
        y[0x16] = 0
        y[0x17] = 0
        y[0x19] = 0
        x0 = y[0x06]
        x1 = y[0x07]
        y[0x1A] = x0
        y[0x1B] = x1
        y[0x14] = x0
        y[0x15] = x1
        y[0x1E] = y[0x0B]
        y[0x21] = y[0x0D]
        y[0x1F] = 0
        y[0x20] = 0
        y[0x0F] = 0x7FFCB9
        y[0x10] = 0x32
        y[0x0E] = 0x7FDBB4

    def render_block(self) -> list[int]:
        """Execute P:$100408..$10050B for one 32-sample DSP2 block."""
        y = self.y
        b_hi = u24(y[0x1C])
        x1_u = u24(y[0x1D])
        noise = [0] * 32
        idx = 0
        for _ in range(10):
            b_hi = (b_hi + x1_u) & 0xFFFFFF
            x0_u = x1_u
            b1_s = sx24(b_hi)
            noise[idx] = b1_s
            noise[idx + 1] = b1_s
            noise[idx + 2] = b1_s
            idx += 3
            x1_u = b_hi
            b_hi = x0_u
        b_hi = (b_hi + x1_u) & 0xFFFFFF
        y[0x1C] = sx24(x1_u)
        b1_s = sx24(b_hi)
        y[0x1D] = b1_s
        noise[30] = b1_s
        noise[31] = b1_s

        x0 = y[0x1E]
        x1 = y[0x0C]
        for i in range(32):
            noise[i] = lim(mpy(noise[i], x0))
        y[0x1E] = lim(mpy(x1, x0))

        a = acc(y[0x12])
        if a != 0:
            a = sub(a, acc(1))
            y[0x12] = lim(a)
            if a == acc(9):
                x0 = 0x733333
                x1 = y[0x18]
                out = [0] * 32
                for i in range(32):
                    s = lim(mpy(x1, x0))
                    out[i] = s
                    x1 = s
                return out
            off = y[0x19]
            y[0x19] = lim(add(acc(0x20), acc(off)))
            x0 = y[0x0F]
            x1 = y[0x02]
            out = [0] * 32
            for i in range(32):
                y1_t = self.table_b[off + i]
                y0_t = self.table_a[off + i]
                a = mpy(y1_t, x1)
                a = asl(a, 1)
                a = asl(a, 1)
                a = add(a, mpy(y0_t, x0))
                a = add(a, acc(noise[i]))
                out[i] = lim(a)
            return out

        # Oscillator 1 (P:$100461..$10047A)
        a = asr(acc(y[0x03]), 6)
        b = asr(mpy(y[0x13], y[0x04]), 1)
        a = add(a, b)
        b = acc48(y[0x16], u24(y[0x17]))
        x_inc = acc48(a1(a), a0(a))
        x_buf = [0] * 32
        sine = self.sine
        for i in range(32):
            r2 = a1(b) & 0x7FFF
            b = (b & ~0xFFFFFF000000) | (r2 << 24)
            b = add(b, x_inc)
            x_buf[i] = lim(asr(acc(sine[r2]), 2))
        y[0x16] = a1(b)
        y[0x17] = sx24(a0(b))

        # Oscillator 2 & ramp decay (P:$10047B..$10049B)
        a = asr(acc(y[0x03]), 7)
        b = asr(mpy(y[0x13], y[0x04]), 1)
        a = add(a, b)
        y[0x13] = lim(mpy(y[0x13], y[0x05]))
        b = acc48(y[0x1F], u24(y[0x20]))
        x_inc = acc48(a1(a), a0(a))
        y_buf = [0] * 32
        for i in range(32):
            r2 = a1(b) & 0x7FFF
            b = (b & ~0xFFFFFF000000) | (r2 << 24)
            b = add(b, x_inc)
            y_buf[i] = lim(asl(acc(sine[r2]), 1))
        y[0x1F] = a1(b)
        y[0x20] = sx24(a0(b))

        # Harmonic mix (P:$10049C..$1004A9)
        x0 = y[0x0E]
        x1 = y[0x21]
        for i in range(32):
            b = acc(x_buf[i])
            a = asr(mpy(y_buf[i], x1), 4)
            x_buf[i] = lim(add(b, a))
        y[0x21] = lim(mpy(x1, x0))

        # Asymmetric wave-folder / shaper (P:$1004AA..$1004D3)
        a = acc(y[0x10])
        x0 = y[0x1A]
        y[0x1A] = lim(add(a, acc(x0)))
        a = acc(y[0x10])
        x1 = y[0x1B]
        y[0x1B] = lim(add(sub(0, a), acc(x1)))
        x6 = x0
        x7 = x1
        y0 = y[0x14]
        y1 = y[0x15]
        a = acc(y[0x10])
        for i in range(32):
            b = asr(sub(0, acc(x_buf[i])), 1)
            if b > acc(x0):
                b = sub(acc(y0), a)
                y1 = x7
                y0 = lim(b)
            elif b < acc(x1):
                b = add(acc(y1), a)
                y0 = x6
                y1 = lim(b)
            x_buf[i] = lim(asl(b, 1))
        y[0x14] = y0
        y[0x15] = y1

        # Amplitude envelope & noise mix (P:$1004D4..$1004DF)
        x0 = y[0x01]
        x1 = y[0x11]
        for i in range(32):
            a = add(mpy(x1, x_buf[i]), mpy(noise[i], x1))
            b = mpy(x1, x0)
            x_buf[i] = lim(a)
            x1 = lim(b)
        y[0x11] = x1

        # Clipper (P:$1004E0..$1004FC)
        y1 = y[0x0A]
        x0 = y[0x08]
        x1 = y[0x09]
        x3_buf = [0] * 32
        for i in range(32):
            a = acc(x_buf[i])
            if a >= acc(x0):
                y0 = lim(sub(a, acc(x0)))
                a = add(mpy(y0, x0), acc(x0))
            elif a <= acc(x1):
                y0 = lim(sub(a, acc(x1)))
                a = add(mpy(y0, x0), acc(x1))
            y0 = lim(a)
            a = add(a, mpy(y1, y0))
            x3_buf[i] = lim(a)

        # Output stage (P:$1004FD..$10050B)
        out = [0] * 32
        for i in range(32):
            out[i] = lim(asl(add(acc(x3_buf[i]), acc(x_buf[i])), 1))
        y[0x18] = out[31]
        return out


def render_trxbd_pcm24(
    md_path: Path,
    syn: Sequence[int] = (64, 64, 0, 0, 0, 0, 0, 0),
    blocks: int = 256,
    *,
    retrigger_blocks: Sequence[int] = (),
    param_changes: dict[int, Sequence[int]] | None = None,
) -> list[int]:
    """Render signed 24-bit PCM from original MD 1.63 TRX-BD tables and DSP56303 logic."""
    fw = load_firmware(md_path, MD_DEVICE)
    kernel = extract_kernel(
        parse_dsp_image(fw.section_bytes(1)),
        fw.digest,
        hashlib.sha256(fw.section_bytes(1)).hexdigest(),
    )
    t_pitch, t_decay = extract_trxbd_cf_tables(fw.section_bytes(0))
    voice = TRXBDDSP56303Voice(kernel)
    cur_syn = [int(v) for v in syn[:8]]
    coeffs = compute_trxbd_coefficients([v << 7 for v in cur_syn], t_pitch, t_decay, trigger=True)
    voice.load_coefficients(coeffs)
    voice.trigger()

    retrig_set = set(retrigger_blocks)
    changes = param_changes or {}
    out: list[int] = []
    for blk in range(blocks):
        if blk > 0:
            if blk in changes:
                cur_syn = [int(v) for v in changes[blk][:8]]
                coeffs = compute_trxbd_coefficients(
                    [v << 7 for v in cur_syn], t_pitch, t_decay, trigger=(blk in retrig_set)
                )
                voice.load_coefficients(coeffs)
            if blk in retrig_set:
                voice.trigger()
        out.extend(voice.render_block())
    return out

