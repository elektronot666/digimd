"""Isolate the *original* Machinedrum OS 1.63 TRX-BD ColdFire routine.

Pure original-machine coefficient code, NOT the 56303 audio algorithm.
The code and its lookup data are copied verbatim from the owner's OS image.
Only the three absolute ColdFire data-address operands are relocated.
No synthesized or reconstructed lookup values are permitted.

Proves executable relocation in a 68k *model*, not compatibility on DNII
silicon or correctness against an independent 68k emulator. Do not flash.
"""
from __future__ import annotations

import hashlib
import random
from dataclasses import dataclass
from pathlib import Path

from digimd import MD_DEVICE, MD_SHA256, load_firmware
from digimd_md68k import MD68K, EmulationError, SCRATCH_OUT, SCRATCH_PARAMS, STACK_TOP
from tools.extract_md_machines import machine_report

MD_MAIN_BASE = 0x200000
SOURCE_ENTRY = 0x20122A
SOURCE_CODE_END = 0x2013A6
SOURCE_DATA_BEGIN = 0x24BD94
SOURCE_DATA_END = 0x24C800

# Research-only relocated addresses in the Machinedrum bounded-VM map.
# NOT usable DNII placement addresses: those need linker, memory and cache proof.
TARGET_ENTRY = 0x290000
TARGET_DATA_BEGIN = 0x2A0000

# (68k instruction, 32-bit absolute operand at this address, original operand)
# The instruction bytes and literal are checked before any object is built.
RELOCATIONS = (
    (0x20124E, 0x201250, bytes.fromhex('41f9'), 0x24BD94),  # LEA table,a0
    (0x20126C, 0x20126E, bytes.fromhex('47f9'), 0x24BF94),  # LEA table,a3
    (0x2012DE, 0x2012E0, bytes.fromhex('2839'), 0x24BF94),  # MOVE.L table,d4
)


class TransplantError(ValueError):
    pass


@dataclass(frozen=True)
class Transplant:
    code: bytes
    tables: bytes
    entry: int
    tables_address: int
    patches: tuple[dict, ...]

    def manifest(self) -> dict:
        return {
            'format': 'digimd.md-trxbd-native-coldfire-relocation.v1',
            'source_firmware': 'Machinedrum SPS-1 UW OS 1.63',
            'original_entry': hex(SOURCE_ENTRY),
            'relocated_entry_vm_ONLY': hex(self.entry),
            'original_68k_code_bytes': len(self.code),
            'original_68k_code_sha256_after_relocation': hashlib.sha256(self.code).hexdigest(),
            'original_lookup_data_bytes': len(self.tables),
            'original_lookup_data_sha256': hashlib.sha256(self.tables).hexdigest(),
            'relocated_data_start_vm_ONLY': hex(self.tables_address),
            'patches': list(self.patches),
            'verified_audio': False,
            'dn2_machine_patched': False,
            'flashable': False,
        }


def extract(original_main: bytes) -> Transplant:
    if len(original_main) <= SOURCE_DATA_END - MD_MAIN_BASE:
        raise TransplantError('Machinedrum OS control image does not cover TRX-BD tables')
    c0 = SOURCE_ENTRY - MD_MAIN_BASE
    code = bytearray(original_main[c0:SOURCE_CODE_END - MD_MAIN_BASE])
    tables = original_main[SOURCE_DATA_BEGIN - MD_MAIN_BASE:SOURCE_DATA_END - MD_MAIN_BASE]
    if code[-2:] != bytes.fromhex('4e75'):
        raise TransplantError('TRX-BD routine no longer ends in RTS')
    patches = []
    for instruction, operand, opcode, old in RELOCATIONS:
        p = instruction - SOURCE_ENTRY
        o = operand - SOURCE_ENTRY
        if code[p:p+len(opcode)] != opcode:
            raise TransplantError(f'wrong original instruction at {instruction:#x}')
        if int.from_bytes(code[o:o+4], 'big') != old:
            raise TransplantError(f'wrong original data address at {operand:#x}')
        if not SOURCE_DATA_BEGIN <= old < SOURCE_DATA_END:
            raise TransplantError('original lookup pointer outside copied span')
        new = TARGET_DATA_BEGIN + old - SOURCE_DATA_BEGIN
        code[o:o+4] = new.to_bytes(4, 'big')
        patches.append({'instruction': hex(instruction), 'source_operand': hex(old),
                        'vm_operand': hex(new), 'bytes_modified': 4})
    if len(patches) != 3:
        raise TransplantError('missing original lookup table relocation')
    return Transplant(bytes(code), bytes(tables), TARGET_ENTRY,
                      TARGET_DATA_BEGIN, tuple(patches))


class IsolatedTRXBD(MD68K):
    """Run only transplanted firmware code/data, with OS RAM otherwise empty.

    A memory access outside the transplanted code/table and ABI scratch/stack
    fails; we never silently fall back to the original OS's functions/tables.
    """
    def __init__(self, transplant: Transplant):
        super().__init__(b'')
        self.entry = transplant.entry
        self.main[self.entry-MD_MAIN_BASE:self.entry-MD_MAIN_BASE+len(transplant.code)] = transplant.code
        self.main[transplant.tables_address-MD_MAIN_BASE:
                  transplant.tables_address-MD_MAIN_BASE+len(transplant.tables)] = transplant.tables
        self._code = (self.entry, self.entry + len(transplant.code))
        self._data = (transplant.tables_address, transplant.tables_address + len(transplant.tables))
        self._scratch = (SCRATCH_PARAMS, SCRATCH_OUT + 32 * 4)
        self._stack = (STACK_TOP - 0x1000, STACK_TOP + 4)
        self._checking = False
        self._pcs: set[int] = set()
        self._data_addresses: set[int] = set()

    def ptr(self, address, length):
        if self._checking:
            ranges = (self._code, self._data, self._scratch, self._stack)
            if not any(lo <= address and address + length <= hi for lo, hi in ranges):
                raise EmulationError(f'TRX-BD transplant reached undeclared MD OS dependency '
                                     f'{address:#x} (len={length}, PC={self.pc:#x})')
            if self._data[0] <= address < self._data[1]:
                self._data_addresses.add(address)
        return super().ptr(address, length)

    def step(self):
        self._pcs.add(self.pc)
        self._checking = True
        try:
            super().step()
        finally:
            self._checking = False

    def coefficients(self, descriptor: dict, knobs, trigger=False):
        if int(descriptor['coefficient_function_address'], 16) != SOURCE_ENTRY:
            raise TransplantError('descriptor is not the original TRX-BD')
        copy = {**descriptor, 'coefficient_function_address': hex(self.entry)}
        return super().coefficients(copy, knobs, trigger=trigger)

    def coverage(self) -> dict:
        return {'distinct_executed_pcs': len(self._pcs),
                'min_pc': hex(min(self._pcs)) if self._pcs else None,
                'max_pc': hex(max(self._pcs)) if self._pcs else None,
                'distinct_table_word_addresses': len(self._data_addresses),
                'min_data_access': hex(min(self._data_addresses)) if self._data_addresses else None,
                'max_data_access': hex(max(self._data_addresses)) if self._data_addresses else None}


def test_vectors(original_defaults: tuple[int, ...], random_cases: int = 256,
                 seed: int = 0x56303):
    """Deterministic original-parameter stress vectors, with all 128 values of each knob."""
    rng = random.Random(seed)
    for v in (0, 1, 2, 63, 64, 65, 126, 127):
        yield (v,) * 8
    yield original_defaults
    for i in range(8):
        for v in range(128):
            values = list(original_defaults)
            values[i] = v
            yield tuple(values)
    # Adjacent/every-other extremes exercise branches missed by one-knob tests.
    for mask in range(256):
        yield tuple(127 if mask & (1 << k) else 0 for k in range(8))
    for _ in range(random_cases):
        yield tuple(rng.randrange(128) for _ in range(8))


def verify_firmware(md_path: Path, *, random_cases: int = 256) -> dict:
    firmware = load_firmware(md_path, MD_DEVICE)
    if firmware.digest != MD_SHA256:
        raise TransplantError('input is not the verified owner-supplied MD OS 1.63 image')
    original = firmware.section_bytes(0)
    descriptor = next(r for r in machine_report(firmware, Path(__file__).resolve().parent / 'data/machine_catalog.csv')['machines']
                      if r['display_name'] == 'TRX-BD')
    t = extract(original)
    source = MD68K(original)
    candidate = IsolatedTRXBD(t)
    defaults = tuple(int(x['default_raw']) for x in descriptor['syn'])
    cases = 0
    for vec in test_vectors(defaults, random_cases=random_cases):
        for trigger in (False, True):
            n1, w1 = source.coefficients(descriptor, vec, trigger=trigger)
            n2, w2 = candidate.coefficients(descriptor, vec, trigger=trigger)
            if n1 != n2 or w1 != w2:
                raise TransplantError(f'original MD code differs from isolated relocated MD code '
                                      f'at vector {vec}, trigger={trigger}, words {w1[:n1]} != {w2[:n2]}')
            cases += 1
    result = t.manifest()
    result.update({'input_md_sha256': firmware.digest,
                   'original_coefficient_fn': descriptor['coefficient_function_address'],
                   'vectors_tested': cases // 2, 'full_coefficient_comparisons': cases,
                   'result': 'PASS: isolated original 68k code+tables equals stock original 68k model',
                   'isolated_vm_access_guard': True,
                   'coverage': candidate.coverage(),
                   'important_limitation': ('Both branches run within the same bounded 68k VM; independent '
                                            'Musashi/hardware confirmation is still required. Audio is NOT tested.')})
    return result
