#!/usr/bin/env python3
"""Verify exact compiled SHARC signed24x25 multiplier, register/accumulator ABI.

Run AFTER compare_optimization has freshly built the optimized section7.
This executes the real SHARC instructions in sharcvm, not a host approximation.
It cannot measure physical cycles, DMA/cache or deadline headroom.
"""
from __future__ import annotations

import argparse
import ctypes as C
import hashlib
import json
from pathlib import Path
import random
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.trxbd_sharc.build_sharc import CODE_RANGE
import vm as V
from tools.md_runtime.build import assert_provenance
import vmrun

MATH = ROOT / 'tools/trxbd_sharc/math_leaf.s'


def ref(a: int, b: int) -> int:
    return ((a * b) >> 23) & 0xffffffff


def mr_state(vm):
    """Read both MRF and MRB on both SHARC processing elements, without mutation."""
    vm.lib.vm_get_mr.argtypes = [C.c_void_p, C.c_int, C.c_int,
                                C.POINTER(C.c_uint64), C.POINTER(C.c_uint64)]
    vm.lib.vm_get_mr.restype = None
    state = []
    for pe in (0, 1):
        for which in (1, 2):
            low = C.c_uint64()
            high = C.c_uint64()
            vm.lib.vm_get_mr(vm.h, pe, which, C.byref(low), C.byref(high))
            state.append((low.value, high.value))
    return tuple(state)


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--section7', type=Path, default=ROOT /
                   'out/benchmarks/trxbd-dual-mpy-vs-sixth/optimized/section7_trxbd.bin')
    p.add_argument('--build-report', type=Path, default=ROOT /
                   'out/benchmarks/trxbd-dual-mpy-vs-sixth/optimized/build_report.json')
    p.add_argument('--random', type=int, default=2048)
    p.add_argument('--seed', type=int, default=56303215)
    a = p.parse_args()
    if not 0 <= a.random <= 10000:
        p.error('--random must be in 0..10000')
    section = a.section7.read_bytes()
    meta = json.loads(a.build_report.read_text())
    sha = hashlib.sha256(section).hexdigest()
    if sha != meta['spliced_section7_sha256']:
        raise AssertionError('Section7 differs from exact compiled build report')
    assert_provenance(meta, section)
    if meta.get('diagnostic_only'):
        raise AssertionError('Unexpected diagnostic source section')
    actual_math_hash = hashlib.sha256(MATH.read_bytes()).hexdigest()
    if meta['source_sha256']['tools/trxbd_sharc/math_leaf.s'] != actual_math_hash:
        raise AssertionError('Native math helper changed after section build')
    symbol = meta['symbols'].get('mpy_ss_hi')
    if not symbol:
        raise AssertionError('Compiled SHARC mpy_ss_hi symbol missing')
    entry = int(symbol, 16)
    if not CODE_RANGE[0]//2 <= entry < CODE_RANGE[1]//2:
        raise AssertionError('Compiled SHARC math symbol outside expected code range')
    vm = V.VM()
    vm.load_dn2()
    vmrun.load_stream(vm, section)
    rng = random.Random(a.seed)
    edge_a = (-0x800000, -0x7fffff, -0x400000, -1, 0,
               1, 0x400000, 0x7ffffe, 0x7fffff)
    edge_b = (-0x1000000, -0xffffff, -0x800000, -1, 0,
               1, 0x7fffff, 0xfffffe, 0xffffff)
    pairs = [(x,y) for x in edge_a for y in edge_b]
    pairs += [(rng.randrange(-0x800000,0x800000),
               rng.randrange(-0x1000000,0x1000000))
              for _ in range(a.random)]
    # Snapshot all accumulator halves and MODE1 before each leaf call.
    # A direct-register SSF + SSI operation must not touch MRF/MRB.
    for idx, (x, y) in enumerate(pairs):
        for r in range(9, 16):
            vm.set_reg(f'R{r}', 0x12340000 + r)
        saved_r8 = vm.reg('R8')
        saved_mode = vm.reg('MODE1')
        before_acc = mr_state(vm)
        got = vm.call(entry, args=(x & 0xffffffff, y & 0xffffffff))
        if got != ref(x, y):
            raise AssertionError(f'Exact native multiply mismatch at {idx}: '
                                 f'{x} * {y}, {got:#x} != {ref(x,y):#x}')
        if vm.reg('R8') != (y & 0xffffffff):
            raise AssertionError(f'R8 argument register clobbered at trial {idx}')
        if vm.reg('MODE1') != saved_mode:
            raise AssertionError(f'MODE1 changed at trial {idx}')
        if mr_state(vm) != before_acc:
            raise AssertionError(f'Multiplier accumulator MRF/MRB modified at trial {idx}')
        for r in range(9, 16):
            if vm.reg(f'R{r}') != 0x12340000 + r:
                raise AssertionError(f'Callee-saved R{r} clobbered at trial {idx}')
    print(f'PASS: {len(pairs)} compiled SHARC signed24 x signed25 products')
    print('PASS: all accumulator halves, MODE1 and callee-saved registers preserved')
    print(f'Section7 SHA256: {sha}')
    print('No hardware cycles/IRQ/DMA/cache measurements.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
