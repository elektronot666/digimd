"""Fail-closed write tracing around the compiled renderer's real dispatch hook.

This watches executed stores, rather than only comparing final RAM (which can
miss a bad write that is subsequently overwritten). The VM logs at most 65536
stores; saturation is an incomplete trace and fails the check.
"""
from __future__ import annotations

import hashlib
import struct

from tools.trxbd_sharc.build_sharc import (
    SAVE_ADDR, STATE_ADDR, VOICE_HOOK_SW, VOICE_RESUME_SW, stub,
)
import vm as V
import fullblock

VOICE_BYTES = 320
STATE_USED = 8 + 16 * VOICE_BYTES


def assert_writes(log, regions):
    if len(log) >= 65536:
        raise AssertionError('Write trace saturated; memory safety is unproven')
    for pc, address, value in log:
        if not any(lo <= address and address + 4 <= hi for lo, hi in regions):
            raise AssertionError(f'Unexpected renderer store PC={pc:#x} address={address:#x} value={value:#x}')


def trace_hook(vm, runner, active_tracks, *, capture=None, extra_state_regions=(), state_region=None, verify_mr=False):
    """Run the whole stock handler, watching only the inline custom hook."""
    state_start, state_end = state_region or (STATE_ADDR, STATE_ADDR + STATE_USED)
    allowed = [(state_start, state_end),
               (SAVE_ADDR, SAVE_ADDR + stub.SAVE_BYTES), *extra_state_regions]
    entry = {}
    if verify_mr:
        from tools.trxbd_sharc.check_mpy_dual_format_asm import mr_state
    registers = [f'R{i}' for i in range(16) if i != 10]
    registers += [f'{kind}{i}' for kind in ('I', 'M', 'L', 'B') for i in range(16)
                  if not (kind == 'I' and i in (5, 12))]
    registers += ['MODE1', 'MODE2', 'IMASK']
    # I5/R10 are replaced by the displaced stock instructions. I12 is the
    # C return scratch; dataflow in stock 0x1c944c onward does not read it.
    guard = b'\xa5\x5a\x3c\xc3' * 8
    vm.write(state_end, guard)

    def enter():
        if verify_mr: entry['__mr'] = mr_state(vm)
        entry['__icount'] = vm.icount
        entry.update({name: vm.reg(name) for name in registers})
        result['frame_machine_types'] = list(struct.unpack('<16H', vm.read(0x25c48c+148, 32)))
        result['cached_machine_types'] = [vm.rd32(0x25566c+0x234*t) for t in range(16)]
        # Model the hardware Audio Task's 4000-byte allocation, including
        # the stock handler's existing frames above this hook.
        top = initial_top
        allowed.append((top - 4000, top + 4))
        for t in active_tracks:
            pointer = vm.rd32(0x254a60 + 4*t)
            if pointer != 0x804acf90 + 128*t:
                raise AssertionError(f'Unexpected mono voice buffer {t}: {pointer:#x}')
            allowed.append((pointer, pointer + 128))
        V.watch(vm, 0, 0xffffffff)

    result = {}
    def leave():
        log = V.watch_log(vm)
        V.watch(vm, 0, 0)
        assert_writes(log, allowed)
        for name, value in entry.items():
            if name in ('__icount','__mr'):
                continue
            if vm.reg(name) != value:
                raise AssertionError(f'Hook changed live {name}: {value:#x} -> {vm.reg(name):#x}')
        if verify_mr and mr_state(vm) != entry['__mr']:
            raise AssertionError('Hook changed MRF/MRB on either processing element')
        if vm.read(state_end, len(guard)) != guard:
            raise AssertionError('Voice-state end canary changed')
        stack_addresses = [a for _, a, _ in log if initial_top - 4000 <= a <= initial_top]
        result.update(stores=len(log), hook_interpreter_instructions=vm.icount-entry['__icount'],
                      multiplier_banks_preserved=verify_mr,
                      physical_cycles_measured=False,
                      stack_bytes_below_handler_entry=initial_top-min(stack_addresses),
                      trace_sha256=hashlib.sha256(b''.join(struct.pack('<III', *row) for row in log)).hexdigest(),
                      allowed_regions=[[hex(lo), hex(hi)] for lo, hi in allowed])
        if capture is not None:
            capture()

    initial_top = vm.reg('I7')
    spi = vm.rd32(fullblock.SPI_HALF)
    vm.write(fullblock.RX_BASE + 0x1000*spi, bytes(runner.d.frame))
    half = vm.rd32(fullblock.DMA_HALF)
    vm.write(fullblock.CODEC_IN + 0x100*half, bytes(256))
    runner._call_probed([(VOICE_HOOK_SW, enter), (VOICE_RESUME_SW, leave)])
    return result
