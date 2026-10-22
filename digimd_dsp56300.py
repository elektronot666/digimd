"""Read *original* Machinedrum DSP56300 load records from OS 1.63.

The firmware's section 1 is a stream of 24-bit little-endian DSP memory
records; it is not a flat 250,123-instruction binary. This module extracts
P/X/Y memory images without altering the original instructions or inventing
an emulation. DSP bootstrap populates additional runtime tables, so absence
of a word from this loader is distinct from a zero-valued word.

Format independently cross-checked against:
  sd88me/mpc-vst-machinedrum/tools/mdfw/Firmware.cpp::parseDspRecords
  (C++ source licensed AGPL-3.0; this Python parser is separately written.)
"""
from __future__ import annotations
from dataclasses import dataclass
from typing import Literal


class DspFormatError(ValueError):
    pass


Space = Literal['P', 'X', 'Y']
SPACES = ('P', 'X', 'Y')


def u24(data: bytes, at: int) -> int:
    if at < 0 or at + 3 > len(data):
        raise DspFormatError('truncated 24-bit DSP word')
    return int.from_bytes(data[at:at + 3], 'little')


@dataclass(frozen=True)
class Record:
    space: Space
    address: int
    words: tuple[int, ...]

    @property
    def end(self) -> int:
        return self.address + len(self.words)


@dataclass(frozen=True)
class DspImage:
    records: tuple[Record, ...]
    start_address: int | None
    other_markers: tuple[int, ...]

    def word(self, space: Space, address: int) -> int:
        """Firmware-loaded word; raises when not initialized by the image.

        Later records take precedence if ranges overlap, as with the hardware
        DSP loader. Words created at *runtime* are not assumed present.
        """
        if space not in SPACES or address < 0:
            raise ValueError('unknown DSP space/address')
        for r in reversed(self.records):
            if r.space == space and r.address <= address < r.end:
                return r.words[address - r.address]
        raise KeyError(f'{space}:${address:06x} is runtime/unmapped, not firmware-loaded')

    def words(self, space: Space, address: int, count: int) -> tuple[int, ...]:
        if count < 0 or count > 65536:
            raise ValueError('invalid DSP span length')
        return tuple(self.word(space, address + i) for i in range(count))

    def inspect(self) -> dict:
        summary: dict = {
            'start_address': None if self.start_address is None else f'0x{self.start_address:06x}',
            'records': len(self.records),
            'marker4': [f'0x{m:06x}' for m in self.other_markers],
            'spaces': {},
        }
        for space in SPACES:
            rs = [r for r in self.records if r.space == space]
            summary['spaces'][space] = {
                'records': len(rs),
                'program_words': sum(len(r.words) for r in rs),
                'first_address': f'0x{min(r.address for r in rs):06x}' if rs else None,
                'last_address_exclusive': f'0x{max(r.end for r in rs):06x}' if rs else None,
            }
        return summary


def parse_dsp_image(data: bytes) -> DspImage:
    """Decode one compressed section *after* aPLib decompression.

    [3, address] is a start address marker; [4, value] is an unspecified
    metadata marker. [0/1/2, address, count, ...count words] load P/X/Y.
    All fields are little-endian 24-bit words; this is a word-addressed CPU.
    """
    if len(data) % 3:
        raise DspFormatError('DSP image must contain complete 24-bit words')
    records: list[Record] = []
    markers: list[int] = []
    start: int | None = None
    pos = 0
    n = len(data) // 3
    while pos < n:
        tag = u24(data, pos * 3)
        if tag in (3, 4):
            if pos + 2 > n:
                raise DspFormatError('truncated DSP metadata marker')
            val = u24(data, (pos + 1) * 3)
            if tag == 3:
                start = val
            else:
                markers.append(val)
            pos += 2
            continue
        if tag > 2:
            raise DspFormatError(f'unknown DSP load record tag {tag} at word {pos}')
        if pos + 3 > n:
            raise DspFormatError('truncated DSP record header')
        addr, count = u24(data, (pos + 1) * 3), u24(data, (pos + 2) * 3)
        if pos + 3 + count > n:
            raise DspFormatError(f'DSP record {SPACES[tag]}:${addr:06x} truncated')
        if addr + count > (1 << 24):
            raise DspFormatError('DSP record runs outside 24-bit address space')
        raw = data[3 * (pos + 3):3 * (pos + 3 + count)]
        words = tuple(int.from_bytes(raw[i:i + 3], 'little') for i in range(0, len(raw), 3))
        records.append(Record(SPACES[tag], addr, words))
        pos += 3 + count
    if not records or start is None:
        raise DspFormatError('DSP image missing load records or entry marker')
    return DspImage(tuple(records), start, tuple(markers))
