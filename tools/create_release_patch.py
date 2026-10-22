#!/usr/bin/env python3
"""Create a portable COPY/INSERT release patch from original and modified .syx files.

This is a packaging tool. It does not compile the synthesis implementation or
certify the modified firmware for hardware. Inputs stay outside the release.
"""
from __future__ import annotations

import argparse
from collections import defaultdict
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from build_firmware import MAGIC, MAX_FILE_BYTES, apply_patch, read_limited


def make_patch(stock: bytes, target: bytes) -> tuple[bytes, int]:
    block = 32
    index = defaultdict(list)
    for at in range(0, len(stock) - block + 1, 16):
        key = stock[at:at + block]
        if len(index[key]) < 8:
            index[key].append(at)
    records = bytearray(MAGIC)
    literal = bytearray()
    at = 0

    def flush():
        if literal:
            records.extend(b'I' + struct.pack('>I', len(literal)) + literal)
            literal.clear()

    while at < len(target):
        best_offset, best_size = 0, 0
        for offset in index.get(target[at:at + block], ()):
            size = block
            limit = min(len(stock) - offset, len(target) - at)
            while size + 256 <= limit and stock[offset + size:offset + size + 256] == target[at + size:at + size + 256]:
                size += 256
            while size < limit and stock[offset + size] == target[at + size]:
                size += 1
            if size > best_size:
                best_offset, best_size = offset, size
        if best_size:
            flush()
            records.extend(b'C' + struct.pack('>II', best_offset, best_size))
            at += best_size
        else:
            literal.append(target[at])
            at += 1
    flush()
    records.extend(b'E')
    if len(records) > MAX_FILE_BYTES:
        raise ValueError('Patch exceeds the decoder limit')
    return zlib.compress(bytes(records), 9), len(records)


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--stock', type=Path, required=True)
    p.add_argument('--target', type=Path, required=True)
    p.add_argument('--out', type=Path, required=True)
    a = p.parse_args()
    if a.out.exists():
        p.error('Release destination already exists; select a new directory')
    stock, target = read_limited(a.stock), read_limited(a.target)
    packed, raw_size = make_patch(stock, target)
    sha = lambda data: hashlib.sha256(data).hexdigest()
    manifest = dict(format='digimd.copy-insert.v1', device='Digitone II', stock_version='1.12',
                    status='EXPERIMENTAL-HARDWARE-VALIDATION-INCOMPLETE',
                    stock_sha256=sha(stock), output_sha256=sha(target), output_bytes=len(target),
                    patch_sha256=sha(packed), patch_decoded_bytes=raw_size)
    if apply_patch(stock, packed, manifest) != target:
        raise ValueError('Release reconstruction differs from target')
    a.out.mkdir(parents=True)
    (a.out / 'full49.patch').write_bytes(packed)
    (a.out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    print(f'Patch: {len(packed):,} bytes; exact reconstruction verified.')


if __name__ == '__main__':
    main()
