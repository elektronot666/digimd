#!/usr/bin/env python3
"""Recreate the DigiMD experimental firmware from a supplied Digitone II OS 1.12."""
from __future__ import annotations

import argparse
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib

ROOT = Path(__file__).resolve().parent
MAGIC = b'DMDPATCH1'
MAX_FILE_BYTES = 16 * 1024 * 1024


class BuildError(ValueError):
    """An input does not match the release or the patch is malformed."""


def digest(data: bytes) -> str:
    return hashlib.sha256(data).hexdigest()


def read_limited(path: Path) -> bytes:
    with path.open('rb') as stream:
        data = stream.read(MAX_FILE_BYTES + 1)
    if len(data) > MAX_FILE_BYTES:
        raise BuildError(f'{path.name}: file exceeds the supported size')
    return data


def apply_patch(stock: bytes, packed: bytes, manifest: dict) -> bytes:
    """Apply bounded COPY/INSERT records; reject input and result hash changes."""
    if digest(stock) != manifest['stock_sha256']:
        raise BuildError('Unsupported firmware. Supply the original Digitone II OS 1.12 .syx file.\n'
                         f"Expected SHA-256: {manifest['stock_sha256']}\n"
                         f'Provided SHA-256: {digest(stock)}')
    if digest(packed) != manifest['patch_sha256']:
        raise BuildError('Patch checksum failed. Extract a fresh copy of the release.')
    raw_size = manifest['patch_decoded_bytes']
    output_size = manifest['output_bytes']
    if not (0 < raw_size <= MAX_FILE_BYTES and 0 < output_size <= MAX_FILE_BYTES):
        raise BuildError('Invalid release sizes')
    decoder = zlib.decompressobj()
    try:
        raw = decoder.decompress(packed, raw_size + 1)
    except zlib.error as exc:
        raise BuildError('Cannot decompress the release patch') from exc
    if (len(raw) != raw_size or not decoder.eof or decoder.unused_data
            or decoder.unconsumed_tail or not raw.startswith(MAGIC)):
        raise BuildError('Malformed release patch')
    at = len(MAGIC)
    output = bytearray()
    while at < len(raw):
        opcode = raw[at:at + 1]
        at += 1
        if opcode == b'E':
            if at != len(raw):
                raise BuildError('Unexpected bytes after patch end')
            break
        if opcode == b'C':
            if at + 8 > len(raw):
                raise BuildError('Truncated COPY record')
            offset, size = struct.unpack_from('>II', raw, at)
            at += 8
            if not size or offset + size > len(stock):
                raise BuildError('COPY record is outside the supplied firmware')
            chunk = stock[offset:offset + size]
        elif opcode == b'I':
            if at + 4 > len(raw):
                raise BuildError('Truncated INSERT record')
            size = struct.unpack_from('>I', raw, at)[0]
            at += 4
            if not size or at + size > len(raw):
                raise BuildError('Truncated INSERT data')
            chunk = raw[at:at + size]
            at += size
        else:
            raise BuildError('Unknown patch record')
        if len(output) + len(chunk) > output_size:
            raise BuildError('Patch exceeds the expected firmware size')
        output.extend(chunk)
    else:
        raise BuildError('Missing patch end marker')
    result = bytes(output)
    if len(result) != output_size or digest(result) != manifest['output_sha256']:
        raise BuildError('Reconstructed firmware checksum failed')
    return result


def reconstruct(firmware: Path, destination: Path | None, release: Path = ROOT / 'release') -> dict:
    manifest = json.loads((release / 'manifest.json').read_text(encoding='utf-8'))
    if manifest.get('format') != 'digimd.copy-insert.v1':
        raise BuildError('Unsupported release format')
    result = apply_patch(read_limited(firmware), read_limited(release / 'full49.patch'), manifest)
    if destination is not None:
        if destination.suffix.lower() != '.syx':
            raise BuildError('Output must have the .syx extension')
        destination.parent.mkdir(parents=True, exist_ok=True)
        # Exclusive creation also protects an input path or a previous build.
        with destination.open('xb') as stream:
            stream.write(result)
    return manifest


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('firmware', type=Path, help='original Digitone II OS 1.12 .syx')
    parser.add_argument('--output', type=Path, default=ROOT / 'build/DigiMD-FULL49-EXPERIMENTAL-dn2-1.12.syx')
    parser.add_argument('--verify-only', action='store_true', help='reconstruct and verify in memory without writing')
    args = parser.parse_args(argv)
    try:
        manifest = reconstruct(args.firmware, None if args.verify_only else args.output)
    except (OSError, ValueError, KeyError, TypeError) as exc:
        print(f'Build failed: {exc}', file=sys.stderr)
        return 1
    print('Verified reconstruction.' if args.verify_only else f'Created: {args.output.resolve()}')
    print(f"SHA-256: {manifest['output_sha256']}")
    print('Experimental candidate.')
    print('First installation: startup-menu OS UPGRADE over DIN MIDI. Do not use normal Transfer on stock OS.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
