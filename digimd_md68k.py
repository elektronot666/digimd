"""Bounded execution of original Machinedrum 1.63 coefficient machine code.

This is an intentionally SMALL 68k/ColdFire instruction subset. It executes
instructions read from the user's original MD image; it does not reimplement
algorithms in Python. Unknown operations FAIL CLOSED. Its first verified
machines are GND-NS and GND-SIN (small, deterministic table-based functions).

Architecture contract from md::engine::MachineRunner in Machinemodule:
    d0 = coefficient_fn(uint32_t *out, const uint16_t *params)
    arguments are 68k ABI longs on stack; output words 1..12 drive DSP2.

This is research-only, NOT an emulator for general Machinedrum firmware, and
not suitable for use on the Digitone II DSP audio thread. The final build
will require an established, independently tested 68k engine or native
statically recompiled equivalents verified against an emulator.
"""
from __future__ import annotations
import struct

MASK32 = (1 << 32) - 1
MAIN_BASE = 0x200000
MAIN_SIZE = 0x100000
SRAM_BASE = 0x1000000
SRAM_SIZE = 0x10000
SCRATCH_PARAMS = 0x100E000
SCRATCH_OUT = SCRATCH_PARAMS + 0x100
STACK_TOP = SRAM_BASE + SRAM_SIZE - 0x10
SENTINEL = 0x100


class EmulationError(RuntimeError):
    pass


def sx(number: int, bits: int) -> int:
    number &= (1 << bits) - 1
    return number - (1 << bits) if number & (1 << (bits - 1)) else number


class MD68K:
    def __init__(self, original_cpu_image: bytes):
        if len(original_cpu_image) > MAIN_SIZE:
            raise EmulationError('Machinedrum control image does not fit mapped RAM')
        self.main = bytearray(MAIN_SIZE)
        self.main[:len(original_cpu_image)] = original_cpu_image
        self.sram = bytearray(SRAM_SIZE)
        sram_at = 0x2622F4 - MAIN_BASE
        self.sram[:min(SRAM_SIZE, max(0, len(original_cpu_image) - sram_at))] = original_cpu_image[sram_at:sram_at+SRAM_SIZE]
        # Firmware's startup initializes tempo/denominator fields in SRAM.
        self.sram[0x1500:0x1510] = bytes([6]*12 + [0, 0, 0x0b, 0xb8])
        self.d = [0]*8
        self.a = [0]*8
        self.pc = 0
        self.flags = {'Z': False, 'N': False, 'V': False, 'C': False, 'X': False}
        self.instructions = 0

    def ptr(self, address, length):
        if length < 0 or address < 0:
            raise EmulationError('invalid memory request')
        if MAIN_BASE <= address <= MAIN_BASE + MAIN_SIZE - length:
            return self.main, address - MAIN_BASE
        if SRAM_BASE <= address <= SRAM_BASE + SRAM_SIZE - length:
            return self.sram, address - SRAM_BASE
        raise EmulationError(f'unmapped memory {address:#010x} + {length} @PC={self.pc:#010x}')

    def read(self, address, size):
        b, off = self.ptr(address, size)
        return int.from_bytes(b[off:off+size], 'big')

    def write(self, address, value, size):
        b, off = self.ptr(address, size)
        b[off:off+size] = (value & ((1 << (size*8))-1)).to_bytes(size, 'big')

    def fetch16(self):
        v = self.read(self.pc, 2)
        self.pc += 2
        return v

    def fetch32(self):
        return (self.fetch16() << 16) | self.fetch16()

    def address(self, mode, reg, size):
        """Return the effective address for a memory operand; update A regs."""
        if mode == 2:
            return self.a[reg]
        if mode == 3:
            p = self.a[reg]
            self.a[reg] = (p + (2 if size == 1 and reg == 7 else size)) & MASK32
            return p
        if mode == 4:
            self.a[reg] = (self.a[reg] - (2 if size == 1 and reg == 7 else size)) & MASK32
            return self.a[reg]
        if mode == 5:
            return (self.a[reg] + sx(self.fetch16(), 16)) & MASK32
        if mode == 6:
            ext = self.fetch16()
            if ext & 0x0100:
                raise EmulationError('full-extension indexed addressing not supported')
            idx = (self.a if ext & 0x8000 else self.d)[(ext >> 12) & 7]
            if not ext & 0x0800:
                idx = sx(idx, 16)
            idx *= 1 << ((ext >> 9) & 3)
            return (self.a[reg] + sx(ext, 8) + idx) & MASK32
        if mode == 7 and reg == 0:
            return sx(self.fetch16(), 16) & MASK32
        if mode == 7 and reg == 1:
            return self.fetch32()
        raise EmulationError(f'unsupported address mode {mode}/{reg} @ {self.pc:#x}')

    def operand(self, mode, reg, size):
        if mode == 0:
            return self.d[reg] & ((1 << (size*8))-1)
        if mode == 1:
            return self.a[reg] & ((1 << (size*8))-1)
        if mode == 7 and reg == 4:
            return (self.fetch32() if size == 4 else self.fetch16()) & ((1 << (size*8))-1)
        return self.read(self.address(mode, reg, size), size)

    def write_ea(self, mode, reg, size, value):
        mask = (1 << (size*8))-1
        if mode == 0:
            self.d[reg] = (self.d[reg] & ~mask) | (value & mask)
            return
        if mode == 1:
            if size == 2:
                self.a[reg] = sx(value, 16) & MASK32
            else:
                self.a[reg] = value & MASK32
            return
        self.write(self.address(mode, reg, size), value, size)

    def nz(self, value, size=4):
        mask = (1 << (size*8)) - 1
        v = value & mask
        self.flags['Z'] = v == 0
        self.flags['N'] = bool(v & (1 << (size*8-1)))
        self.flags['V'] = self.flags['C'] = False

    def arithmetic_flags(self, before, rhs, result, size, subtract, write_x=True):
        bits=size*8
        mask=(1<<bits)-1
        sign=1<<(bits-1)
        a,b,r=before&mask,rhs&mask,result&mask
        self.flags['Z']=(r==0)
        self.flags['N']=bool(r&sign)
        if subtract:
            self.flags['C'] = a < b
            self.flags['V'] = bool(((a^b)&(a^r))&sign)
        else:
            self.flags['C'] = a+b>mask
            self.flags['V'] = bool((~(a^b)&(a^r))&sign)
        if write_x:
            self.flags['X']=self.flags['C']

    def step(self):
        at = self.pc
        op = self.fetch16()
        self.instructions += 1
        if op == 0x4E75:  # RTS
            self.pc = self.read(self.a[7], 4)
            self.a[7] += 4
            return
        if (op & 0xF000) == 0x7000:  # MOVEQ
            reg = (op >> 9) & 7
            self.d[reg] = sx(op, 8) & MASK32
            self.nz(self.d[reg])
            return
        if (op & 0xF000) in (0x1000, 0x2000, 0x3000):  # MOVE, MOVEA
            size = {1: 1, 2: 4, 3: 2}[op >> 12]
            value = self.operand((op >> 3) & 7, op & 7, size)
            dm, dr = (op >> 6) & 7, (op >> 9) & 7
            self.write_ea(dm, dr, size, value)
            if dm != 1:
                self.nz(value, size)
            return
        if (op & 0xFFF8) == 0x49C0:  # EXTB.L Dn, sign extend byte
            reg=op&7
            self.d[reg]=sx(self.d[reg],8)&MASK32
            self.nz(self.d[reg])
            return
        if (op & 0xF1C0) == 0x41C0:  # LEA source,An
            dest = (op >> 9) & 7
            self.a[dest] = self.address((op >> 3) & 7, op & 7, 4)
            return
        if (op & 0xFF00) == 0x4200:  # CLR.b/w/l <ea>
            size = {0:1,1:2,2:4}.get((op >> 6) & 3)
            if not size: raise EmulationError(f'bad CLR size at {at:#x}')
            self.write_ea((op >> 3)&7,op&7,size,0)
            self.nz(0, size)
            return
        if (op & 0xFB80) == 0x4880:  # MOVEM.W/L (ColdFire compiler saves temporary registers)
            mask = self.fetch16()
            size = 4 if op & 0x40 else 2
            direction = bool(op & 0x0400)  # 0=regs to RAM, 1=RAM to regs
            mode, reg = (op >> 3)&7, op&7
            if mode not in (2, 3, 4):
                raise EmulationError(f'unsupported MOVEM mode {mode} at {at:#x}')
            indices = [i for i in range(16) if mask & (1 << i)]
            if mode == 4:
                indices = list(reversed(indices))
            for i in indices:
                address = self.address(mode, reg, size)
                if direction:
                    val = self.read(address, size)
                    if size == 2: val = sx(val,16) & MASK32
                    if i < 8: self.d[i] = val
                    else: self.a[i-8] = val
                else:
                    self.write(address, (self.d if i<8 else self.a)[i if i<8 else i-8], size)
            return
        if (op & 0xFFC0) == 0x4C00:  # 68020/ColdFire MUL[SU].L source,Dn
            ext = self.fetch16()
            if ext & 0x0400:
                raise EmulationError('64-bit MUL.L result not supported')
            target = (ext >> 12) & 7
            source = self.operand((op >> 3)&7, op&7, 4)
            if ext & 0x0800:  # signed multiplication
                value = sx(source,32) * sx(self.d[target],32)
            else:
                value = source * self.d[target]
            self.d[target] = value & MASK32
            self.nz(self.d[target])
            return
        if (op & 0xF000) == 0x5000 and ((op >> 6)&3)!=3:  # ADDQ/SUBQ
            size = (1,2,4)[(op>>6)&3]
            count = (op>>9)&7 or 8
            mode, reg = (op>>3)&7, op&7
            val = self.operand(mode,reg,size)
            out = val - count if (op&0x100) else val + count
            self.write_ea(mode,reg,size,out)
            if mode != 1:
                self.arithmetic_flags(val,count,out,size,bool(op&0x100))
            return
        if (op & 0xFF00) in (0x0200, 0x0600, 0x0C00):  # ANDI, ADDI, CMPI
            size_code=(op>>6)&3
            if size_code == 3:
                raise EmulationError('bad immediate arithmetic size')
            size=(1,2,4)[size_code]
            immediate=(self.fetch32() if size==4 else self.fetch16()) & ((1 << (8*size))-1)
            mode,reg=(op>>3)&7,op&7
            before=self.operand(mode,reg,size)
            opcode=op & 0xff00
            if opcode == 0x0200:
                out=before & immediate
            elif opcode == 0x0600:
                out=before + immediate
            else:
                out=before - immediate
            if opcode != 0x0c00:
                self.write_ea(mode,reg,size,out)
            if opcode == 0x0c00:
                # Implement CMP subtraction flags for unsigned higher/lower tests
                bits=size*8; mask=(1<<bits)-1; low=out&mask
                self.flags['Z']=low==0
                self.flags['N']=bool(low&(1<<(bits-1)))
                self.flags['C']=(before & mask)<(immediate & mask)
                self.flags['V']=bool(((before ^ immediate)&(before ^ low)) & (1<<(bits-1)))
            elif opcode == 0x0600:
                self.arithmetic_flags(before, immediate, out, size, False)
            else:
                self.nz(out,size)
            return
        if (op & 0xF000) == 0x6000:  # 68k Bcc relative (including BRA)
            cond=(op>>8)&15
            displacement=sx(op,8)
            if (op&0xff)==0: displacement=sx(self.fetch16(),16)
            elif (op&0xff)==0xff: displacement=sx(self.fetch32(),32)
            z,n,v,c=(self.flags[x] for x in ('Z','N','V','C'))
            take={0:True, 2:not c and not z, 3:c or z, 4:not c, 5:c,
                  6:not z, 7:z, 8:not v, 9:v, 10:not n, 11:n,
                  12:n==v, 13:n!=v, 14:not z and n==v, 15:z or n!=v}.get(cond)
            if take is None: raise EmulationError(f'unsupported branch condition {cond} at {at:#x}')
            if take: self.pc=(at+2+displacement)&MASK32
            return
        if (op & 0xFFC0) == 0x4C40:  # DIV[SU].L: quotient and optional remainder
            ext=self.fetch16()
            quotient_reg=(ext>>12)&7
            remainder_reg=ext&7
            signed=bool(ext&0x0800)
            dividend64=bool(ext&0x0400)
            divisor=self.operand((op>>3)&7,op&7,4)
            if dividend64:
                high=self.d[remainder_reg]
                dividend=(high<<32)|self.d[quotient_reg]
                if signed: dividend=sx(dividend,64)
            else:
                dividend=self.d[quotient_reg]
                if signed: dividend=sx(dividend,32)
            if signed: divisor=sx(divisor,32)
            if divisor==0:
                raise EmulationError('original coefficient code attempted divide by zero')
            result=abs(dividend)//abs(divisor)
            if (dividend<0)!=(divisor<0): result=-result
            if result < (-0x80000000 if signed else 0) or result > (0x7fffffff if signed else MASK32):
                raise EmulationError('division overflow')
            remainder=dividend - result*divisor
            if remainder_reg != quotient_reg:
                self.d[remainder_reg]=remainder & MASK32
            self.d[quotient_reg]=result & MASK32
            self.nz(result)
            return
        if (op & 0xFF00) == 0x4A00 and ((op>>6)&3)!=3:  # TST.b/w/l
            size=(1,2,4)[(op>>6)&3]
            value=self.operand((op>>3)&7,op&7,size)
            self.nz(value,size)
            return
        if (op & 0xF000) == 0xB000 and ((op>>6)&7) in (0,1,2,3,7):  # CMP/CMPA
            mode,reg=(op>>3)&7,op&7
            ctype=(op>>6)&7
            size=(1,2,4)[ctype] if ctype in (0,1,2) else (2 if ctype==3 else 4)
            val=self.operand(mode,reg,size)
            if ctype in (3,7):
                before=self.a[(op>>9)&7]
                val=sx(val,size*8)&MASK32
                size=4
            else:
                before=self.d[(op>>9)&7]
            mask=(1 << (size*8))-1
            out=(before-val)&mask
            sign=1<<(size*8-1)
            self.flags['Z']=out==0
            self.flags['N']=bool(out&sign)
            self.flags['C']=(before&mask)<(val&mask)
            self.flags['V']=bool(((before^val)&(before^out))&sign)
            return
        if (op & 0xF0C0) == 0x50C0:  # Scc: set a byte to $FF or $00
            condition=(op>>8)&15
            z,n,v,c=(self.flags[x] for x in ('Z','N','V','C'))
            truth={0:True,1:False,2:not c and not z,3:c or z,4:not c,5:c,
                   6:not z,7:z,8:not v,9:v,10:not n,11:n,
                   12:n==v,13:n!=v,14:not z and n==v,15:z or n!=v}[condition]
            self.write_ea((op>>3)&7,op&7,1,0xff if truth else 0)
            return
        if (op & 0xF1F8) == 0x9180:  # SUBX.L Dn,Dn with X borrow
            src=op&7
            dest=(op>>9)&7
            rhs=self.d[src]
            before=self.d[dest]
            x=int(self.flags['X'])
            out=(before-rhs-x)&MASK32
            original_z=self.flags['Z']
            self.d[dest]=out
            self.arithmetic_flags(before,rhs+x,out,4,True)
            # Unlike SUB, SUBX only clears the zero flag; it cannot set it.
            self.flags['Z']=original_z and out==0
            return
        if (op & 0xFF00) in (0x4400,0x4600):  # NEG and NOT
            sizecode=(op>>6)&3
            if sizecode==3: raise EmulationError(f'bad NEG/NOT size at {at:#x}')
            size=(1,2,4)[sizecode]
            mode,reg=(op>>3)&7,op&7
            val=self.operand(mode,reg,size)
            out=-val if (op&0xff00)==0x4400 else ~val
            self.write_ea(mode,reg,size,out)
            if (op&0xff00)==0x4400:
                self.arithmetic_flags(0,val,out,size,True)
            else: self.nz(out,size)
            return
        if (op & 0xFFF8) == 0x4840:  # SWAP.W halves of Dn (common fixed-point operation)
            reg=op&7
            val=self.d[reg]
            self.d[reg]=((val&0xffff)<<16)|((val>>16)&0xffff)
            self.nz(self.d[reg])
            return
        if (op & 0xF000) in (0x9000, 0xD000) and ((op>>6)&7) in (0,1,2):  # SUB/ADD <ea>,Dn
            size=(1,2,4)[(op>>6)&7]
            reg=(op>>9)&7
            val=self.operand((op>>3)&7,op&7,size)
            before=self.d[reg]&((1<<(size*8))-1)
            out=before - val if (op&0xf000)==0x9000 else before+val
            self.write_ea(0,reg,size,out)
            self.arithmetic_flags(before,val,out,size,(op&0xf000)==0x9000)
            return
        if (op & 0xF000) == 0xD000 and ((op >> 6)&7) in (3,7):  # ADDA.w/l
            size=2 if ((op>>6)&7)==3 else 4
            source=self.operand((op>>3)&7,op&7,size)
            self.a[(op>>9)&7]=(self.a[(op>>9)&7] + sx(source,8*size))&MASK32
            return
        if (op & 0xF000) == 0xE000 and ((op >> 6) & 3) != 3:  # register shift/rotate
            cnt = (op >> 9) & 7
            sz_code = (op >> 6) & 3
            size = (1, 2, 4)[sz_code]
            if op & 0x20: cnt = self.d[cnt] & 63
            elif cnt == 0: cnt = 8
            action = (op >> 3) & 3
            direction = (op >> 8) & 1
            if action not in (0, 1):
                raise EmulationError(f'unsupported rotate @ {at:#x}')
            mask = (1 << (size * 8)) - 1
            bits = size * 8
            value = self.d[op & 7] & mask
            for _ in range(cnt):
                if direction:
                    value = (value << 1) & mask
                elif action == 0:
                    value = (sx(value, bits) >> 1) & mask
                else:
                    value >>= 1
            self.write_ea(0, op & 7, size, value)
            self.nz(value, size)
            return
        if (op & 0xF1C0) == 0xC0C0:  # MULU.W <ea>, Dn
            reg = (op >> 9) & 7
            value = self.operand((op >> 3)&7, op&7, 2)
            self.d[reg] = ((self.d[reg] & 65535) * value) & MASK32
            self.nz(self.d[reg])
            return
        if (op & 0xF100) == 0xC000 and ((op >> 6) & 7) in (0, 1, 2):  # AND <ea>,Dn
            size = (1,2,4)[(op >> 6)&3]
            reg = (op >> 9) & 7
            val = self.operand((op>>3)&7,op&7,size)
            out = (self.d[reg] & val) & ((1 << (size*8))-1)
            self.write_ea(0, reg, size, out)
            self.nz(out, size)
            return
        raise EmulationError(f'unsupported 68k opcode {op:04x} at 0x{at:06x}')

    def call(self, function, args: tuple[int, ...], budget=30000):
        self.d = [0]*8
        self.a = [0]*8
        self.flags = {'Z': False, 'N': False, 'V': False, 'C': False, 'X': False}
        self.instructions = 0
        self.pc = function
        sp = STACK_TOP
        for arg in reversed(args):
            sp -= 4
            self.write(sp, arg, 4)
        sp -= 4
        self.write(sp, SENTINEL, 4)
        self.a[7] = sp
        while self.pc != SENTINEL:
            if self.instructions >= budget:
                raise EmulationError('instruction budget exceeded')
            self.step()
        return self.d[0]

    def coefficients(self, descriptor: dict, knobs: tuple[int, ...] | list[int], trigger=False):
        if len(knobs) != 8 or any(not isinstance(v, int) or not 0 <= v <= 127 for v in knobs):
            raise ValueError('expected eight SYN values in 0..127')
        for i, value in enumerate(knobs):
            self.write(SCRATCH_PARAMS + 2*i, value << 7, 2)
        for i in range(32):
            self.write(SCRATCH_OUT + 4*i, 0, 4)
        self.write(SCRATCH_OUT, int(trigger), 4)
        count = self.call(int(descriptor['coefficient_function_address'], 16),
                          (SCRATCH_OUT, SCRATCH_PARAMS))
        if count > 32:
            raise EmulationError(f'coefficient function returned bad word count {count}')
        return count, tuple(self.read(SCRATCH_OUT + 4*i, 4) for i in range(32))
