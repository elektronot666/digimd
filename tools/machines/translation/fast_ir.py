"""DSP56300 routine -> register IR, mirroring tools.machines.translation.dsp.

The C lowering in dsp.py (plus generate.py's function wrappers) is the
semantic reference that passed original-firmware PCM/state parity. This
module lifts the same instructions, with the same decisions, into a small
three-address IR over 32-bit values. Two backends consume it: a host C
emitter (fast_c.py, checked against the original recordings) and the SHARC+
assembler emitter (fast_sharc.py).

IR instruction tuples (operands are variable names or Python ints):
  ('mov', d, a)                 d = a
  ('add'|'sub'|'and'|'or'|'xor', d, a, b)   32-bit wrap-around
  ('shl'|'shr'|'sar', d, a, n)  n: int 0..31 (logical left/right, arith right)
  ('min'|'max', d, a, b)        signed
  ('lt'|'ltu'|'eq', d, a, b)    0/1 (signed less, unsigned less, equal)
  ('mulhi', d, a, b)            floor(a*b / 2^23)   (a,b signed, |a*b| < 2^54)
  ('mullo', d, a, b)            low 32 bits of a*b
  ('load', d, base, index)      d = base[index]  (int32 words)
  ('store', base, index, s)     base[index] = s
  ('dread', d, space, addr, raw)   d = md_read_<space>(addr) (generic, may fault)
  ('dwrite', space, addr, s)    md_write_<space>(addr, s)
  ('fault', addr)               v->memory_fault = 1
  ('label', name) ('jmp', name) ('jnz', c, name) ('jz', c, name) ('ret',)
  ('global_load', d, symbol) ('global_store', symbol, s)
  ('field_load', d, field) ('field_store', field, s)   voice struct scalars
"""
import re
from tools.machines.translation.dsp import Unsupported, Instruction, reachable, target

ACC = ('a', 'b')
WORD_REGS = ('x0', 'x1', 'y0', 'y1')
FLAGS = ('zero', 'less', 'greater', 'negative', 'carry')


def imm(text):
    return int(text.split('$')[1], 16)


class Builder:
    def __init__(self):
        self.ops = []
        self.temps = 0
        self.labels = 0

    def t(self):
        self.temps += 1
        return f't{self.temps}'

    def label(self):
        self.labels += 1
        return f'X{self.labels}'

    def emit(self, *op):
        self.ops.append(tuple(op))

    def op(self, kind, a, b=None):
        d = self.t()
        if b is None:
            self.emit(kind, d, a)
        else:
            self.emit(kind, d, a, b)
        return d

    # Common C idioms of the reference lowering.
    def sx24(self, a):
        if isinstance(a, int):
            return _sx24(a) & 0xffffffff
        return self.op('sar', self.op('shl', a, 8), 8)

    def u24(self, a):
        if isinstance(a, int):
            return a & 0xffffff
        return self.op('and', a, 0xffffff)

    def lim24(self, a):
        return self.op('max', self.op('min', a, 0x7fffff), -0x800000)


def _sx24(a):
    a &= 0xffffff
    return a - 0x1000000 if a & 0x800000 else a


class Machine:
    """Per-machine memory configuration (generate.py's Lowering flags)."""

    def __init__(self, m):
        family = m.get('family') or m['name'].split('-')[0]
        self.name = m['name']
        self.prefix = m['prefix']
        self.words = m['original_slot_words']
        self.private_memory = bool(m.get('private_memory'))
        self.buffered_output = bool(m.get('buffered_output'))
        self.requires_source_image = bool(m.get('requires_source_image'))
        name = m['name']
        fam = name.split('-')[0] if not name.startswith('P-I') else 'P-I'
        self.scratch_x_words = 256 if fam in ('E12', 'P-I') else 128
        self.scratch_y_words = 352 if self.buffered_output else 256 if fam in ('P-I', 'E12') else 128
        if fam == 'GND':
            self.scratch_x_words = 32
            self.scratch_y_words = 1
        if name == 'TRX-CP':
            self.scratch_x_words = 256
            self.scratch_y_words = 256
        self.tables = {}


class Lifter:
    def __init__(self, instructions, machine, image=None):
        self.instructions = instructions
        self.m = machine
        self.image = image
        self.b = Builder()
        self.loops = {}
        self.direct = {}
        self.used_scratch = set()
        self.raw = False

    # ------------------------------------------------------------ operands
    def memory_address(self, operand):
        """Return (kind, payload) for an x:/y: operand, exactly as dsp.memory().

        kinds: ('dyn', space, addr_var, raw?) generic read/write
               ('arr', base, index_var)      direct array element
               ('src', addr_var)             md_read_source
               ('sine', index_var)
        """
        b = self.b
        m = re.fullmatch(r'([xy]):\(r([0-7])\+n([0-7])\)', operand)
        if m:
            space, reg, n = m.groups()
            addr = self.agu_index(f'r{reg}', f'm{reg}', b.sx24(f'n{n}'))
            return ('dyn', space, addr)
        m = re.fullmatch(r'([xy]):[<>]?\$([0-9a-f]+)', operand)
        if m:
            address = int(m[2], 16)
            limit = self.m.scratch_x_words if m[1] == 'x' else self.m.scratch_y_words
            if address >= limit:
                raise Unsupported('Absolute scratch bound: ' + operand)
            self.used_scratch.add(m[1])
            return ('arr', 'S' + m[1].upper(), address)
        m = re.fullmatch(r'([xy]):\(r([0-7])(?:\+\$([0-9a-f]+))?\)([+-]?)(?:n([0-7]))?', operand)
        pre = False
        stride = None
        if not m:
            m = re.fullmatch(r'([xy]):(-)\(r([0-7])\)', operand)
            if not m:
                raise Unsupported('Memory operand: ' + operand)
            space, reg, offset, update = m[1], m[3], None, '-'
            pre = True
        else:
            space, reg, offset, update, stride = m.groups()
        r = f'r{reg}'
        if offset and int(offset, 16) == 0x148000:
            return ('sine', r)
        if offset:
            base = int(offset, 16)
            if base >= 0x100000:
                return ('src', b.op('add', r, base))
            return ('dyn', space, b.op('add', r, base))
        address = r
        if update:
            if not pre and stride:
                step = b.sx24(f'n{stride}')
                if update == '-':
                    step = b.op('sub', 0, step)
                address = self.agu_update(r, f'm{reg}', step, pre=False)
            elif reg in self.direct:
                # dsp.py: ++r / r++ / --r / r-- without 24-bit masking
                delta = 1 if update == '+' else -1
                if pre:
                    b.emit('add', r, r, delta & 0xffffffff)
                    address = r
                else:
                    old = b.op('mov', r)
                    b.emit('add', r, r, delta & 0xffffffff)
                    address = old
            else:
                address = self.agu_update(r, f'm{reg}', 1 if update == '+' else 0xffffffff, pre=pre)
        if reg in self.direct and self.direct[reg] < 128:
            base = self.direct[reg]
            self.used_scratch.add(space)
            return ('arr', 'S' + space.upper(), b.op('add', b.op('and', address, 31), base))
        return ('dyn', space, address)

    def agu_update(self, r, m, step, pre):
        """md_agu(&r, m, step, pre): updates r, returns old or new value."""
        b = self.b
        old = b.op('mov', r)
        b.emit('agu', r, old, m, step)
        return r if pre else old

    def agu_index(self, r, m, step):
        """md_index(r, m, step) = md_agu on a copy, pre=1 result."""
        b = self.b
        d = b.t()
        b.emit('agu', d, r, m, step)
        return d

    def read(self, operand, raw=False):
        """value of a memory operand as dsp.word() (24-bit, zero-extended)."""
        b = self.b
        raw = self.raw
        kind = self.memory_address(operand)
        if kind[0] == 'dyn':
            d = b.t()
            self.used_scratch.add(kind[1])
            b.emit('dread', d, kind[1], kind[2], raw)
            return d
        if kind[0] == 'src':
            d = b.t()
            b.emit('dread', d, 'src', kind[1], raw)
            return d
        if kind[0] == 'sine':
            return b.u24(b.op('load', 'SINE', kind[1]) if False else self._load('SINE', kind[1]))
        return b.u24(self._load(kind[1], kind[2]))

    def _load(self, base, index):
        d = self.b.t()
        self.b.emit('load', d, base, index)
        return d

    def write(self, operand, value):
        b = self.b
        kind = self.memory_address(operand)
        if kind[0] == 'dyn':
            self.used_scratch.add(kind[1])
            b.emit('dwrite', kind[1], kind[2], value)
            return
        if kind[0] in ('src', 'sine'):
            raise Unsupported('Immutable source write')
        b.emit('store', kind[1], kind[2], b.sx24(value))

    def word(self, operand, raw=False):
        b = self.b
        if self.m.private_memory and operand == 'y:>$142':
            d = b.t()
            b.emit('field_load', d, 'virtual_voice')
            return d
        if self.m.private_memory and operand == 'y:>$140':
            return 0x100
        if re.fullmatch(r'#[<>]?\$[0-9a-f]+', operand):
            return imm(operand)
        if operand in ACC:
            return b.u24(b.lim24(operand + 'h'))
        if operand in ('a0', 'b0'):
            return operand[0] + 'l'
        if operand in ('a1', 'b1'):
            return b.u24(operand[0] + 'h')
        if re.fullmatch(r'[xy][01]', operand):
            # data ALU input registers are held sign-extended (see store)
            return b.u24(operand)
        if re.fullmatch(r'[rmn][0-7]', operand):
            return operand
        m = re.fullmatch(r'y:\(r6(?:\+\$([0-9a-f]+))?\)', operand)
        if m:
            offset = int(m[1] or '0', 16)
            if offset >= 64:
                raise Unsupported('Outside original voice slot')
            return b.u24(self._load('Y', offset))
        if operand.startswith(('x:', 'y:')):
            return self.read(operand, raw)
        raise Unsupported('Operand: ' + operand)

    def store(self, operand, value):
        b = self.b
        if operand in ACC:
            b.emit('mov', operand + 'h', b.sx24(value))
            b.emit('mov', operand + 'l', 0)
            return
        if operand in ('a0', 'b0'):
            b.emit('and', operand[0] + 'l', value, 0xffffff)
            return
        if operand in ('a1', 'b1'):
            h = operand[0] + 'h'
            b.emit('or', h, b.op('and', h, 0xff000000), b.u24(value))
            return
        if re.fullmatch(r'[xy][01]', operand):
            # x0/x1/y0/y1 hold sx24(value): the 24-bit register contents are
            # unchanged (u24(sx24(v)) == v & 0xffffff); every read applies u24.
            b.emit('mov', operand, b.sx24(value))
            return
        if re.fullmatch(r'[rmn][0-7]', operand):
            b.emit('and', operand, value, 0xffffff)
            return
        m = re.fullmatch(r'y:\(r6(?:\+\$([0-9a-f]+))?\)', operand)
        if m:
            offset = int(m[1] or '0', 16)
            if offset >= 64:
                raise Unsupported('Outside original voice slot')
            b.emit('store', 'Y', offset, b.sx24(value))
            return
        if operand.startswith(('x:', 'y:')):
            self.write(operand, value)
            return
        raise Unsupported('Destination: ' + operand)

    def pair(self, operand):
        b = self.b
        if operand in ACC:
            return operand + 'h', operand + 'l'
        if operand in ('x', 'y'):
            return b.sx24(operand + '1'), b.u24(operand + '0')
        return b.sx24(self.word(operand)), 0

    # ------------------------------------------------------------ arithmetic
    def add56(self, ah, al, bh, bl, dh, dl):
        b = self.b
        sl = b.op('add', al, bl)
        sh = b.op('add', b.op('add', ah, bh), b.op('shr', sl, 24))
        b.emit('mov', dh, sh)
        b.emit('and', dl, sl, 0xffffff)

    def sub56(self, ah, al, bh, bl, dh, dl):
        b = self.b
        dlv = b.op('sub', al, bl)
        dhv = b.op('sub', ah, bh)
        borrow = b.op('lt', dlv, 0)
        b.emit('and', dl, b.op('add', dlv, b.op('and', b.op('sub', 0, borrow), 0x1000000)), 0xffffffff)
        b.emit('sub', dh, dhv, borrow)

    def neg56(self, h, l):
        b = self.b
        nz = b.op('ltu', 0, l)  # l != 0
        nh = b.op('sub', b.op('sub', 0, h), nz)
        nl = b.op('and', b.op('sub', 0, l), 0xffffff)
        return nh, nl

    def product(self, fn, a, c):
        """(hi, lo) of mpy_ss32 / mpy_su32 / mpy_uu32 for prepared operands."""
        b = self.b
        if fn == 'mpy_su32':
            a = b.sx24(a)
            c = b.u24(c)
        elif fn == 'mpy_uu32':
            a = b.u24(a)
            c = b.u24(c)
        hi = b.op('mulhi', a, c)
        lo = b.op('and', b.op('shl', b.op('mullo', a, c), 1), 0xffffff)
        return hi, lo

    def flags_acc(self, d):
        b = self.b
        h, l = d + 'h', d + 'l'
        b.emit('lt', 'negative', h, 0)
        b.emit('and', 'zero', b.op('eq', h, 0), b.op('eq', l, 0))
        b.emit('mov', 'less', 'negative')
        b.emit('and', 'greater', b.op('eq', 'zero', 0), b.op('eq', 'negative', 0))

    # ------------------------------------------------------------ instructions
    def instruction(self, i):
        self._instruction(i)
        if len(re.split(r'\s{2,}', i.args)) > 1:
            return
        if i.op in ('add', 'sub', 'mpy', 'mpysu', 'mpyuu', 'mpyr', 'mac', 'macr', 'macsu', 'mpyi', 'maci', 'dmac',
                    'asr', 'asl', 'abs', 'neg', 'clr', 'tst'):
            dest = i.args.split(',')[-1]
            if dest in ACC:
                self.flags_acc(dest)

    def raw_access(self, i):
        opcode = i.words[0]
        return opcode >= 0x100000 or (opcode & 0xfe4000) == 0x080000

    def _instruction(self, i):
        b = self.b
        op, args = i.op, i.args
        raw = self.raw_access(i)
        self.raw = raw
        parallel = re.split(r'\s{2,}', args)
        if len(parallel) > 1:
            if parallel[1].startswith('if'):
                cond = {'ifeq': 'zero', 'ifne': '!zero', 'ifle': '!greater', 'iflt': 'less', 'ifge': '!less',
                        'ifgt': 'greater', 'ifmi': 'negative', 'ifpl': '!negative'}.get(parallel[1])
                if cond is None:
                    raise Unsupported('Conditional ALU: ' + i.text)
                base = Instruction(i.pc, i.size, i.words, op + ' ' + parallel[0])
                skip = b.label()
                self.branch_unless(cond, skip)
                self.instruction(base)
                b.emit('label', skip)
                return
            after = []
            for k, move in enumerate(parallel[1:]):
                src, dest = move.split(',')
                if src.startswith('l:') or dest.startswith('l:'):
                    after.append(self.long_move(src, dest, raw))
                else:
                    value = b.op('mov', self.word(src, raw))
                    after.append(lambda dest=dest, value=value: self.store(dest, value))
            base = Instruction(i.pc, i.size, i.words, op + ' ' + parallel[0])
            self.instruction(base)
            for fn in after:
                fn()
            return
        p = args.split(',')
        if op == 'nop':
            return
        if op == 'rts':
            b.emit('ret')
            return
        if op == 'move' and len(p) == 1:
            agu = re.fullmatch(r'\(r([0-7])\)([+-])(?:n([0-7]))?', args)
            if agu:
                reg, sign, stride = agu.groups()
                step = b.sx24(f'n{stride}') if stride else 1
                if sign == '-':
                    step = b.op('sub', 0, step) if not isinstance(step, int) else (-step) & 0xffffffff
                self.agu_update(f'r{reg}', f'm{reg}', step, pre=False)
                return
        if op == 'move' and len(p) == 2:
            if p[0].startswith('l:') or p[1].startswith('l:'):
                self.long_move(p[0], p[1], raw)()
                return
            value = self.word(p[0], raw)
            if re.fullmatch(r'#\$[0-9a-f]+', p[0]) and re.fullmatch(r'[ab]|[xy][01]', p[1]):
                value = (value << 16) & 0xffffffff
            self.store(p[1], value)
            return
        if op == 'clr' and p[0] in ACC:
            b.emit('mov', p[0] + 'h', 0)
            b.emit('mov', p[0] + 'l', 0)
            return
        if op in ('mpy', 'mpysu', 'mpyuu', 'dmac', 'mpyr', 'mac', 'macr', 'mpyi', 'maci', 'macsu'):
            if op == 'dmac':
                if not args.startswith('ss '):
                    raise Unsupported('DMAC signedness: ' + args)
                p = args[3:].split(',')
            if len(p) != 3 or p[2] not in ACC:
                raise Unsupported('Multiply form: ' + args)
            d = p[2]
            negate = p[0].startswith('-')
            p[0] = p[0].lstrip('-')
            x, y = self.word(p[0], raw), self.word(p[1], raw)
            fn = {'mpy': 'mpy_ss32', 'mpyr': 'mpy_ss32', 'mac': 'mpy_ss32', 'macr': 'mpy_ss32', 'mpyi': 'mpy_ss32',
                  'maci': 'mpy_ss32', 'macsu': 'mpy_su32', 'mpysu': 'mpy_su32', 'mpyuu': 'mpy_uu32',
                  'dmac': 'mpy_ss32'}[op]
            if op != 'mpyuu':
                x = b.sx24(x)
            if op not in ('mpysu', 'mpyuu', 'macsu'):
                y = b.sx24(y)
            th, tl = self.product(fn, x, y)
            if negate:
                th, tl = self.neg56(th, tl)
            dh, dl = d + 'h', d + 'l'
            if op == 'dmac':
                b.emit('and', dl, dh, 0xffffff)
                b.emit('sar', dh, dh, 24)
                self.add56(th, tl, dh, dl, dh, dl)
            elif op in ('mac', 'macr', 'maci', 'macsu'):
                self.add56(dh, dl, th, tl, dh, dl)
            else:
                b.emit('mov', dh, th)
                b.emit('mov', dl, tl)
            if op in ('mpyr', 'macr'):
                up = b.op('or', b.op('ltu', 0x800000, dl),
                          b.op('and', b.op('eq', dl, 0x800000), b.op('and', dh, 1)))
                b.emit('add', dh, dh, up)
                b.emit('mov', dl, 0)
            return
        if op in ('add', 'sub') and len(p) == 2 and p[1] in ACC:
            h, l = self.pair(p[0])
            d = p[1]
            (self.add56 if op == 'add' else self.sub56)(d + 'h', d + 'l', h, l, d + 'h', d + 'l')
            return
        if op in ('asr', 'asl') and len(p) == 1:
            self._instruction(Instruction(i.pc, i.size, i.words, op + ' #$1,' + p[0] + ',' + p[0]))
            return
        if op in ('asr', 'asl') and len(p) == 3 and p[1] in ACC and p[2] in ACC:
            s, d = p[1], p[2]
            if not p[0].startswith('#'):
                count = b.op('and', self.word(p[0], raw), 63)
                th, tl = b.op('mov', s + 'h'), b.op('mov', s + 'l')
                b.emit('mov', d + 'h', th)
                b.emit('mov', d + 'l', tl)
                top, done = b.label(), b.label()
                b.emit('label', top)
                b.emit('jz', count, done)
                dh, dl = d + 'h', d + 'l'
                if op == 'asr':
                    b.emit('and', dl, b.op('or', b.op('shr', dl, 1), b.op('shl', dh, 23)), 0xffffff)
                    b.emit('sar', dh, dh, 1)
                else:
                    b.emit('or', dh, b.op('shl', dh, 1), b.op('shr', dl, 23))
                    b.emit('and', dl, b.op('shl', dl, 1), 0xffffff)
                b.emit('sub', count, count, 1)
                b.emit('jmp', top)
                b.emit('label', done)
                return
            n = imm(p[0])
            if not 0 <= n < 56:
                raise Unsupported('Shift domain')
            dh, dl = d + 'h', d + 'l'
            if n == 0:
                b.emit('mov', dh, s + 'h')
                b.emit('mov', dl, s + 'l')
                return
            if op == 'asl':
                if n >= 24:
                    tl = b.op('mov', s + 'l')
                    b.emit('shl', dh, tl, n - 24) if n - 24 < 32 else b.emit('mov', dh, 0)
                    b.emit('mov', dl, 0)
                    return
                th, tl = b.op('mov', s + 'h'), b.op('mov', s + 'l')
                b.emit('or', dh, b.op('shl', th, n), b.op('shr', tl, 24 - n))
                b.emit('and', dl, b.op('shl', tl, n), 0xffffff)
                return
            if n >= 24:
                th = b.op('mov', s + 'h')
                b.emit('and', dl, b.op('sar', th, n - 24), 0xffffff)
                b.emit('sar', dh, th, min(n, 31))
                return
            th, tl = b.op('mov', s + 'h'), b.op('mov', s + 'l')
            b.emit('sar', dh, th, n)
            b.emit('and', dl, b.op('or', b.op('shr', tl, n), b.op('shl', th, 24 - n)), 0xffffff)
            return
        if op == 'and' and len(p) == 2 and p[1] in ACC:
            d = p[1] + 'h'
            b.emit('or', d, b.op('and', d, 0xff000000), b.op('and', b.u24(d), self.word(p[0], raw)))
            return
        if op == 'tst' and p[0] in ACC:
            d = p[0]
            b.emit('and', 'zero', b.op('eq', d + 'h', 0), b.op('eq', d + 'l', 0))
            return
        if op in ('neg', 'abs') and p[0] in ACC:
            d = p[0]
            skip = None
            if op == 'abs':
                skip = b.label()
                b.emit('jz', b.op('lt', d + 'h', 0), skip)
            nh, nl = self.neg56(d + 'h', d + 'l')
            b.emit('mov', d + 'h', nh)
            b.emit('mov', d + 'l', nl)
            if skip:
                b.emit('label', skip)
            return
        if op == 'cmp' and len(p) == 2 and p[1] in ACC:
            h, l = self.pair(p[0])
            d = p[1]
            th, tl = b.op('mov', h), b.op('mov', l)
            dh, dl = d + 'h', d + 'l'
            b.emit('and', 'zero', b.op('eq', dh, th), b.op('eq', dl, tl))
            b.emit('or', 'less', b.op('lt', dh, th), b.op('and', b.op('eq', dh, th), b.op('ltu', dl, tl)))
            b.emit('and', 'greater', b.op('eq', 'zero', 0), b.op('eq', 'less', 0))
            b.emit('mov', 'negative', 'less')
            return
        if op in ('tgt', 'tge', 'tlt', 'tle', 'teq', 'tne') and len(p) == 2 and p[1] in ACC:
            cond = {'tgt': 'greater', 'tge': '!less', 'tlt': 'less', 'tle': '!greater', 'teq': 'zero', 'tne': '!zero'}[op]
            skip = b.label()
            self.branch_unless(cond, skip)
            self.store(p[1], self.word(p[0], raw))
            b.emit('label', skip)
            return
        if op == 'tfr' and len(p) == 2 and p[1] in ACC:
            h, l = self.pair(p[0])
            d = p[1]
            b.emit('mov', d + 'h', h)
            b.emit('mov', d + 'l', l)
            return
        if op == 'max' and args == 'a,b':
            ge = b.op('or', b.op('lt', 'bh', 'ah'), b.op('and', b.op('eq', 'ah', 'bh'), b.op('eq', b.op('ltu', 'al', 'bl'), 0)))
            other, done = b.label(), b.label()
            b.emit('jz', ge, other)
            b.emit('mov', 'bh', 'ah')
            b.emit('mov', 'bl', 'al')
            b.emit('mov', 'carry', 0)
            b.emit('jmp', done)
            b.emit('label', other)
            b.emit('mov', 'carry', 1)
            b.emit('label', done)
            return
        if op in ('btst', 'brset', 'brclr'):
            bit = imm(p[0])
            value = self.word(p[1], raw)
            test = b.op('and', b.op('shr', value, bit), 1) if not isinstance(value, int) else (value >> bit) & 1
            if op == 'btst':
                b.emit('mov', 'carry', test)
                return
            b.emit('jz' if op == 'brclr' else 'jnz', test, f'L_{target(p[2]):x}')
            return
        if op in ('lsl', 'lsr') and len(p) == 2 and p[1] in ACC:
            n = imm(p[0])
            d = p[1] + 'h'
            if not 0 < n <= 24:
                raise Unsupported('Logical shift count')
            bit = 24 - n if op == 'lsl' else n - 1
            b.emit('and', 'carry', b.op('shr', b.u24(d), bit), 1)
            shifted = b.op('shl' if op == 'lsl' else 'shr', b.u24(d), n)
            b.emit('or', d, b.op('and', d, 0xff000000), b.op('and', shifted, 0xffffff))
            b.emit('eq', 'zero', b.u24(d), 0)
            b.emit('ltu', 'negative', 0, b.op('shr', b.u24(d), 23))
            return
        if op in ('bra', 'jmp', 'beq', 'bne', 'jeq', 'jne', 'blt', 'bgt', 'ble', 'bge', 'bmi', 'bpl', 'bcc', 'bcs'):
            cond = {'bra': None, 'jmp': None, 'beq': 'zero', 'jeq': 'zero', 'bne': '!zero', 'jne': '!zero',
                    'blt': 'less', 'bgt': 'greater', 'ble': '!greater', 'bge': '!less', 'bmi': 'negative',
                    'bpl': '!negative', 'bcc': '!carry', 'bcs': 'carry'}[op]
            dest = f'L_{target(args):x}'
            if cond is None:
                b.emit('jmp', dest)
            else:
                self.branch_if(cond, dest)
            return
        if op == 'do' and len(p) == 2:
            n = self.word(p[0], raw)
            end = target(p[1])
            body = i.pc + i.size
            if end not in self.instructions:
                raise Unsupported('DO end is not a decoded instruction')
            self.loops.setdefault(end, []).append((i.pc, body))
            counter = f'loop_{i.pc:x}'
            b.emit('mov', counter, n)
            b.emit('jz', counter, f'L_{end:x}')
            return
        if op == 'lua' and len(p) == 2 and re.fullmatch('r[0-7]', p[1]):
            m = re.fullmatch(r'\(r([0-7])([+-])\$([0-9a-f]+)\)', p[0])
            if m:
                step = int(m[3], 16)
                if m[2] == '-':
                    step = (-step) & 0xffffffff
                self.store(p[1], self.agu_index(f'r{m[1]}', f'm{m[1]}', step))
                return
            m = re.fullmatch(r'\(r([0-7])\)([+-])n([0-7])', p[0])
            if m:
                step = b.sx24(f'n{m[3]}')
                if m[2] == '-':
                    step = b.op('sub', 0, step)
                self.store(p[1], self.agu_index(f'r{m[1]}', f'm{m[1]}', step))
                return
            raise Unsupported('LUA addressing: ' + args)
        if op == 'rep':
            count = b.op('mov', self.word(args, raw))
            nexti = self.instructions[i.pc + i.size]
            if nexti.op in ('do', 'rep', 'dor', 'rts'):
                raise Unsupported('REP control instruction')
            top, done = b.label(), b.label()
            b.emit('label', top)
            b.emit('jz', count, done)
            self.instruction(nexti)
            b.emit('sub', count, count, 1)
            b.emit('jmp', top)
            b.emit('label', done)
            b.emit('jmp', f'L_{nexti.pc + nexti.size:x}')
            return
        if op == 'dor':
            m = re.fullmatch(r'>\*\+\$([0-9a-f]+)', p[1])
            if not m:
                raise Unsupported('DOR target')
            end = i.pc + int(m[1], 16)
            self.loops.setdefault(end, []).append((i.pc, i.pc + i.size))
            counter = f'loop_{i.pc:x}'
            b.emit('mov', counter, self.word(p[0], raw))
            b.emit('jz', counter, f'L_{end:x}')
            return
        raise Unsupported(f'{i.pc:#x}: {i.text}')

    def condition(self, cond):
        neg = cond.startswith('!')
        return cond.lstrip('!'), neg

    def branch_if(self, cond, dest):
        flag, neg = self.condition(cond)
        self.b.emit('jz' if neg else 'jnz', flag, dest)

    def branch_unless(self, cond, dest):
        flag, neg = self.condition(cond)
        self.b.emit('jnz' if neg else 'jz', flag, dest)

    def long_move(self, src, dest, raw):
        """Returns a callable performing the write half; the read half runs now."""
        b = self.b
        operand = src if src.startswith('l:') else dest
        m = re.fullmatch(r'l:[<>]?\$([0-9a-f]+)', operand)
        dynamic = False
        if m:
            address = int(m[1], 16)
            if address >= 256:
                raise Unsupported('Long scratch bound')
        else:
            m = re.fullmatch(r'l:\(r([0-7])\)([+-]?)(?:n([0-7]))?', operand)
            pre = False
            if not m:
                m = re.fullmatch(r'l:(-)\(r([0-7])\)', operand)
                if not m:
                    raise Unsupported('Long memory form: ' + operand)
                r, update, stride = m[2], '-', None
                pre = True
            else:
                r, update, stride = m.groups()
            dynamic = True
            if update:
                step = b.sx24(f'n{stride}') if stride else 1
                if update == '-':
                    step = b.op('sub', 0, step) if not isinstance(step, int) else (-step) & 0xffffffff
                address = self.agu_update(f'r{r}', f'm{r}', step, pre)
                address = b.op('mov', address)
            else:
                address = b.op('mov', f'r{r}')
        other = dest if src == operand else src
        if other not in ('a', 'b', 'a10', 'b10', 'x', 'y'):
            raise Unsupported('Long register pair: ' + other)
        d = other[0]
        if src == operand:
            if dynamic:
                pm, qm = b.t(), b.t()
                self.used_scratch.update('xy')
                b.emit('dread', pm, 'x', address, self.raw)
                b.emit('dread', qm, 'y', address, self.raw)
            else:
                self.used_scratch.update('xy')
                pm = b.u24(self._load('SX', address))
                qm = b.u24(self._load('SY', address))

            def finish():
                if d in ACC:
                    if other.endswith('10'):
                        high = b.op('or', b.op('and', d + 'h', 0xff000000), b.op('and', pm, 0xffffff))
                    else:
                        high = b.sx24(pm)
                    b.emit('mov', d + 'h', high)
                    b.emit('mov', d + 'l', qm)
                else:
                    b.emit('mov', d + '1', b.sx24(pm))
                    b.emit('mov', d + '0', b.sx24(qm))
            return finish
        h, l = self.pair(d)
        if d in ACC and other == d:
            h = b.u24(b.lim24(d + 'h'))
            # (dh > 0x7fffff ? 0xffffff : dh < -0x800000 ? 0 : dl)
            over = b.op('lt', 0x7fffff, d + 'h')
            under = b.op('lt', d + 'h', -0x800000)
            keep = b.op('eq', b.op('or', over, under), 0)
            l = b.op('or', b.op('and', b.op('sub', 0, over), 0xffffff), b.op('and', b.op('sub', 0, keep), d + 'l'))
        pm = b.op('mov', h)
        qm = b.op('mov', l)

        if dynamic:
            self.used_scratch.update('xy')

        def finish():
            if dynamic:
                b.emit('dwrite', 'x', address, pm)
                b.emit('dwrite', 'y', address, qm)
            else:
                self.used_scratch.update('xy')
                b.emit('store', 'SX', address, b.sx24(pm))
                b.emit('store', 'SY', address, b.sx24(qm))
        return finish

    # ------------------------------------------------------------ function
    def prepare_direct(self, code):
        for reg in range(8):
            r = f'r{reg}'
            bases, mods, escapes = [], [], False
            for i in code.values():
                if i.op == 'move':
                    for part in re.split(r'\s{2,}', i.args):
                        p = part.split(',')
                        if len(p) != 2:
                            continue
                        if p[0] == r:
                            escapes = True
                        if p[1] == r:
                            if re.fullmatch(r'#[<>]?\$[0-9a-f]+', p[0]):
                                bases.append(int(p[0].split('$')[1], 16))
                            else:
                                escapes = True
                        if p[1] == f'm{reg}':
                            if re.fullmatch(r'#[<>]?\$[0-9a-f]+', p[0]):
                                mods.append(int(p[0].split('$')[1], 16))
                            else:
                                escapes = True
                elif re.search(r',\s*' + r + r'\b', i.args):
                    escapes = True
            if reg == 7 and not bases:
                bases = [0x100]
            if reg == 7 and not mods:
                mods = [31]
            if not escapes and bases and set(mods) == {31} and len(set(x & ~31 for x in bases)) == 1:
                base = bases[0] & ~31
                if base < 128 or reg == 7:
                    self.direct[str(reg)] = base

    def tables_scan(self, code):
        for i in code.values():
            match = re.fullmatch(r'move\s+#[<>]?\$([0-9a-f]+),r[0-7]', i.text)
            if match and 0x100000 <= int(match[1], 16) < 0x148000 and self.image:
                a = int(match[1], 16)
                record = next((r for r in self.image.records if r.space == 'P' and r.address <= a < r.end), None)
                if record is None:
                    raise Unsupported(f'Unmapped immutable source {a:#x}')
                self.m.tables[record.address] = record.words

    def function(self, entry, render=True):
        code = reachable(self.instructions, entry)
        self.loops = {}
        self.direct = {}
        self.prepare_direct(code)
        self.tables_scan(code)
        b = self.b
        body = Builder()
        body.temps, body.labels = 0, 0
        self.b = body
        for pc, i in code.items():
            body.emit('label', f'L_{pc:x}')
            self.instruction(i)
            if pc + i.size in self.loops:
                for loop, start in sorted(self.loops[pc + i.size], reverse=True):
                    if loop in code:
                        c = f'loop_{loop:x}'
                        body.emit('sub', c, c, 1)
                        body.emit('jnz', c, f'L_{start:x}')
        ops = body.ops
        # Function wrapper (generate.py / dsp.function text replacements).
        prologue = []
        epilogue = []
        if self.m.buffered_output and render and 'y' in self.used_scratch:
            for k in range(256, self.m.scratch_y_words):
                prologue.append(('store', 'SY', k, 0))
        if 'y' in self.used_scratch and self.m.scratch_y_words > 32:
            t = '_y20'
            prologue += [('global_load', t, 'md_shared_y20'), ('shl', t, t, 8), ('sar', t, t, 8), ('store', 'SY', 32, t)]
            if self.m.buffered_output and render:
                epilogue += [('load', '_h', 'SY', 32), ('field_store', 'scratch_y20_history', '_h')]
                for k in range(32):
                    epilogue += [('load', f'_o{k}', 'SY', 256 + k), ('store', 'BLK', k, f'_o{k}')]
            epilogue += [('load', '_e', 'SY', 32), ('and', '_e', '_e', 0xffffff), ('global_store', 'md_shared_y20', '_e')]
        elif self.m.buffered_output and render:
            epilogue += [('load', '_h', 'SY', 32), ('field_store', 'scratch_y20_history', '_h')]
            for k in range(32):
                epilogue += [('load', f'_o{k}', 'SY', 256 + k), ('store', 'BLK', k, f'_o{k}')]
            prologue += [('field_load', '_h', 'scratch_y20_history'), ('store', 'SY', 32, '_h')]
        init = []
        for r in 'rmn':
            for k in range(8):
                v = 0
                if r == 'r' and k == 6:
                    v = 0x800
                elif r == 'r' and k == 7:
                    v = 0x100
                elif r == 'm':
                    v = 0xffffff
                init.append(('mov', f'{r}{k}', v))
        if self.m.private_memory:
            init = [o for o in init if o[1] != 'r6']
            init += [('field_load', '_vv', 'virtual_voice'), ('shl', '_vv', '_vv', 6), ('add', 'r6', '_vv', 0x800)]
        for name in ('ah', 'bh', 'al', 'bl', 'x0', 'x1', 'y0', 'y1') + FLAGS:
            init.append(('mov', name, 0))
        for pc, i in code.items():
            if i.op in ('do', 'dor'):
                init.append(('mov', f'loop_{pc:x}', 0))
        final = []
        for o in ops:
            if o[0] == 'ret':
                final += epilogue
            final.append(o)
        # Falling off the end without rts never happens in reachable code.
        return dict(ops=init + prologue + final, code=code, direct=dict(self.direct),
                    used_scratch=set(self.used_scratch), entry=entry)


def lift_machine(instructions, m, image, entry):
    return Lifter(instructions, Machine(m), image).function(entry)
