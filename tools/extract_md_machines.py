#!/usr/bin/env python3
"""Decode genuine MD 1.63 machine descriptors from its 68k control program.

Reference: sd88me/mpc-vst-machinedrum/docs/PROTOCOL.md, 135 x 86 bytes at
MD memory address 0x24ef54; executable section 0 maps from 0x200000.
Fields beyond parameter defaults have not yet been completely identified.
Nothing in this file emulates coefficient routines or generates audio.
"""
from __future__ import annotations

import argparse
import csv
import json
from pathlib import Path
import struct
import sys
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
from digimd import MD_DEVICE, FormatError, check, load_firmware, sha

MD_BASE = 0x200000
DESCRIPTOR_ADDRESS = 0x24EF54
DESCRIPTOR_STRIDE = 86
DESCRIPTOR_COUNT = 135


def load_catalog(path: Path) -> dict[int, dict]:
    with path.open(newline='') as f:
        rows = list(csv.DictReader(f))
    check(len(rows) == 50, f'catalog expected 50 entries, got {len(rows)}')
    return {int(row['md_machine_id']): row for row in rows}


def extract_descriptors(cpu: bytes) -> list[dict]:
    offset = DESCRIPTOR_ADDRESS - MD_BASE
    check(offset >= 0 and offset + DESCRIPTOR_STRIDE * DESCRIPTOR_COUNT <= len(cpu),
          'descriptor table outside original MD control binary')
    found = []
    ids = set()
    for i in range(DESCRIPTOR_COUNT):
        rec = cpu[offset + i * DESCRIPTOR_STRIDE:offset + (i + 1) * DESCRIPTOR_STRIDE]
        addr, machine_id = struct.unpack_from('>IB', rec, 0)
        check(machine_id not in ids, f'duplicate MD machine ID {machine_id}')
        ids.add(machine_id)
        check(MD_BASE <= addr < MD_BASE + len(cpu),
              f'machine {machine_id}: coefficient address 0x{addr:x} outside MD control image')
        raw_name = rec[5:10]
        check(all(32 <= z < 127 for z in raw_name), f'machine {machine_id}: invalid name')
        params = []
        for p in range(8):
            raw = rec[10 + 4 * p:14 + 4 * p]
            check(all(c == 0 or 32 <= c < 127 for c in raw),
                  f'machine {machine_id}: param {p} nonprintable label')
            label = raw.rstrip(b'\x00').decode('ascii').strip()
            params.append({'slot': p, 'label': label, 'default_raw': rec[42 + p]})
        found.append({'md_id': machine_id,
                      'md_name': raw_name.decode('ascii'),
                      'coefficient_function_address': f'0x{addr:06x}',
                      'coefficient_function_binary_offset': addr - MD_BASE,
                      'syn': params})
    return found


def machine_report(md, catalog_path: Path) -> dict:
    check(md.device == MD_DEVICE, 'need Machinedrum firmware')
    cpu = md.section_bytes(0)
    all_machines = extract_descriptors(cpu)
    catalogue = load_catalog(catalog_path)
    filtered = []
    for desc in all_machines:
        row = catalogue.get(desc['md_id'])
        if row is None or row['phase1_include'] != 'true':
            continue
        desc = dict(desc)
        desc['family'] = row['family']
        desc['display_name'] = row['md_machine_name']
        filtered.append(desc)
    check(len(filtered) == 49, f'expected 49 synthesis machines; got {len(filtered)}')
    check(len(all_machines) == 135, 'missing MD descriptors')
    check({m['md_id'] for m in filtered} == {mid for mid, row in catalogue.items()
                                             if row['phase1_include'] == 'true'},
          'some synth machine IDs missing in MD firmware')
    return {'source_firmware_sha256': md.digest,
            'source_cpu_sha256': sha(cpu),
            'descriptor_table_address': f'0x{DESCRIPTOR_ADDRESS:06x}',
            'descriptor_count': 135,
            'synthesis_machine_count': len(filtered),
            'note': 'Original metadata and entry pointers only. CPU coefficient routines not executed yet.',
            'machines': filtered}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--md', required=True, type=Path)
    parser.add_argument('--catalog', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'data/machine_catalog.csv')
    parser.add_argument('--out', type=Path, default=Path('out/original/md_synth_descriptors.json'))
    args = parser.parse_args()
    report = machine_report(load_firmware(args.md, MD_DEVICE), args.catalog)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(json.dumps(report, indent=2) + '\n')
    print(f"Decoded {report['descriptor_count']} original MD descriptors, "
          f"{report['synthesis_machine_count']} in scope; wrote {args.out}")
    for machine in report['machines']:
        if machine['md_id'] in (16, 17, 32):
            params = ' '.join(p['label'] for p in machine['syn'])
            print(f"  {machine['display_name']}: {params} @ {machine['coefficient_function_address']}")


if __name__ == '__main__':
    main()
