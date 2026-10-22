"""Fast digikit ISA hooks preserving its ColdFire instruction/exception model.

The scoped upstream hooks pre-scan ~68k opcode-shaped words, including data.
Unicorn traps FF1/BITREV/BYTEREV, so handle those at an illegal-instruction
boundary instead. MOVEC still needs address hooks because Unicorn can crash
before raising a trap. No instruction semantics or boot checks are relaxed.
"""
from __future__ import annotations

import struct


def install():
    from unicorn import UC_HOOK_CODE, UC_HOOK_INTR
    from unicorn.m68k_const import UC_M68K_REG_D0, UC_M68K_REG_PC
    from emu.harness import Machine, _bitrev32, _byterev32

    if getattr(Machine, '_digimd_fast_isa', False):
        return
    original_exceptions = Machine.install_exceptions

    def scoped(self, image, load_addr):
        def handle(uc, vec):
            if vec != 4:
                return False
            pc = uc.reg_read(UC_M68K_REG_PC)
            if not load_addr <= pc < load_addr + len(image) - 1:
                return False
            w = struct.unpack('>H', uc.mem_read(pc, 2))[0]
            reg = UC_M68K_REG_D0 + (w & 7)
            if 0x04C0 <= w <= 0x04C7:
                v = uc.reg_read(reg) & 0xFFFFFFFF
                uc.reg_write(reg, 32 if v == 0 else 32 - v.bit_length())
                self.ff1_count += 1
            elif 0x00C0 <= w <= 0x00C7 or 0x02C0 <= w <= 0x02C7:
                fn = _bitrev32 if w < 0x0200 else _byterev32
                uc.reg_write(reg, fn(uc.reg_read(reg) & 0xFFFFFFFF))
            else:
                return False
            uc.reg_write(UC_M68K_REG_PC, pc + 2)
            return True

        self._digimd_isa_handle = handle
        def isa_only(uc, vec, _data):
            if not handle(uc, vec):
                # Isolated validator/depacker calls have no guest exception
                # dispatcher. Never swallow an unsupported trap there.
                uc.emu_stop()
                raise RuntimeError(f'Unhandled ColdFire vector {vec} at {uc.reg_read(UC_M68K_REG_PC):#x}')
        self._digimd_isa_hook = self.uc.hook_add(UC_HOOK_INTR, isa_only)

        def movec(addr, w):
            def h(uc, _addr, _size, _data):
                ext = struct.unpack('>H', uc.mem_read(addr + 2, 2))[0]
                rc = ext & 0x0FFF
                reg = UC_M68K_REG_D0 + ((ext >> 12) & 7)
                if w == 0x4E7B:
                    self.ctlregs[rc] = uc.reg_read(reg)
                else:
                    uc.reg_write(reg, self.ctlregs.get(rc, 0))
                uc.reg_write(UC_M68K_REG_PC, addr + 4)
                self.movec_count += 1
            return h

        n_movec = 0
        for off in range(0, len(image) - 1, 2):
            w = image[off] << 8 | image[off + 1]
            if w in (0x4E7A, 0x4E7B):
                addr = load_addr + off
                self.uc.hook_add(UC_HOOK_CODE, movec(addr, w), begin=addr, end=addr)
                n_movec += 1
        return 0, n_movec

    def exceptions(self, on_unhandled=None):
        if not hasattr(self, '_digimd_isa_hook'):
            return original_exceptions(self, on_unhandled)
        # Two independent interrupt callbacks would process a handled ISA
        # trap again as a guest exception. Wrap the upstream callback itself
        # so RTE, vector frames, and unhandled-trap behavior stay upstream.
        self.uc.hook_del(self._digimd_isa_hook)
        original_add = self.uc.hook_add

        def add(kind, callback, *args, **kwargs):
            if kind == UC_HOOK_INTR:
                def combined(uc, vec, data):
                    if not self._digimd_isa_handle(uc, vec):
                        callback(uc, vec, data)
                return original_add(kind, combined, *args, **kwargs)
            return original_add(kind, callback, *args, **kwargs)

        self.uc.hook_add = add
        try:
            return original_exceptions(self, on_unhandled)
        finally:
            self.uc.hook_add = original_add

    Machine.install_isa_patches_scoped = scoped
    Machine.install_exceptions = exceptions
    Machine._digimd_fast_isa = True
