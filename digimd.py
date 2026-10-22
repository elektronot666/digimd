#!/usr/bin/env python3
"""digimd read-only firmware analysis: original MD 1.63 -> DNII 1.12.

Extracts original Machinedrum 68k/voice DSP bytes and Digitone II control/SHARC
bytes, validates framing and compressed sections, maps SHARC load addresses.
It deliberately DOES NOT generate a firmware, synthesize sound or flash devices.

Transport and compression formats were independently implemented from the
published MIT-licensed elektron-firmware-tool format documentation. SHARC
ADI boot records are described in dn2_firmware_explore's research documents.
"""
from __future__ import annotations

import argparse
from dataclasses import dataclass
import hashlib
import json
from pathlib import Path
import struct

MD_SHA256 = 'a58cd61f2efacfb07add0c643162fb4365c30e73831f2021c17b4aa3b42cabd5'
DNII_SHA256 = '7e5b8a733d4ab1fdf7e5d22757f7d8eddaa3cbf71d0858874a791844c02ceb35'
MD_DEVICE, DNII_DEVICE = 2, 21
MD_ROLES = {0: 'md_control', 1: 'md_voice_dsp2', 2: 'md_mixer_dsp1'}
DN_ROLES = {3: 'dn_main_os', 7: 'dn_sharc_bootstream'}
# Machinedrum's genuine DSP2 protocol: 16 voices, 0x40 Y words per voice.
MD_DSP2_Y_SLOT_BASE = 0x800
MD_DSP2_Y_SLOT_WORDS = 0x40
MD_DSP2_COEFF_WORDS = 12  # words 1..12; word 0 encodes trigger/machine


class FormatError(ValueError):
    """Firmware input is truncated, malformed or fails integrity checks."""


def check(condition: bool, description: str):
    if not condition:
        raise FormatError(description)


def be32(data: bytes, pos: int) -> int:
    return struct.unpack_from('>I', data, pos)[0]


def sha(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


@dataclass(frozen=True)
class Section:
    ident: int
    offset: int
    length: int
    destination: int = 0


@dataclass(frozen=True)
class Firmware:
    device: int
    original: bytes
    container: bytes
    packets: int
    sections: tuple[Section, ...]

    @property
    def digest(self) -> str:
        return sha(self.original)

    @property
    def role(self) -> str:
        return {MD_DEVICE: 'Machinedrum', DNII_DEVICE: 'Digitone II'}[self.device]

    def section_bytes(self, ident: int) -> bytes:
        matches = [s for s in self.sections if s.ident == ident]
        check(len(matches) == 1, f'section {ident} missing or ambiguous')
        s = matches[0]
        return depack(self.container[s.offset:s.offset + s.length])


def sysex_messages(data: bytes):
    check(data.startswith(b'\xf0') and data.endswith(b'\xf7'), 'incomplete SysEx file')
    for piece in data.split(b'\xf7')[:-1]:
        check(piece.startswith(b'\xf0'), 'missing F0 before message')
        body = piece[1:]
        check(len(body) >= 6 and body[:3] == b'\x00\x20\x3c', 'not Elektron SysEx')
        check(all(byte < 128 for byte in body), '8-bit byte inside SysEx')
        yield body


def unpack_7bit(src: bytes) -> bytes:
    decoded = bytearray()
    for off in range(0, len(src), 8):
        group = src[off:off + 8]
        if len(group) <= 1:
            break
        for k, val in enumerate(group[1:]):
            decoded.append(val | (((group[0] >> (6 - k)) & 1) << 7))
    return bytes(decoded)


def unpack_md(src: bytes) -> bytes:
    check(len(src) % 3 == 0, 'partial Machinedrum 2+7+7 word')
    decoded = bytearray()
    for off in range(0, len(src), 3):
        word = ((src[off] & 3) << 14) | (src[off + 1] << 7) | src[off + 2]
        decoded.extend(word.to_bytes(2, 'big'))
    return bytes(decoded)


def decode_container(data: bytes) -> tuple[int, bytes, int]:
    chunks = list(sysex_messages(data))
    check(len(chunks) >= 2, 'no packets')
    devices = {c[3] for c in chunks}
    check(len(devices) == 1, 'mixed device IDs')
    device = devices.pop()
    check(device in (MD_DEVICE, DNII_DEVICE), f'unsupported device {device}')
    packets = [c for c in chunks if c[5] == 0x7e and len(c) > (14 if device == MD_DEVICE else 9)]
    marker = [c for c in chunks if c[5] == 0x7f]
    check(len(packets) > 0, 'empty transport')
    output = bytearray()
    if device == DNII_DEVICE:
        seeds = [c[7] for c in marker if len(c) >= 14 and c[6] == 1]
        check(len(seeds) == 1, 'missing unique DNII start marker')
        seed = seeds[0]
        for i, c in enumerate(packets):
            check(len(c) == 126, f'DNII packet {i}: incorrect size')
            actual = (seed + sum(c[6 + j] ^ ((seed + j) & 255) for j in range(119))) & 127
            check(actual == c[125], f'DNII packet {i}: bad checksum')
            output.extend(unpack_7bit(c[9:125]))
        check(len(output) >= 40, 'empty DNII container')
        size = be32(output, 0)
        check(size <= len(output) - 8, 'DNII declared container truncated')
        container = bytes(output[8:8 + size])
        check(container[:4] == b'ELE3', 'DNII ELE3 signature missing')
    else:
        for i, c in enumerate(packets):
            check(len(c) >= 14 and (len(c) - 14) % 3 == 0,
                  f'Machinedrum packet {i}: incomplete 2+7+7 data')
            decoded = unpack_md(c[14:])
            n = c[8:14]
            expected = (sum(decoded) + n[1] + (n[2] << 4) + n[3] + ((n[4] & 12) << 4)) & 255
            check(c[6] == expected >> 4 and c[7] == (expected & 15),
                  f'Machinedrum packet {i}: bad checksum')
            output.extend(decoded)
        container = bytes(output)
    return device, container, len(packets)


def find_sections(device: int, container: bytes) -> tuple[Section, ...]:
    if device == DNII_DEVICE:
        check(len(container) >= 0x20, 'truncated ELE3 header')
        count = be32(container, 0x1c)
        check(0 < count <= 64 and len(container) >= 0x20 + count * 16,
              'bad ELE3 section directory')
        sections = []
        for i in range(count):
            ident, off, length, dest = struct.unpack_from('>IIII', container, 0x20 + i * 16)
            check(off + length <= len(container), f'ELE3 section {ident} out of bounds')
            sections.append(Section(ident, off, length, dest))
        check(len({s.ident for s in sections}) == len(sections), 'duplicate ELE3 IDs')
        return tuple(sections)
    sections = []
    pos = 0
    while pos + 8 < len(container):
        length, stored_sum = struct.unpack_from('>II', container, pos)
        end = pos + 8 + length
        if length >= 16 and end <= len(container) and (sum(container[pos + 8:end]) & 0xffffffff) == stored_sum:
            sections.append(Section(len(sections), pos, length + 8))
            pos = end
        else:
            pos += 1   # old format embeds small amounts of metadata between sections
    check(len(sections) >= 3, 'Machinedrum sections not found')
    return tuple(sections)


def load_firmware(path: Path, expected_device: int | None = None) -> Firmware:
    raw = path.read_bytes()
    device, container, packets = decode_container(raw)
    if expected_device is not None:
        check(device == expected_device, f'{path.name} is not target device {expected_device}')
    return Firmware(device, raw, container, packets, find_sections(device, container))


class Bits:
    def __init__(self, source: bytes):
        self.source = source
        self.at = 8
        self.tag = 0

    def byte(self):
        if self.at >= len(self.source):
            raise EOFError
        val = self.source[self.at]
        self.at += 1
        return val

    def bit(self):
        self.tag <<= 1
        if not self.tag & 255:
            val = self.byte()
            self.tag = (val << 1) | 1
            return val >> 7
        return (self.tag >> 8) & 1

    def gamma(self):
        val = 1
        while True:
            val = (val << 1) + self.bit()
            if self.bit():
                return val
            check(val < 0x02000000, 'invalid aPLib gamma')


def depack(section: bytes, limit: int = 64 * 1024 * 1024) -> bytes:
    """Read original Elektron aPLib variant, including the 8-byte section header.

    Original firmware sometimes omits a terminal marker. EOF after a complete
    token is accepted. Bounds, backrefs and the compressed checksum are checked.
    """
    check(len(section) >= 8, 'short aPLib section')
    count, checksum = struct.unpack_from('>II', section)
    check(count + 8 <= len(section), 'aPLib packed bytes truncated')
    data = section[:count + 8]
    check((sum(data[8:]) & 0xffffffff) == checksum, 'aPLib packed sum mismatch')
    bits = Bits(data)
    out = bytearray()
    last = 1
    while True:
        try:
            if bits.bit():
                check(len(out) < limit, 'depack size cap')
                out.append(bits.byte())
                continue
            g = bits.gamma()
            if g == 2:
                off = last
            else:
                raw = (g << 8) + bits.byte()
                if raw == 767:
                    break
                off = raw - 767
                last = off
            s = (bits.bit() << 1) | bits.bit()
            span = s if s else bits.gamma() + 2
            span += (off > 3328)
            length = span + 1
            check(0 < off <= len(out), f'invalid aPLib backref {off} at {len(out)}')
            check(len(out) + length <= limit, 'depack size cap')
            for _ in range(length):
                out.append(out[-off])
        except EOFError:
            break
    return bytes(out)


@dataclass(frozen=True)
class BootBlock:
    offset: int
    code: int
    address: int
    count: int
    argument: int

    @property
    def flags(self) -> int:
        return (self.code >> 8) & 255

    @property
    def fill(self) -> bool:
        return bool(self.flags & 1)

    @property
    def payload(self) -> bool:
        return self.count > 0 and not bool(self.flags & 0x09)

    @property
    def final(self) -> bool:
        return bool(self.flags & 0x80)


def sharc_blocks(data: bytes) -> tuple[BootBlock, ...]:
    """Parse ADI load records from DNII SHARC section 7 (little-endian headers)."""
    blocks = []
    off = 0
    while off + 16 <= len(data):
        code, addr, count, arg = struct.unpack_from('<IIII', data, off)
        check(code >> 24 == 0xad, f'bad ADI SHARC record at 0x{off:x}')
        b = BootBlock(off, code, addr, count, arg)
        blocks.append(b)
        off += 16 + (count if b.payload else 0)
        check(off <= len(data), 'SHARC payload exceeds image')
    check(off == len(data), 'SHARC record chain incomplete')
    check(blocks and any(b.final for b in blocks), 'SHARC stream lacks final record')
    return tuple(blocks)


def sharc_read(data: bytes, address: int, length: int) -> bytes:
    """Read load-address bytes, not file offsets, accounting for fill blocks."""
    check(0 <= length <= 8 * 1024 * 1024, 'invalid span length')
    out = bytearray(length)
    seen = bytearray(length)
    for b in sharc_blocks(data):
        first, end = max(address, b.address), min(address + length, b.address + b.count)
        if end <= first:
            continue
        dst = first - address
        if b.payload:
            src = b.offset + 16 + (first - b.address)
            out[dst:dst + (end - first)] = data[src:src + end - first]
        elif b.fill:
            out[dst:dst + end - first] = bytes(end - first)
        else:
            continue
        seen[dst:dst + end - first] = b'\x01' * (end - first)
    check(all(seen), f'unmapped SHARC span address=0x{address:x} length={length}')
    return bytes(out)


def export_sources(md: Firmware, dn: Firmware, output: Path) -> dict:
    """Export *unmodified* original programs and manifest; never emit a patch."""
    check(md.device == MD_DEVICE and dn.device == DNII_DEVICE, 'wrong source device pair')
    output.mkdir(parents=True, exist_ok=True)
    report = {'status': 'extraction only; NOT flashable',
              'inputs': {md.role: md.digest, dn.role: dn.digest},
              'artifacts': {}}
    for firmware, mapping in ((md, MD_ROLES), (dn, DN_ROLES)):
        for ident, label in mapping.items():
            data = firmware.section_bytes(ident)
            target = output / f'{label}.bin'
            target.write_bytes(data)
            report['artifacts'][label] = {'bytes': len(data), 'sha256': sha(data),
                                           'file': target.name}
    voice = md.section_bytes(1)
    check(len(voice) % 3 == 0, 'DSP56300 program not 24-bit word aligned')
    report['artifacts']['md_voice_dsp2']['words24'] = len(voice) // 3
    stream = dn.section_bytes(7)
    blocks = sharc_blocks(stream)
    report['dn_sharc_boot'] = {'block_count': len(blocks),
                                'entry_points': [f'0x{b.address:08x}' for b in blocks if b.final],
                                'payload_bytes': sum(b.count for b in blocks if b.payload),
                                'fill_bytes': sum(b.count for b in blocks if b.fill)}
    (output / 'manifest.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


def main():
    parser = argparse.ArgumentParser(description='Original-code MD/DNII read-only firmware extractor')
    parser.add_argument('--md', type=Path, required=True, help='Machinedrum OS1.63 .syx')
    parser.add_argument('--dn', type=Path, required=True, help='Digitone II OS1.12 .syx')
    parser.add_argument('--out', type=Path, default=Path('out/original'))
    parser.add_argument('--allow-other-builds', action='store_true',
                        help='Allow different SHA256s (never re-use 1.12 patch addresses blindly)')
    args = parser.parse_args()
    md = load_firmware(args.md, MD_DEVICE)
    dn = load_firmware(args.dn, DNII_DEVICE)
    if not args.allow_other_builds:
        check(md.digest == MD_SHA256, 'MD image does not match verified OS1.63; use --allow-other-builds')
        check(dn.digest == DNII_SHA256, 'DNII image does not match verified OS1.12; use --allow-other-builds')
    report = export_sources(md, dn, args.out)
    print(json.dumps(report, indent=2))


if __name__ == '__main__':
    main()
