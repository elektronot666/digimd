"""SHARC+ assembly emitter for the fast IR.

Only instruction forms already present in production selcc output or the
project's hand-verified assembly are emitted (see VOCABULARY). The function
keeps the selcc C ABI: R4 = voice, R8 = sine table; R8..R15 and every other
register except R0..R7, I4, I12 and ASTAT are preserved; MODE1 and the
multiplier accumulators are never touched.

Layout of the frame (word offsets from I6, selcc convention):
  -3 .. -10   saved R8..R15
  then        voice pointer, sine pointer, private-window bases
  then        copy of the voice Y slot words, the 44.1k block, spill slots
"""
from collections import defaultdict
from tools.machines.translation.fast_opt import (blocks, dest, dests, sources, replace_sources, PURE, M32, s32, fill, liveness, Regions, MULTI,
                                                 acc_terms, make_acc)
MULTI.add('mra')

VOCABULARY = """R = R + R; R = R - R; R = R AND/OR/XOR R; R = -R; R = PASS R; R = MIN/MAX(R,R);
COMP/COMPU(R,R); IF cc R = PASS R; IF cc JUMP L; JUMP L; R = imm; R = sym.; R = LSHIFT R BY imm;
R = ASHIFT R BY R; R = FEXT R BY p:l; R = R * R (SSI/SSF); R = DM(n,I6); DM(n,I6) = R; I4 = R;
I4=MODIFY (I4,n)(NW); R = DM(I4,M5); DM(I4,M5) = R; R = DM(sym.); DM(sym.) = R; R = I6;
CJUMP sym (DB) calls; CALL/RTS local subroutines;
multifunction 'compute , R = DM(n,I6)' / 'compute , DM(n,I6) = R' (and the
DM(M4,Ii) forms) where compute is a single unconditional ALU op (including
R = R +/- 1) or multiplier op (R = R * R, MRF = [MRF +/-] R * R); selcc
output itself packs only ALU computes this way."""

ALLOC = [f'R{k}' for k in range(3, 16)]
SCRATCH = ('R0', 'R1', 'R2')


class Frame:
    def __init__(self):
        self.next = 11          # -3..-10 hold saved R8..R15
        self.slots = {}

    def slot(self, name):
        if name not in self.slots:
            self.slots[name] = self.next
            self.next += 1
        return -self.slots[name]


def lower_generic(ops, machine):
    """Expand generic AGU ops (non-constant modifier) into primitive IR."""
    out = []
    n = [0]

    def t():
        n[0] += 1
        return f'u{n[0]}'

    def lab():
        n[0] += 1
        return f'Y{n[0]}'
    for o in ops:
        if o[0] == 'agu' and isinstance(o[3], int):
            d, old, m, step = o[1:]
            if m == 0xffffff:
                nxt = t()
                out += [('add', nxt, old, step), ('and', d, nxt, 0xffffff)]
                continue
            mask = fill(m)
            base, off, c = t(), t(), t()
            fin, over = lab(), lab()
            out += [('and', base, old, (~mask) & M32), ('and', off, old, mask), ('add', off, off, step),
                    ('lt', c, off, 0), ('jz', c, over), ('add', off, off, (m + 1) & M32), ('jmp', fin),
                    ('label', over), ('ltu', c, m, off), ('jz', c, fin), ('sub', off, off, (m + 1) & M32),
                    ('label', fin), ('add', d, base, off)]
            continue
        if o[0] == 'agu' and not isinstance(o[3], int):
            d, old, m, step = o[1:]
            # md_agu: next = (old+step)&0xffffff if m==0xffffff else modulo
            nxt, mask, base, off, tmp, c = t(), t(), t(), t(), t(), t()
            linear, done, neg, over = lab(), lab(), lab(), lab()
            out += [('eq', c, m, 0xffffff), ('jnz', c, linear),
                    ('mov', mask, 1), ('ltu', c, 1, m), ('jz', c, neg + 'm')]
            out += [('mov', mask, m)]
            for sh in (1, 2, 4, 8, 16):
                out += [('shr', tmp, mask, sh), ('or', mask, mask, tmp)]
            out += [('label', neg + 'm'),
                    ('xor', tmp, mask, M32), ('and', base, old, tmp),
                    ('and', off, old, mask), ('add', off, off, step),
                    ('lt', c, off, 0), ('jz', c, over),
                    ('add', off, off, m), ('add', off, off, 1), ('jmp', neg),
                    ('label', over), ('ltu', c, m, off), ('jz', c, neg),
                    ('sub', off, off, m), ('sub', off, off, 1),
                    ('label', neg), ('add', d, base, off), ('jmp', done),
                    ('label', linear), ('add', nxt, old, step), ('and', d, nxt, 0xffffff),
                    ('label', done)]
            continue
        out.append(o)
    return out


# ------------------------------------------------------------------ register allocation
def loop_weights(ops):
    """Approximate execution weight per op: 32**depth of enclosing back edges."""
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}
    w = [1] * len(ops)
    for i, o in enumerate(ops):
        target = o[1] if o[0] == 'jmp' else o[2] if o[0] in ('jz', 'jnz') else None
        if target is not None and pos.get(target, i + 1) <= i:
            for k in range(pos[target], i + 1):
                w[k] *= 16
    return w


def allocate(ops, info=None):
    ranges, live_in, live_out = liveness(ops)
    weight = loop_weights(ops)
    edges = defaultdict(set)
    cost = defaultdict(float)
    moves = defaultdict(set)
    allvars = set()
    for bi, (s, e) in enumerate(ranges):
        live = set(live_out[bi])
        for i in reversed(range(s, e)):
            o = ops[i]
            d = dest(o)
            srcs = [x for x in sources(o) if isinstance(x, str)]
            if o[0] in MULTI:
                d = o[1]
                srcs = [x for x in o[3:] if isinstance(x, str)]
                if o[0] in ('mac24', 'mac24p'):
                    # the second result is written before the sources are read again
                    for x in srcs:
                        for y in (o[1], o[2]):
                            if x != y:
                                edges[x].add(y)
                                edges[y].add(x)
                # both results are written after both sources are read
                for x in [o[2]]:
                    allvars.add(x)
                    cost[x] += weight[i]
                    for y in live:
                        if y != x:
                            edges[x].add(y)
                            edges[y].add(x)
                    edges[x].add(d)
                    edges[d].add(x)
                    for y in srcs:
                        if y != x:
                            edges[x].add(y)
                            edges[y].add(x)
                    live.discard(x)
            for x in srcs:
                cost[x] += weight[i]
                allvars.add(x)
            if d is not None:
                allvars.add(d)
                cost[d] += weight[i]
                for x in live:
                    if x != d and not (o[0] == 'mov' and x == o[2]):
                        edges[d].add(x)
                        edges[x].add(d)
                if o[0] == 'mov' and isinstance(o[2], str):
                    moves[d].add(o[2])
                    moves[o[2]].add(d)
                # multi-instruction patterns write the destination before
                # reading every source: keep it apart from its sources
                if o[0] in ('lt', 'ltu', 'eq', 'agu', 'satlo'):
                    for x in srcs:
                        if x != d:
                            edges[d].add(x)
                            edges[x].add(d)
                live.discard(d)
            for x in srcs:
                live.add(x)
    for v in live_in[0]:
        allvars.add(v)
    # Chaitin-Briggs with optimistic coloring
    k = len(ALLOC)
    degree = {v: len(edges[v]) for v in allvars}
    stack = []
    removed = set()
    work = set(allvars)
    while work:
        low = [v for v in work if degree[v] - sum(1 for u in edges[v] if u in removed) < k]
        # ties broken by name: the result must not depend on set order
        if low:
            v = min(low, key=lambda x: (cost[x], x))
        else:
            v = min(work, key=lambda x: (cost[x] / (1 + len(edges[x])), x))
        work.discard(v)
        removed.add(v)
        stack.append(v)
    color = {}
    for v in reversed(stack):
        used = {color[u] for u in edges[v] if u in color}
        prefer = [color[u] for u in sorted(moves[v]) if u in color and color[u] not in used]
        free = [r for r in ALLOC if r not in used]
        if prefer:
            color[v] = prefer[0]
        elif free:
            color[v] = free[0]
    if info is not None:
        info.update(edges=edges, moves=moves)
    return color


def spill_groups(color, edges, moves, skip=()):
    """Move-related spilled variables that never interfere share one frame
    slot (a copy between them then needs no code): variable -> group name."""
    parent = {}

    def find(v):
        while parent.get(v, v) != v:
            v = parent[v]
        return v
    members = {}
    for v in sorted(moves):
        if v in color or v in skip:
            continue
        for u in sorted(moves[v]):
            if u in color or u in skip:
                continue
            a, b = find(v), find(u)
            if a == b:
                continue
            ga, gb = members.get(a, {a}), members.get(b, {b})
            if any(y in edges[x] for x in ga for y in gb):
                continue
            parent[b] = a
            members[a] = ga | gb
            members.pop(b, None)
    return {v: find(v) for v in parent}


# ------------------------------------------------------------------ emitter
class Emitter:
    def __init__(self, name, machine, layout, ops, source_base):
        self.name = name
        self.m = machine
        self.layout = layout        # dict: blk_byte, fault_byte, vv_byte, xp_byte, ext_byte, hist_byte
        self.ops = ops
        self.source_base = source_base
        self.lines = []
        self.frame = Frame()
        self.labels = {}
        self.slow = []
        self.nlab = 0
        self.flags = None
        self.site = 0
        self.uses_mr = False
        self.hints = None
        # I5 points at the offset region with the most variable-index traffic
        w = loop_weights(ops)
        score = defaultdict(int)
        for i, o in enumerate(ops):
            if o[0] == 'load' and o[2] in ('BLK', 'XP', 'EXT') and not isinstance(o[3], int):
                score[o[2]] += w[i]
            if o[0] == 'store' and o[1] in ('BLK', 'XP', 'EXT') and not isinstance(o[2], int):
                score[o[1]] += w[i]
        self.i5_base = max(score, key=score.get) if score else None
        self.fixups = []
        self.vv_bounded = any(o[0] == 'guard_vv' for o in ops)
        self.remat = {}
        self.groups = {}
        self.mr_holds = None

    def L(self, name):
        if name not in self.labels:
            self.labels[name] = f'.L_{self.name}_{name}'
        return self.labels[name]

    def fresh(self, tag):
        self.nlab += 1
        return f'.L_{self.name}_{tag}{self.nlab}'

    def w(self, text):
        self.lines.append('    ' + text + ';' + '\x00' + self.tag)

    # value access ---------------------------------------------------------
    def reg(self, x, scratch):
        """Register holding operand x, loading into scratch if needed."""
        if isinstance(x, int):
            self.w(f'{scratch} = 0x{x & M32:X}')
            return scratch
        r = self.color.get(x)
        if r:
            return r
        if x in self.remat:
            self.w(f'{scratch} = {self.remat[x]}')
            return scratch
        self.w(f'{scratch} = DM ({self.spill(x)},I6)')
        return scratch

    def rematerializable(self, ops, ranges):
        """Spilled variables defined once by a constant-address load whose
        every use follows in the same block with no memory write or call in
        between: reloaded from memory at the use instead of spilled."""
        defs = defaultdict(int)
        for o in ops:
            for x in ([dest(o)] + ([o[2]] if o[0] in MULTI else [])):
                if isinstance(x, str):
                    defs[x] += 1
        writes = ('dwrite', 'dread', 'global_store', 'field_store', 'guard_y', 'guard_vv', 'hwloop')

        def area(base):
            # SX and SY are the separate md_shared_scratch_x/_y arrays; every
            # voice-relative region (Y, BLK, XP, EXT) counts as one area
            return base if base in ('SX', 'SY') else 'voice'
        out = {}
        for s, e in ranges:
            for i in range(s, e):
                o = ops[i]
                if o[0] != 'load' or o[1] in self.color or defs[o[1]] != 1:
                    continue
                mem = self.direct(o[2], o[3])
                if mem is None:
                    continue
                d = o[1]
                # every use must lie before the first write in this block that
                # could change the loaded word
                uses = [j for j in range(len(ops)) if d in sources(ops[j])]
                stop = next((j for j in range(i + 1, e) if ops[j][0] in writes
                             or ops[j][0] == 'store' and area(ops[j][1]) == area(o[2])), e)
                if uses and all(i < j < stop for j in uses):
                    out[d] = mem
        return out

    def spill(self, x):
        return self.frame.slot('v:' + self.groups.get(x, x))

    def target(self, d):
        return self.color.get(d) or 'R0'

    def commit(self, d, r):
        if d in self.color:
            if self.color[d] != r:
                self.w(f'{self.color[d]} = PASS {r}')
            return
        self.w(f'DM ({self.spill(d)},I6) = {r}')

    # memory ---------------------------------------------------------------
    # I0 = voice, I1 = md_shared_scratch_x, I2 = md_shared_scratch_y, I3 = sine
    IREG = {'SX': ('I1', 0), 'SY': ('I2', 0), 'SINE': ('I3', 0)}

    def ibase(self, base):
        if base in self.IREG:
            return self.IREG[base]
        if base == self.i5_base:
            return ('I5', 0)
        if base == 'Y':
            return ('I0', 0)
        if base == 'BLK':
            return ('I0', self.layout['blk_byte'] // 4)
        if base == 'XP':
            return ('I0', self.layout['x_private_byte'] // 4)
        if base == 'EXT':
            return ('I0', self.layout['external_byte'] // 4)
        return None

    def address_into_i4(self, base, idx):
        """I4 = native byte address of base[idx - off]; returns the DM operand
        (word offset folded into the immediate)."""
        ib = self.ibase(base)
        if isinstance(idx, int):
            if ib is not None:
                ireg, off = ib
                self.w(f'I4 = {ireg}')
                k = off + idx
            else:
                self.w(f'R0 = {self.base_symbol(base)}')
                self.w('I4 = R0')
                k = idx
            return f'DM ({k},I4)' if -0x8000 <= k < 0x8000 and k else 'DM (I4,M5)' if not k else self._far(k)
        ri = self.reg(idx, 'R1')
        if ib is not None and ib[1] == 0 and not _os.environ.get('FAST_NO_PREMOD'):
            # pre-modify addressing: base + 4*M4 (NW word-scaled), base unchanged
            self.w(f'M4 = {ri}')
            return f'DM (M4,{ib[0]})'
        self.w(f'R1 = LSHIFT {ri} BY 2')
        if ib is not None:
            ireg, off = ib
            self.w(f'R0 = {ireg}')
        else:
            self.w(f'R0 = {self.base_symbol(base)}')
            off = 0
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        return f'DM ({off},I4)' if off else 'DM (I4,M5)'

    def _far(self, k):
        self.w(f'I4=MODIFY (I4,{k})(NW)')
        return 'DM (I4,M5)'

    def base_symbol(self, base):
        if base == 'SRC':
            return f'0x{self.source_base:X}'
        sym = f'source_{base[1:]}.'
        self.externs.add(sym)
        return sym

    def direct(self, base, idx):
        """'DM (k,In)' operand for a constant element, or None."""
        ib = self.ibase(base)
        if ib is None or not isinstance(idx, int):
            return None
        ireg, off = ib
        return f'DM ({off + idx},{ireg})'

    def load(self, d, base, idx):
        if d in self.remat:
            return
        t = self.target(d)
        mem = self.direct(base, idx)
        if mem is None:
            mem = self.address_into_i4(base, idx)
        self.w(f'{t} = {mem}')
        self.commit(d, t)

    def store(self, base, idx, s):
        mem = self.direct(base, idx)
        if mem is None:
            mem = self.address_into_i4(base, idx)
        r = self.reg(s, 'R2')
        self.w(f'{mem} = {r}')

    # generic dispatch -------------------------------------------------------
    def hint_window(self, space, write):
        """Profiled region for the next generic site as a clean window, or None."""
        k = self.site
        self.site += 1
        h = self.hints[k] if self.hints and k < len(self.hints) else None
        if h is None:
            return None
        name, lo, hi = h
        if name in ('SX', 'SY', 'BLK31', 'ZERO', 'FAULT', 'BOOTX', 'CONST_FDB42B'):
            return None
        regions = Regions(self.m)
        fn = regions.write if write else regions.read
        w = regions.clean(fn, space, lo, hi, self.vv_bounded)
        if w is None or w[0] != name:
            return None
        if name not in ('SRC', 'SINE', 'EXT', 'XP', 'Y') and not name.startswith('T'):
            return None
        if write and name not in ('EXT', 'XP', 'Y'):
            return None
        return w

    def window_check(self, w, miss):
        """R1 = address - A (A shifted by the virtual voice); jump to miss
        (with R1 restored) unless 0 <= R1 < B - A."""
        name, A, B, origin0, step = w
        restore = self.fresh('wr')
        if step:
            slot = {'EXT': 'w:ext', 'XP': 'w:xp', 'Y': 'w:yw'}[name]
            self.w(f'R0 = DM ({self.frame.slot(slot)},I6)')
            self.w('R1 = R1 - R0')
            if A != origin0:
                self.w(f'R0 = 0x{(A - origin0) & M32:X}')
                self.w('R1 = R1 - R0')
        else:
            self.w(f'R0 = 0x{A:X}')
            self.w('R1 = R1 - R0')
        self.w(f'R0 = 0x{B - A:X}')
        self.w('COMPU (R1,R0)')
        self.w(f'IF GE JUMP {restore}')
        self.fixups.append((restore, miss, w))

    def window_address(self, w):
        """I4 = byte address of the element; returns the DM offset operand."""
        name, A, B, origin0, step = w
        if name == 'SINE':
            ireg, off = 'I3', A - 0x148000
        elif name in ('EXT', 'XP', 'Y'):
            ireg, word = self.ibase(name)
            off = word + (A - origin0)
        else:
            ireg, off = None, None
        if off == 0:
            self.w('M4 = R1')
            return f'DM (M4,{ireg})'
        self.w('R1 = LSHIFT R1 BY 2')
        if name == 'SRC':
            self.w(f'R0 = 0x{self.source_base + 4 * (A - 0x20000):X}')
            self.w('R1 = R1 + R0')
            self.w('I4 = R1')
            return 'DM (I4,M5)'
        if name.startswith('T'):
            sym = f'source_{name[1:]}.'
            self.externs.add(sym)
            self.w(f'R0 = {sym}')
            self.w('R1 = R1 + R0')
            self.w('I4 = R1')
            off = A - int(name[1:], 16)
            return f'DM ({off},I4)' if off else 'DM (I4,M5)'
        if name == 'SINE':
            ireg, off = 'I3', A - 0x148000
        else:
            ireg, word = self.ibase(name)
            off = word + (A - origin0)
        self.w(f'R0 = {ireg}')
        self.w('R1 = R1 + R0')
        self.w('I4 = R1')
        return f'DM ({off},I4)' if off else 'DM (I4,M5)'

    def dread(self, d, space, addr, raw):
        """Inline fast path (profiled window or scratch); full chain out of line."""
        ra = self.reg(addr, 'R1')
        if ra != 'R1':
            self.w(f'R1 = PASS {ra}')
        back = self.fresh('back')
        slow = self.fresh('slow')
        w = self.hint_window(space, False) if space in ('x', 'y', 'src') else None
        if w is not None:
            self.window_check(w, slow)
            mem = self.window_address(w)
            self.w(f'R0 = {mem}')
            if w[0] != 'SRC' and not w[0].startswith('T'):
                self.w('R0 = FEXT R0 BY 0:24')
        elif space in ('x', 'y'):
            limit = self.m.scratch_x_words if space == 'x' else self.m.scratch_y_words
            self.w(f'R0 = 0x{limit:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF GE JUMP {slow}')
            self.w('M4 = R1')
            self.w(f'R0 = DM (M4,{"I1" if space == "x" else "I2"})')
            self.w('R0 = FEXT R0 BY 0:24')
        else:
            self.w(f'JUMP {slow}')
        self.lines.append(f'{back}:')
        self.commit(d, 'R0')
        self.slow.append((slow, back, 'read', space, raw))

    def dwrite(self, space, addr, s):
        rs = self.reg(s, 'R2')
        if rs != 'R2':
            self.w(f'R2 = PASS {rs}')
        ra = self.reg(addr, 'R1')
        if ra != 'R1':
            self.w(f'R1 = PASS {ra}')
        back = self.fresh('back')
        slow = self.fresh('slow')
        w = self.hint_window(space, True)
        if w is not None:
            self.window_check(w, slow)
            mem = self.window_address(w)
            self.sx24('R2', 'R2')
            self.w(f'{mem} = R2')
        else:
            limit = self.m.scratch_x_words if space == 'x' else self.m.scratch_y_words
            self.w(f'R0 = 0x{limit:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF GE JUMP {slow}')
            self.sx24('R2', 'R2')
            self.w('M4 = R1')
            self.w(f'DM (M4,{"I1" if space == "x" else "I2"}) = R2')
        self.lines.append(f'{back}:')
        self.slow.append((slow, back, 'write', space, False))

    def sx24(self, d, a):
        self.w(f'{d} = LSHIFT {a} BY 8')
        self.w('R0 = -8')
        self.w(f'{d} = ASHIFT {d} BY R0')

    def emit_slow(self):
        """Out-of-line full md_read_*/md_write_* chains (R1 = address, R2 = value)."""
        for restore, miss, w in self.fixups:
            name, A, B, origin0, step = w
            self.lines.append(f'{restore}:')
            if step:
                slot = {'EXT': 'w:ext', 'XP': 'w:xp', 'Y': 'w:yw'}[name]
                self.w(f'R0 = DM ({self.frame.slot(slot)},I6)')
                self.w('R1 = R1 + R0')
                if A != origin0:
                    self.w(f'R0 = 0x{(A - origin0) & M32:X}')
                    self.w('R1 = R1 + R0')
            else:
                self.w(f'R0 = 0x{A:X}')
                self.w('R1 = R1 + R0')
            self.w(f'JUMP {miss}')
        for slow, back, kind, space, raw in self.slow:
            self.lines.append(f'{slow}:')
            self.w('R1 = FEXT R1 BY 0:24')
            done = back
            if kind == 'read':
                if raw:
                    nz = self.fresh('raw')
                    self.w('R0 = 0xFFFF80')
                    self.w('COMPU (R1,R0)')
                    self.w(f'IF LT JUMP {nz}')
                    self.w('R0 = 0')
                    self.w(f'JUMP {done}')
                    self.lines.append(f'{nz}:')
                self.read_chain(space, done)
            else:
                self.write_chain(space, done)

    def range_check(self, lo, hi, miss):
        """Jump to miss unless lo <= R1 < hi (R0 clobbered)."""
        self.w(f'R0 = 0x{lo:X}')
        self.w('COMPU (R1,R0)')
        self.w(f'IF LT JUMP {miss}')
        self.w(f'R0 = 0x{hi:X}')
        self.w('COMPU (R1,R0)')
        self.w(f'IF GE JUMP {miss}')

    def read_chain(self, space, done):
        m = self.m
        limit = m.scratch_x_words if space == 'x' else m.scratch_y_words
        if space == 'src':
            nxt = self.fresh('n')
            self.range_check(0x20000, 0x200000, nxt)
            self.source_read(done)
            self.lines.append(f'{nxt}:')
            nxt2 = self.fresh('n')
            self.range_check(0x200000, 0xFFFF80, nxt2)
            self.w('R0 = 0')
            self.w(f'JUMP {done}')
            self.lines.append(f'{nxt2}:')
            self.fault()
            self.w('R0 = 0')
            self.w(f'JUMP {done}')
            return
        nxt = self.fresh('n')
        self.range_check(0, limit, nxt)
        self.w('R0 = LSHIFT R1 BY 2')
        self.w(f'R2 = {"I1" if space == "x" else "I2"}')
        self.w('R0 = R0 + R2')
        self.w('I4 = R0')
        self.w('R0 = DM (I4,M5)')
        self.w('R0 = FEXT R0 BY 0:24')
        self.w(f'JUMP {done}')
        self.lines.append(f'{nxt}:')
        if space == 'x':
            boot, nb = self.fresh('boot'), self.fresh('n')
            self.range_check(256, 0x800, nb)
            self.w(f'JUMP {boot}')
            self.lines.append(f'{nb}:')
            nb2 = self.fresh('n')
            self.range_check(0xc00, 0x20000, nb2)
            self.lines.append(f'{boot}:')
            self.call_c('md_boot_x_read.', done)
            self.lines.append(f'{nb2}:')
        if m.buffered_output and space == 'y':
            n2 = self.fresh('n')
            self.w('R0 = 0x1FF')
            self.w('COMP (R1,R0)')
            self.w(f'IF NE JUMP {n2}')
            self.w('R0 = 0xFDB42B')
            self.w(f'JUMP {done}')
            self.lines.append(f'{n2}:')
        if m.private_memory:
            self.private_region('ext', 1536, 'EXT', done, write=False)
            if space == 'x':
                self.private_region('xp', 64, 'XP', done, write=False)
        if space == 'y':
            ny = self.fresh('n')
            if m.private_memory:
                self.private_region('yw', 64, 'Y', done, write=False)
            else:
                self.range_check(0x800, 0x800 + m.words, ny)
                self.w('R0 = 0x800')
                self.w('R1 = R1 - R0')
                self.region_rw('Y', done, write=False)
                self.lines.append(f'{ny}:')
            if not m.buffered_output:
                nb = self.fresh('n')
                self.range_check(0x100, 0x140, nb)
                self.w('R1 = FEXT R1 BY 0:5')
                self.region_rw('BLK', done, write=False)
                self.lines.append(f'{nb}:')
        ns = self.fresh('n')
        self.range_check(0x148000, 0x150000, ns)
        self.w('R0 = 0x148000')
        self.w('R1 = R1 - R0')
        self.w('R0 = I3')
        self.w('R1 = LSHIFT R1 BY 2')
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        self.w('R0 = DM (I4,M5)')
        self.w('R0 = FEXT R0 BY 0:24')
        self.w(f'JUMP {done}')
        self.lines.append(f'{ns}:')
        for base, values in sorted(m.tables.items()):
            nt = self.fresh('n')
            self.range_check(base, base + len(values), nt)
            self.w(f'R0 = 0x{base:X}')
            self.w('R1 = R1 - R0')
            sym = f'source_{base:x}.'
            self.externs.add(sym)
            self.w('R1 = LSHIFT R1 BY 2')
            self.w(f'R0 = {sym}')
            self.w('R0 = R0 + R1')
            self.w('I4 = R0')
            self.w('R0 = DM (I4,M5)')
            self.w(f'JUMP {done}')
            self.lines.append(f'{nt}:')
        if m.requires_source_image:
            n1 = self.fresh('n')
            self.range_check(0x20000, 0x200000, n1)
            self.source_read(done)
            self.lines.append(f'{n1}:')
            n2 = self.fresh('n')
            self.range_check(0x200000, 0xFFFF80, n2)
            self.w('R0 = 0')
            self.w(f'JUMP {done}')
            self.lines.append(f'{n2}:')
        self.fault()
        self.w('R0 = 0')
        self.w(f'JUMP {done}')

    def write_chain(self, space, done):
        m = self.m
        limit = m.scratch_x_words if space == 'x' else m.scratch_y_words
        nxt = self.fresh('n')
        self.range_check(0, limit, nxt)
        self.sx24('R2', 'R2')
        self.w('R0 = LSHIFT R1 BY 2')
        self.w(f'R1 = {"I1" if space == "x" else "I2"}')
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        self.w('DM (I4,M5) = R2')
        self.w(f'JUMP {done}')
        self.lines.append(f'{nxt}:')
        if m.buffered_output and space == 'y':
            n2, ok = self.fresh('n'), self.fresh('ok')
            self.w('R0 = 0x1FF')
            self.w('COMP (R1,R0)')
            self.w(f'IF NE JUMP {n2}')
            self.w('R0 = FEXT R2 BY 0:24')
            self.w('R1 = 0xFDB42B')
            self.w('COMP (R0,R1)')
            self.w(f'IF EQ JUMP {done}')
            self.fault()
            self.w(f'JUMP {done}')
            self.lines.append(f'{n2}:')
        if m.private_memory:
            self.private_region('ext', 1536, 'EXT', done, write=True)
            if space == 'x':
                self.private_region('xp', 64, 'XP', done, write=True)
        if space == 'y':
            if m.private_memory:
                self.private_region('yw', 64, 'Y', done, write=True)
            else:
                ny = self.fresh('n')
                self.range_check(0x800, 0x800 + m.words, ny)
                self.w('R0 = 0x800')
                self.w('R1 = R1 - R0')
                self.region_rw('Y', done, write=True)
                self.lines.append(f'{ny}:')
            if not m.buffered_output:
                nb = self.fresh('n')
                self.range_check(0x100, 0x140, nb)
                self.w('R1 = FEXT R1 BY 0:5')
                self.region_rw('BLK', done, write=True)
                self.lines.append(f'{nb}:')
        self.fault()
        self.w(f'JUMP {done}')

    def private_region(self, key, size, base, done, write):
        """Window [frame base, +size) relative to the virtual voice."""
        miss = self.fresh('n')
        self.w(f'R0 = DM ({self.frame.slot("w:" + key)},I6)')
        self.w('COMPU (R1,R0)')
        self.w(f'IF LT JUMP {miss}')
        self.w('R1 = R1 - R0')
        self.w(f'R0 = 0x{size:X}')
        self.w('COMPU (R1,R0)')
        restore = self.fresh('r')
        self.w(f'IF GE JUMP {restore}')
        self.region_rw(base, done, write)
        self.lines.append(f'{restore}:')
        # undo the subtraction for the remaining chain
        self.w(f'R0 = DM ({self.frame.slot("w:" + key)},I6)')
        self.w('R1 = R1 + R0')
        self.lines.append(f'{miss}:')

    def region_rw(self, base, done, write):
        """R1 = word index into base; R2 = value (write). Read result in R0."""
        ireg, off = self.ibase(base)
        self.w(f'R0 = {ireg}')
        self.w('R1 = LSHIFT R1 BY 2')
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        if off:
            self.w(f'I4=MODIFY (I4,{off})(NW)')
        if write:
            self.sx24('R2', 'R2')
            self.w('DM (I4,M5) = R2')
        else:
            self.w('R0 = DM (I4,M5)')
            self.w('R0 = FEXT R0 BY 0:24')
        self.w(f'JUMP {done}')

    def source_read(self, done):
        self.w('R0 = 0x20000')
        self.w('R1 = R1 - R0')
        self.w('R1 = LSHIFT R1 BY 2')
        self.w(f'R0 = 0x{self.source_base:X}')
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        self.w('R0 = DM (I4,M5)')
        self.w(f'JUMP {done}')

    def fault(self):
        self.w('R0 = 1')
        self.w(f'DM ({self.layout["fault_byte"] // 4},I0) = R0')

    def _acc_sum(self, gens, rh, rl):
        """rh:rl = sum of +-(h:l) with rl normalized to 24 bits (rh, rl scratch)."""
        def total(items, into):
            # positive terms first; the first two in one add (as 'acc')
            items = sorted(items, key=lambda x: -x[0])
            g, x = items[0]
            rest = items[1:]
            if g > 0 and rest:
                r = self.reg(x, into)
                g2, x2 = rest[0]
                r2 = self.reg(x2, 'R2')
                self.w(f'{into} = {r} {"+" if g2 > 0 else "-"} {r2}')
                rest = rest[1:]
            elif g > 0:
                r = self.reg(x, into)
                if r != into:
                    self.w(f'{into} = PASS {r}')
            else:
                r = self.reg(x, 'R2')
                self.w(f'{into} = -{r}')
            for g, x in rest:
                r = self.reg(x, 'R2')
                self.w(f'{into} = {into} {"+" if g > 0 else "-"} {r}')
        ls = [(g, l) for g, h, l in gens if l != 0]
        hs = [(g, h) for g, h, l in gens if h != 0]
        if ls:
            total(ls, rl)
        else:
            self.w(f'{rl} = 0')
        if hs:
            total(hs, rh)
        else:
            self.w(f'{rh} = 0')
        # low words are 24-bit unsigned: with no subtracted low word the
        # carry is the plain top byte (as in 'acc')
        if any(g < 0 for g, _ in ls):
            self.w('R2 = -24')
            self.w(f'R2 = ASHIFT {rl} BY R2')
        else:
            self.w(f'R2 = FEXT {rl} BY 24:8')
        self.w(f'{rh} = {rh} + R2')
        self.w(f'{rl} = FEXT {rl} BY 0:24')

    def tail_call(self, name):
        """Render through another function with the same arguments, then return."""
        sym = name + '.'
        if sym != self.name + '.':
            self.externs.add(sym)
        self.w('R4 = I0')
        self.w('R8 = I3')
        ret = self.fresh('ret')
        self.w('R2 = I6')
        self.lines.append(f'    CJUMP {sym} (DB);')
        self.lines.append('    DM (I7,M7)=R2;')
        self.lines.append(f'    DM (I7,M7)={ret}-1;')
        self.lines.append(f'{ret}:')
        self.w(f'JUMP {self.L("__exit")}')

    def call_c(self, sym, done):
        """R0 = sym(R1) under the selcc ABI; caller-saved R3..R7 preserved."""
        self.externs.add(sym)
        for k in range(3, 8):
            self.w(f'DM ({self.frame.slot(f"c:R{k}")},I6) = R{k}')
        self.w('R4 = PASS R1')
        ret = self.fresh('ret')
        self.w('R2 = I6')
        self.lines.append(f'    CJUMP {sym} (DB);')
        self.lines.append('    DM (I7,M7)=R2;')
        self.lines.append(f'    DM (I7,M7)={ret}-1;')
        self.lines.append(f'{ret}:')
        for k in range(3, 8):
            self.w(f'R{k} = DM ({self.frame.slot(f"c:R{k}")},I6)')
        self.w(f'JUMP {done}')

    # main -----------------------------------------------------------------
    def emit(self):
        ops = self.ops
        info = {}
        self.color = allocate(ops, info)
        self.externs = set()
        used_bases = {o[2] for o in ops if o[0] == 'load'} | {o[1] for o in ops if o[0] == 'store'}
        generic_y = any(o[0] in ('dread', 'dwrite') and o[2] == 'y' for o in ops)
        self.frame_regions = set()
        body = []
        self.lines = body
        self.tag = 'pre'
        # comparisons consumed only by the following conditional jump
        ranges, live_in, live_out = liveness(ops)
        self.remat = {} if _os.environ.get('FAST_NO_REMAT') else self.rematerializable(ops, ranges)
        self.groups = {} if _os.environ.get('FAST_NO_SLOTSHARE') else \
            spill_groups(self.color, info['edges'], info['moves'], self.remat)
        block_of = {}
        for bi, (s, e) in enumerate(ranges):
            for i in range(s, e):
                block_of[i] = bi
        self.fuse = set()
        for i, o in enumerate(ops[:-1]):
            q = ops[i + 1]
            if o[0] in ('lt', 'ltu', 'eq') and q[0] in ('jz', 'jnz') and q[1] == o[1] \
                    and o[1] not in live_out[block_of[i + 1]]:
                self.fuse.add(i)
        self.pending = None
        self.hw = []
        for i, o in enumerate(ops):
            self.index = i
            self.tag = o[0] if o[0] not in ('add', 'sub') or not isinstance(o[1], str) or not o[1].startswith(('g', 'r')) else 'ptr'
            self.op(o)
        self.tag = 'slow'
        self.emit_slow()
        return body

    # ops that never touch the multiplier result register (others forget
    # what it holds: calls, out-of-line paths, control flow)
    MR_SAFE = {'mov', 'add', 'sub', 'or', 'xor', 'and', 'min', 'max', 'shl', 'shr', 'sar', 'sarv', 'lt', 'ltu',
               'eq', 'satlo', 'mulhi', 'mullo', 'mulpair', 'mac24', 'mac24p', 'acc', 'load', 'store',
               'global_load', 'global_store', 'field_load', 'field_store'}

    def op(self, o):
        k = o[0]
        if k != 'mra' and (k not in self.MR_SAFE or self.mr_holds and set(dests(o)) & set(self.mr_holds)):
            self.mr_holds = None
        before = len(self.lines)
        keep = self.flags
        self.flags = None
        if k in ('jz', 'jnz'):
            self.flags = keep
        self._op(o)
        if k in ('jz', 'jnz'):
            self.flags = None
        elif k in ('add', 'sub', 'or', 'xor', 'min', 'max') and o[1] in self.color:
            last = self.lines[-1] if len(self.lines) > before else ''
            if last.strip().startswith(self.color[o[1]] + ' =') and 'FEXT' not in last:
                self.flags = o[1]
        elif k == 'mulpair':
            self.flags = o[1] if o[1] in self.color else None

    def _op(self, o):
        k = o[0]
        if k == 'hwloop':
            end = self.fresh('hwend')
            head = self.ops[self.index + 1]
            assert head[0] == 'label'
            self.hw.append((end, head[1], len(self.lines)))
            self.w(f'LCNTR = 0x{o[1]:X} , DO (PC,{end})UNTIL LCE')
            return
        if k == 'jnz' and o[1] == 1 and self.hw and self.hw[-1][1] == o[2]:
            end, head, start = self.hw.pop()
            body = [l for l in self.lines[start + 1:] if not l.endswith(':')]
            pad = max(0, 3 - len(body))
            if body and ('JUMP' in body[-1] or 'CALL' in body[-1] or 'RTS' in body[-1]):
                pad = max(pad, 1)
            for _ in range(pad):
                self.w('NOP')
            # the DO target labels the loop's last instruction
            last = max(k for k in range(start + 1, len(self.lines)) if not self.lines[k].endswith(':'))
            self.lines.insert(last, f'{end}:')
            return
        if k == 'guard_y':
            ok, miss = self.fresh('gy'), self.fresh('gmiss')
            kk, lo, hi, s = o[1:]
            self.w(f'R1 = DM ({kk},I0)')
            if s:
                self.w(f'R2 = DM ({self.layout["virtual_voice_byte"] // 4},I0)')
                self.w(f'R0 = 0x{s:X}')
                self.w('R2 = R2 * R0 (SSI)')
                self.w('R1 = R1 - R2')
            if lo:
                self.w(f'R0 = 0x{lo:X}')
                self.w('R1 = R1 - R0')
            self.w(f'R0 = 0x{hi - lo + 1:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF LT JUMP {ok}')
            self.lines.append(f'{miss}:')
            self.tail_call(self.generic)
            self.lines.append(f'{ok}:')
            return
        if k == 'guard_vv':
            ok = self.fresh('vv')
            self.w(f'R1 = DM ({self.layout["virtual_voice_byte"] // 4},I0)')
            self.w(f'R0 = 0x{o[1]:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF LT JUMP {ok}')
            self.tail_call(self.fallback)
            self.lines.append(f'{ok}:')
            return
        if k == 'label':
            self.lines.append(f'{self.L(o[1])}:')
            return
        if k == 'jmp':
            self.w(f'JUMP {self.L(o[1])}')
            return
        if k in ('jz', 'jnz') and self.pending is not None:
            cond = self.pending
            self.pending = None
            if k == 'jz':
                cond = {'LT': 'GE', 'EQ': 'NE'}[cond]
            self.w(f'IF {cond} JUMP {self.L(o[2])}')
            return
        if k in ('jz', 'jnz'):
            if self.flags != o[1] or isinstance(o[1], int):
                r = self.reg(o[1], 'R1')
                self.w(f'{r} = PASS {r}')
            self.w(f'IF {"EQ" if k == "jz" else "NE"} JUMP {self.L(o[2])}')
            return
        if k == 'ret':
            self.w(f'JUMP {self.L("__exit")}')
            return
        if k == 'mov':
            d, a = o[1], o[2]
            t = self.target(d)
            if isinstance(a, int):
                self.w(f'{t} = 0x{a & M32:X}')
            else:
                if d not in self.color and a not in self.color and a not in self.remat \
                        and self.groups.get(d, d) == self.groups.get(a, a):
                    return
                ra = self.reg(a, 'R1')
                if d not in self.color:
                    # spilled destination: store the source register itself
                    self.commit(d, ra)
                    return
                if ra != t:
                    self.w(f'{t} = PASS {ra}')
            self.commit(d, t)
            return
        if k in ('add', 'sub', 'or', 'xor', 'and', 'min', 'max'):
            d, a, b = o[1:]
            t = self.target(d)
            if k == 'and' and isinstance(b, int) and b and ((b + 1) & b) == 0:
                ra = self.reg(a, 'R1')
                self.w(f'{t} = FEXT {ra} BY 0:{b.bit_length()}')
                self.commit(d, t)
                return
            if k == 'and' and isinstance(a, int) and a and ((a + 1) & a) == 0:
                rb = self.reg(b, 'R1')
                self.w(f'{t} = FEXT {rb} BY 0:{a.bit_length()}')
                self.commit(d, t)
                return
            if k == 'sub' and a == 0:
                rb = self.reg(b, 'R2')
                self.w(f'{t} = -{rb}')
                self.commit(d, t)
                return
            # increment / decrement forms need no constant register
            step = None
            if k == 'add' and isinstance(b, int) and b & M32 in (1, M32):
                x, step = a, 1 if b & M32 == 1 else -1
            elif k == 'add' and isinstance(a, int) and a & M32 in (1, M32):
                x, step = b, 1 if a & M32 == 1 else -1
            elif k == 'sub' and isinstance(b, int) and b & M32 in (1, M32):
                x, step = a, -1 if b & M32 == 1 else 1
            if step is not None and not isinstance(x, int):
                rx = self.reg(x, 'R1')
                self.w(f'{t} = {rx} {"+" if step > 0 else "-"} 1')
                self.commit(d, t)
                return
            ra = self.reg(a, 'R1')
            rb = self.reg(b, 'R2')
            sym = {'add': '+', 'sub': '-', 'or': 'OR', 'xor': 'XOR', 'and': 'AND'}.get(k)
            if sym:
                self.w(f'{t} = {ra} {sym} {rb}')
            else:
                self.w(f'{t} = {k.upper()}({ra},{rb})')
            self.commit(d, t)
            return
        if k in ('shl', 'shr', 'sar'):
            d, a, n = o[1:]
            t = self.target(d)
            ra = self.reg(a, 'R1')
            if n == 0:
                if ra != t:
                    self.w(f'{t} = PASS {ra}')
            elif k == 'shl':
                self.w(f'{t} = LSHIFT {ra} BY {n}')
            elif k == 'shr':
                self.w(f'{t} = FEXT {ra} BY {n}:{32 - n}')
            else:
                self.w(f'R0 = {-n}')
                self.w(f'{t} = ASHIFT {ra} BY R0')
            self.commit(d, t)
            return
        if k in ('lt', 'ltu', 'eq'):
            d, a, b = o[1:]
            t = self.target(d)
            ra = self.reg(a, 'R1')
            rb = self.reg(b, 'R2')
            self.w(f'{"COMPU" if k == "ltu" else "COMP"} ({ra},{rb})')
            if self.index in self.fuse:
                self.pending = 'EQ' if k == 'eq' else 'LT'
                return
            self.w('R1 = 1')
            self.w(f'{t} = 0')
            self.w(f'IF {"EQ" if k == "eq" else "LT"} {t} = PASS R1')
            self.commit(d, t)
            return
        if k == 'satlo':
            # k if c1 < h, else 0 if h < c2, else l; R2 holds each constant
            # only after the previous compare (loads keep the flags)
            d, h, l, c1, c2, kk = o[1:]
            t = self.target(d)
            rl = self.reg(l, 'R1')
            self.w(f'{t} = PASS {rl}')
            rh = self.reg(h, 'R1')
            r2 = self.reg(c2, 'R2')
            self.w(f'COMP ({rh},{r2})')
            self.w('R2 = 0')
            self.w(f'IF LT {t} = PASS R2')
            r1 = self.reg(c1, 'R2')
            self.w(f'COMP ({r1},{rh})')
            rk = self.reg(kk, 'R2')
            self.w(f'IF LT {t} = PASS {rk}')
            self.commit(d, t)
            return
        if k == 'mulhi':
            d, a, b = o[1:]
            ra = self.reg(a, 'R1')
            rb = self.reg(b, 'R2')
            self.w(f'R0 = {ra} * {rb} (SSI)')
            self.w(f'R1 = {ra} * {rb} (SSF)')
            self.w('R0 = FEXT R0 BY 23:8')
            self.w('R1 = LSHIFT R1 BY 8')
            t = self.target(d)
            self.w(f'{t} = R1 OR R0')
            self.commit(d, t)
            return
        if k == 'mulpair':
            dh, dl, a, b = o[1:]
            ra = self.reg(a, 'R1')
            rb = self.reg(b, 'R2')
            self.w(f'R0 = {ra} * {rb} (SSI)')
            self.w(f'R1 = {ra} * {rb} (SSF)')
            self.commit(dl, 'R0')
            self.w('R0 = FEXT R0 BY 23:8')
            self.w('R1 = LSHIFT R1 BY 8')
            t = self.target(dh)
            self.w(f'{t} = R1 OR R0')
            self.commit(dh, t)
            return
        if k in ('mac24', 'mac24p'):
            H, L, a, b = o[1:]
            if k == 'mac24':
                ra = self.reg(a, 'R1')
                self.w(f'R1 = LSHIFT {ra} BY 8')
                a8 = 'R1'
            else:
                a8 = self.reg(a, 'R1')
            rb = self.reg(b, 'R2')
            self.w(f'R0 = {a8} * {rb} (SSI)')
            tl = self.target(L)
            self.w(f'{tl} = FEXT R0 BY 7:24')
            self.commit(L, tl)
            th = self.target(H)
            self.w(f'{th} = {a8} * {rb} (SSF)')
            self.commit(H, th)
            return
        if k == 'mra':
            self.uses_mr = True
            dh, dl, spec = o[1], o[2], o[3]
            args = o[4:]
            gens = [(sg, args[2 * n], args[2 * n + 1]) for n, (kind, sg) in enumerate(spec) if kind == 'g']
            prods = [(kind, sg, args[2 * n], args[2 * n + 1]) for n, (kind, sg) in enumerate(spec) if kind != 'g']
            gens = [(sg, h, l) for sg, h, l in gens if not (h == 0 and l == 0)]
            held = self.mr_holds
            self.mr_holds = None
            if len(gens) == 1 and gens[0][0] > 0 and held == gens[0][1:] and prods \
                    and not _os.environ.get('FAST_NO_MRKEEP'):
                # MRF still holds h:l << 8 from the previous accumulation
                # (its low byte is zero: every product is of a << 8)
                first = False
            elif gens:
                if len(gens) == 1 and gens[0][0] > 0:
                    _, h, l = gens[0]
                    rh = self.reg(h, 'R1')
                    self.w(f'MR1F = {rh}')
                    rl = self.reg(l, 'R2')
                    self.w(f'R0 = LSHIFT {rl} BY 8')
                    self.w('MR0F = R0')
                else:
                    # several (or negated) generic terms: normalized ALU sum first
                    self._acc_sum(gens, 'R1', 'R0')
                    self.w('MR1F = R1')
                    self.w('R0 = LSHIFT R0 BY 8')
                    self.w('MR0F = R0')
                first = False
            else:
                first = True
            for kind, sg, a, b in prods:
                if kind == 'p':
                    ra = self.reg(a, 'R1')
                    self.w(f'R1 = LSHIFT {ra} BY 8')
                    a8 = 'R1'
                else:
                    a8 = self.reg(a, 'R1')
                rb = self.reg(b, 'R2')
                if first and sg > 0:
                    self.w(f'MRF = {a8} * {rb} (SSF)')
                elif first:
                    self.w('MRF = 0')
                    self.w(f'MRF = MRF - {a8} * {rb} (SSF)')
                else:
                    self.w(f'MRF = MRF {"+" if sg > 0 else "-"} {a8} * {rb} (SSF)')
                first = False
            th = self.target(dh)
            self.w(f'{th} = MR1F')
            self.commit(dh, th)
            self.w('R0 = MR0F')
            tl = self.target(dl)
            self.w(f'{tl} = FEXT R0 BY 8:24')
            self.commit(dl, tl)
            if dh != dl:
                self.mr_holds = (dh, dl)
            return
        if k == 'acc':
            dh, dl = o[1], o[2]
            terms = acc_terms(o)
            hs = sorted([(g, h) for g, h, l in terms if h != 0], key=lambda x: -x[0])
            ls = sorted([(g, l) for g, h, l in terms if l != 0], key=lambda x: -x[0])

            def total(items, into):
                if not items:
                    self.w(f'{into} = 0')
                    return
                g, x = items[0]
                rest = items[1:]
                if g > 0 and rest:
                    r = self.reg(x, into)
                    g2, x2 = rest[0]
                    r2 = self.reg(x2, 'R2')
                    self.w(f'{into} = {r} {"+" if g2 > 0 else "-"} {r2}')
                    rest = rest[1:]
                elif g > 0:
                    r = self.reg(x, into)
                    if r != into:
                        self.w(f'{into} = PASS {r}')
                else:
                    r = self.reg(x, 'R2')
                    self.w(f'{into} = -{r}')
                for g, x in rest:
                    r = self.reg(x, 'R2')
                    self.w(f'{into} = {into} {"+" if g > 0 else "-"} {r}')
            total(ls, 'R0')
            total(hs, 'R1')
            if any(g < 0 for g, _ in ls):
                self.w('R2 = -24')
                self.w('R2 = ASHIFT R0 BY R2')
            else:
                self.w('R2 = FEXT R0 BY 24:8')
            th = self.color.get(dh) or 'R1'
            self.w(f'{th} = R1 + R2')
            self.commit(dh, th)
            tl = self.target(dl)
            self.w(f'{tl} = FEXT R0 BY 0:24')
            self.commit(dl, tl)
            return
        if k == 'sarv':
            d, a, c = o[1:]
            t = self.target(d)
            ra = self.reg(a, 'R1')
            rc = self.reg(c, 'R2')
            self.w(f'{t} = ASHIFT {ra} BY {rc}')
            self.commit(d, t)
            return
        if k == 'mullo':
            d, a, b = o[1:]
            ra = self.reg(a, 'R1')
            rb = self.reg(b, 'R2')
            t = self.target(d)
            self.w(f'{t} = {ra} * {rb} (SSI)')
            self.commit(d, t)
            return
        if k == 'load':
            self.load(o[1], o[2], o[3])
            return
        if k == 'store':
            self.store(o[1], o[2], o[3])
            return
        if k == 'dread':
            self.dread(o[1], o[2], o[3], o[4])
            return
        if k == 'dwrite':
            self.dwrite(o[1], o[2], o[3])
            return
        if k == 'global_load':
            t = self.target(o[1])
            self.w(f'{t} = DM ({o[2]}.)')
            self.externs.add(o[2] + '.')
            self.commit(o[1], t)
            return
        if k == 'global_store':
            r = self.reg(o[2], 'R1')
            self.w(f'DM ({o[1]}.) = {r}')
            self.externs.add(o[1] + '.')
            return
        if k in ('field_load', 'field_store'):
            field = o[2] if k == 'field_load' else o[1]
            off = self.layout[field + '_byte'] // 4
            if k == 'field_load':
                t = self.target(o[1])
                self.w(f'{t} = DM ({off},I0)')
                self.commit(o[1], t)
            else:
                r = self.reg(o[2], 'R2')
                self.w(f'DM ({off},I0) = {r}')
            return
        raise ValueError(o)


REG_OPERAND_OPS = {'add', 'sub', 'or', 'xor', 'and', 'min', 'max', 'lt', 'ltu', 'eq', 'satlo', 'mulhi', 'mullo', 'mulpair',
                   'mac24', 'mac24p', 'acc', 'mra'}


def pair_multiplies(ops):
    """mulhi(a,b) and mullo(a,b) in one block -> one 'mulpair' (shared SSI)."""
    out = list(ops)
    for i, o in enumerate(out):
        if o is None or o[0] != 'mulhi':
            continue
        for j in range(i + 1, min(i + 4, len(out))):
            q = out[j]
            if q is None or q[0] in ('label', 'jmp', 'jz', 'jnz', 'ret'):
                break
            if q[0] == 'mullo' and set(q[2:]) == set(o[2:]) and o[1] not in q[2:]:
                between = [x for x in out[i + 1:j] if x is not None]
                if any(dest(x) in o[2:] + (q[1],) for x in between) or any(q[1] in sources(x) for x in between):
                    break
                out[i] = ('mulpair', o[1], q[1], o[2], o[3])
                out[j] = None
                break
    return [o for o in out if o is not None]


def hoist_constants(ops):
    """Hot constants used as register operands become entry-defined variables."""
    weight = loop_weights(ops)
    score = defaultdict(int)

    def mask_like(c):
        return c != 0 and ((c + 1) & c) == 0
    for i, o in enumerate(ops):
        k = o[0]
        if k in REG_OPERAND_OPS:
            for x in o[2:] if k not in MULTI else o[4:] if k in ('acc', 'mra') else o[3:]:
                if isinstance(x, int) and not (k == 'and' and mask_like(x & M32)) and not (k == 'sub' and x == 0 and x is o[2]) \
                        and not (k in ('acc', 'mra') and x == 0) and not (k in ('add', 'sub') and x & M32 in (1, M32)):
                    score[x & M32] += weight[i]
        elif k == 'sar':
            score[(-o[3]) & M32] += weight[i]
        elif k == 'store' and isinstance(o[3], int):
            score[o[3] & M32] += weight[i]
    hot = {c for c, w in score.items() if w >= 16}
    if not hot:
        return ops
    name = {c: f'#c{c:x}' for c in hot}
    out = []
    for i, o in enumerate(ops):
        k = o[0]
        if k in REG_OPERAND_OPS:
            head = 4 if k in ('acc', 'mra') else 3 if k in MULTI else 2
            args = []
            for n, x in enumerate(o[head:]):
                if isinstance(x, int) and (x & M32) in hot and not (k == 'and' and mask_like(x & M32)) \
                        and not (k in ('acc', 'mra') and x == 0) and not (k == 'sub' and n == 0 and x == 0) \
                        and not (k in ('add', 'sub') and x & M32 in (1, M32)):
                    args.append(name[x & M32])
                else:
                    args.append(x)
            o = o[:head] + tuple(args)
        elif k == 'sar' and ((-o[3]) & M32) in hot:
            o = ('sarv', o[1], o[2], name[(-o[3]) & M32])
        elif k == 'store' and isinstance(o[3], int) and (o[3] & M32) in hot:
            o = (k, o[1], o[2], name[o[3] & M32])
        out.append(o)
    return [('mov', name[c], c) for c in sorted(hot)] + out


def localize_constants(ops, keep_shared=frozenset()):
    """Split hoisted constants (#c...) at loop boundaries: inside each
    innermost loop region that uses one, the uses read a loop-local copy
    defined just before the loop. The short live range can stay in a register
    through the hot loop even when the function-wide constant is spilled.
    Only regions entered solely by falling into the header (every jump to a
    label inside comes from inside) are split; a constant's value never
    changes, so the copy is exact."""
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}

    def target(o):
        return o[1] if o[0] == 'jmp' else o[2] if o[0] in ('jz', 'jnz') else None
    regions = []
    for j, o in enumerate(ops):
        t = target(o)
        if t is not None and pos.get(t, j + 1) <= j:
            regions.append((pos[t], j))
    regions = [r for r in regions if not any(q != r and r[0] <= q[0] and q[1] <= r[1] for q in regions)]
    jumps = [(k, pos[target(o)]) for k, o in enumerate(ops) if target(o) is not None and target(o) in pos]
    inserts = defaultdict(list)
    rename = {}
    for n, (i, j) in enumerate(regions):
        if any(not i <= k <= j and i <= t <= j for k, t in jumps):
            continue
        used = sorted({x for k in range(i, j + 1) for x in sources(ops[k]) if isinstance(x, str) and x.startswith('#c')})
        if not used:
            continue
        at = i - 1 if i and ops[i - 1][0] == 'hwloop' else i
        for c in used:
            local = f'{c}@{n}'
            if local in keep_shared:
                continue
            inserts[at].append(('mov', local, int(c[2:], 16)))
            for k in range(i, j + 1):
                rename.setdefault(k, {})[c] = local
    out = []
    for k, o in enumerate(ops):
        out += inserts.get(k, [])
        if k in rename:
            r = rename[k]
            o = replace_sources(o, lambda x: r.get(x, x) if isinstance(x, str) else x)
        out.append(o)
    used = {x for o in out for x in sources(o) if isinstance(x, str)}
    return [o for o in out if not (o[0] == 'mov' and isinstance(o[1], str) and o[1].startswith('#c')
                                   and '@' not in o[1] and o[1] not in used)]


def hw_loops(ops):
    """Innermost counted loops with straight-line bodies run as SHARC hardware
    loops: 'mov c, N' becomes ('hwloop', N), the latch 'sub c; jnz c, L'
    becomes ('jnz', 1, L) -- still a back edge for liveness, emitted as the
    loop end. Bodies with out-of-line paths (generic memory sites, calls)
    or other jumps keep the software counter."""
    from tools.machines.translation.fast_opt import find_loops
    ranges, succ, pred = blocks(ops)
    loops = find_loops(ops, ranges, pred)
    _, live_in, _ = liveness(ops)
    out = list(ops)
    for h, (latch, c, body) in loops.items():
        if any(g != h and g in body for g in loops):
            continue
        s0, e0 = ranges[h][0], ranges[latch][1]
        if s0 == 0 or out[s0 - 1] != ('mov', c, out[s0 - 1][2]) or not isinstance(out[s0 - 1][2], int):
            continue
        n = out[s0 - 1][2] & M32
        if not 1 <= n <= 0xffff:
            continue
        if pred[h] != sorted({h - 1, latch}) and set(pred[h]) != {h - 1, latch}:
            continue
        # the counter is read only by the latch sub and jnz, and is dead
        # after the loop (unrolled copies of one DSP loop share its name)
        if sum(1 for i in range(s0, e0) for x in sources(out[i]) if x == c) != 2 \
                or (latch + 1 < len(ranges) and c in live_in[latch + 1]):
            continue
        ok = True
        for i in range(s0, e0 - 2):
            o = out[i]
            if o[0] in ('dread', 'dwrite', 'guard_vv', 'guard_y', 'jmp', 'jz', 'jnz', 'ret'):
                ok = False
                break
            if o[0] == 'label' and i != s0:
                ok = False
                break
        if not ok:
            continue
        out[s0 - 1] = ('hwloop', n)
        out[e0 - 2] = None
        out[e0 - 1] = ('jnz', 1, out[e0 - 1][2])
    return [o for o in out if o is not None]


import re as _re
_FRAME = _re.compile(r'DM \((-\d+),I6\)')


def _split(line):
    if '\x00' in line:
        a, b = line.split('\x00')
        return a, b
    return line, None


def line_weights(lines):
    """Rough execution weight per line: 16x per enclosing loop (back jumps, DO)."""
    pos = {}
    for k, l in enumerate(lines):
        s = _split(l)[0].strip()
        if s.endswith(':'):
            pos[s[:-1]] = k
    w = [1] * len(lines)
    for k, l in enumerate(lines):
        s = _split(l)[0]
        m = _re.search(r'JUMP (\.\S+?);', s)
        if m and pos.get(m.group(1), k + 1) <= k:
            for q in range(pos[m.group(1)], k + 1):
                w[q] *= 16
        m = _re.search(r'DO \(PC,(\S+?)\)UNTIL', s)
        if m and m.group(1) in pos:
            for q in range(k, pos[m.group(1)] + 2):
                w[q] *= 16
    return w


# (Sn, PEy's half of Rn in SIMD pair code, counts as a write of Rn)
_DEF = _re.compile(r'^(?:IF (?:NOT )?\w+ )?[RS](\d+) ?=(?!=)')


def forward_spills(lines):
    """Drop a frame reload `Rk = DM (s,I6)` when Rk still holds slot s (it was
    loaded from or stored to s, and neither Rk nor s changed since) in the
    same straight-line run. Labels, hardware-loop starts and calls forget
    everything; frame slots are written only by `DM (s,I6) = Rk`."""
    out = []
    held = {}                       # register -> frame slot it holds
    for line in lines:
        s = _split(line)[0].strip()
        if s.endswith(':') or not s.endswith(';'):
            held.clear()
            out.append(line)
            continue
        s = s[:-1]
        if 'CJUMP' in s or 'CALL' in s or 'DO (' in s or 'RTS' in s or 'RFRAME' in s:
            held.clear()
            out.append(line)
            continue
        m = _re.match(r'^(R\d+) = DM \((-\d+),I6\)$', s)
        if m:
            r, slot = m.group(1), int(m.group(2))
            if held.get(r) == slot:
                continue
            held[r] = slot
            out.append(line)
            continue
        m = _re.match(r'^DM \((-\d+),I6\) = (R\d+)$', s)
        if m:
            slot, r = int(m.group(1)), m.group(2)
            for q in [q for q, v in held.items() if v == slot]:
                del held[q]
            held[r] = slot
            out.append(line)
            continue
        if 'I6' in s and 'DM' in s:
            held.clear()
        for part in s.split(','):
            m = _DEF.match(part.strip())
            if m:
                held.pop('R' + m.group(1), None)
        out.append(line)
    return pad_hw_loops(out)


def pad_hw_loops(lines):
    """Keep every hardware loop body at least three instructions long (as the
    emitter does; removing frame reloads can shorten one). The last
    instruction follows the end label and is never removed, so a body that
    did not end in a jump still does not."""
    for k in range(len(lines)):
        m = _re.search(r'DO \(PC,(\S+?)\)UNTIL', _split(lines[k])[0])
        if not m:
            continue
        end = next(j for j in range(k + 1, len(lines)) if _split(lines[j])[0].strip() == m.group(1) + ':')
        body = [j for j in range(k + 1, end + 2) if not _split(lines[j])[0].strip().endswith(':')]
        need = max(0, 3 - len(body))
        if need:
            lines[end:end] = ['    NOP;'] * need
            return pad_hw_loops(lines)
    return lines


def remap_frame(pre, body, post, top):
    """Renumber frame slots so the hottest sit nearest I6 (within the 6-bit
    immediate reach of multifunction instructions)."""
    w = line_weights(body)
    score = defaultdict(int)
    for k, l in enumerate(body):
        for m in _FRAME.finditer(_split(l)[0]):
            score[-int(m.group(1))] += w[k]
    slots = list(range(3, top))
    order = sorted(slots, key=lambda s: (-score.get(s, 0), s))
    new = {old: slot for old, slot in zip(order, slots)}

    def fix(lines):
        return [_FRAME.sub(lambda m: f'DM ({-new.get(-int(m.group(1)), -int(m.group(1)))},I6)', l) for l in lines]
    return fix(pre), fix(body), fix(post)


_COMPUTE = _re.compile(r'^(?:(R\d+) = (?:PASS (R\d+)|(R\d+) [+-] (R\d+)|(R\d+) (?:AND|OR|XOR) (R\d+)|-(R\d+)|'
                       r'(?:MIN|MAX)\((R\d+),(R\d+)\))|COMPU? \((R\d+),(R\d+)\))$')
_LOAD = _re.compile(r'^(R\d+) = DM \((-?\d+),I6\)$|^(R\d+) = DM \(I4,M5\)$')
_STORE = _re.compile(r'^DM \((-?\d+),I6\) = (R\d+)$|^DM \(I4,M5\) = (R\d+)$')


def _mem_text(m, load):
    if load:
        if m.group(1):
            off = int(m.group(2))
            return f'{m.group(1)}=DM ({"-" if off < 0 else ""}0x{abs(off):X},I6)', m.group(1), off
        return f'{m.group(3)}=DM (I4,M5)', m.group(3), 0
    if m.group(1):
        off = int(m.group(1))
        return f'DM ({"-" if off < 0 else ""}0x{abs(off):X},I6)={m.group(2)}', m.group(2), off
    return f'DM (I4,M5)={m.group(3)}', m.group(3), 0


def pack(lines):
    """Fold a frame/DAG load into the preceding single ALU compute, and a store
    into the following one (SHARC multifunction 'compute , memory move'), the
    forms production selcc output uses. Lines right after a label keep their
    own instruction slot."""
    out = []
    k = 0
    n = len(lines)

    def instr(l):
        s = _split(l)[0].strip()
        return s[:-1] if s.endswith(';') else None
    m4 = _re.compile(r'^M4 = (R\d+)$')
    lm = _re.compile(r'^(R\d+) = DM \(M4,(I\d)\)$')
    sm = _re.compile(r'^DM \(M4,(I\d)\) = (R\d+)$')
    while k < n:
        line = lines[k]
        s = instr(line)
        # compute; M4 = Rx; access  ->  M4 = Rx; compute , access
        # never fold into an instruction that directly follows a label: it
        # may be a hardware loop's last instruction
        after_label = len(out) >= 2 and instr(out[-2]) is None or len(out) == 1
        if s is not None and out and not after_label and k + 1 < n and instr(out[-1]) is not None \
                and instr(lines[k + 1]) is not None:
            mm = m4.match(s)
            prev = instr(out[-1])
            mc = _COMPUTE.match(prev) if prev and ' , ' not in prev else None
            acc_line = instr(lines[k + 1])
            ml, ms = lm.match(acc_line), sm.match(acc_line)
            if mm and mc and (ml or ms) and mc.group(1) != mm.group(1):
                dest = mc.group(1)
                if ml and ml.group(1) != dest:
                    mem = f'{ml.group(1)}=DM (M4,{ml.group(2)})'
                elif ms and ms.group(2) != dest:
                    mem = f'DM (M4,{ms.group(1)})={ms.group(2)}'
                else:
                    mem = None
                if mem:
                    tag = _split(out[-1])[1]
                    out[-1:] = [line, f'    {prev} , {mem};' + (f'\x00{tag}' if tag else '')]
                    k += 2
                    continue
        # store followed by a compute: merge into the compute
        if s is not None and k + 1 < n and (k == 0 or instr(lines[k - 1]) is not None):
            ms = _STORE.match(s)
            nxt = instr(lines[k + 1])
            if ms and nxt is not None and _COMPUTE.match(nxt):
                mem, reg, off = _mem_text(ms, False)
                if -32 <= off <= 31:
                    tag = _split(lines[k + 1])[1]
                    out.append(f'    {nxt} , {mem};' + (f'\x00{tag}' if tag else ''))
                    k += 2
                    continue
        # load following a compute (not right after a label): merge back
        if s is not None and out and not after_label and k > 0 and instr(lines[k - 1]) is not None \
                and instr(out[-1]) is not None:
            ml = _LOAD.match(s)
            prev = instr(out[-1])
            mc = _COMPUTE.match(prev) if prev and ' , ' not in prev else None
            if ml and mc:
                mem, reg, off = _mem_text(ml, True)
                dest = mc.group(1)
                if -32 <= off <= 31 and reg != dest:
                    tag = _split(out[-1])[1]
                    out[-1] = f'    {prev} , {mem};' + (f'\x00{tag}' if tag else '')
                    k += 1
                    continue
        out.append(line)
        k += 1
    return out


def mr_fuse(ops):
    """Products feeding only one 'acc' accumulate in the multiplier register
    MRF instead (MRF = (h:l) << 8, products added with (SSF) on a << 8):
    ('mra', dh, dl, spec, *args), spec[k] = (kind, sign) with kind 'p'
    (a, b: a sign-extended 24-bit, shifted at use), 'k' (a8 constant, b) or
    'g' (generic h, l pair)."""
    from tools.machines.translation.fast_opt import _block_defuse, dests
    ranges, live_in, live_out = liveness(ops)
    out = list(ops)
    for bi, (s, e) in enumerate(ranges):
        last, uses = _block_defuse(out, s, e, live_out[bi])
        for i in range(s, e):
            o = out[i]
            if o is None or o[0] != 'acc':
                continue
            spec, args, drop = [], [], []
            for sg, h, l in acc_terms(o):
                j = None
                if isinstance(h, str) and isinstance(l, str):
                    for q in range(i - 1, s - 1, -1):
                        if out[q] is not None and h in dests(out[q]):
                            j = q
                            break
                if j is not None and out[j][0] in ('mac24', 'mac24p') and out[j][1] == h and out[j][2] == l \
                        and uses.get(j) == [i, i] and not any(
                            out[q] is not None and set(dests(out[q])) & {x for x in out[j][3:] if isinstance(x, str)}
                            for q in range(j + 1, i)):
                    spec.append(('p' if out[j][0] == 'mac24' else 'k', sg))
                    args += [out[j][3], out[j][4]]
                    drop.append(j)
                else:
                    spec.append(('g', sg))
                    args += [h, l]
            if not drop:
                continue
            for q in drop:
                out[q] = None
            out[i] = ('mra', o[1], o[2], tuple(spec)) + tuple(args)
    return [o for o in out if o is not None]


def preshift(ops):
    """Multiply operands shifted at every use ('p' terms of 'mra', 'mac24')
    get a left-aligned copy x8 = x << 8 computed once after each definition
    of x, used as the pre-shifted operand ('k' terms, 'mac24p') -- when the
    shifts saved (uses, weighted by loop depth) outnumber the ones added
    (definitions). The value is the same LSHIFT the use would have done."""
    w = loop_weights(ops)
    defs, uses = defaultdict(list), defaultdict(list)
    for i, o in enumerate(ops):
        for x in dests(o):
            defs[x].append(i)
    for i, o in enumerate(ops):
        if o[0] == 'mra':
            for n, (kind, sg) in enumerate(o[3]):
                if kind == 'p' and isinstance(o[4 + 2 * n], str):
                    uses[o[4 + 2 * n]].append(i)
        elif o[0] == 'mac24' and isinstance(o[3], str):
            uses[o[3]].append(i)
    names = {x for o in ops for x in dests(o) + [s for s in sources(o) if isinstance(s, str)]}
    other = defaultdict(int)
    for o in ops:
        for s in sources(o):
            if isinstance(s, str):
                other[s] += 1
    chosen = {}
    mode = _os.environ.get('FAST_PRESHIFT', 'only')
    for x, us in uses.items():
        ds = defs.get(x)
        if not ds:
            continue
        # only-multiply variables: x8 replaces x, no extra register
        if mode == 'only' and other[x] != len(us):
            continue
        if sum(w[i] for i in us) <= sum(w[i] for i in ds):
            continue
        n = x + '<<8'
        while n in names:
            n += "'"
        chosen[x] = n
    if not chosen:
        return ops
    out = []
    for i, o in enumerate(ops):
        if o[0] == 'mra':
            spec, args = list(o[3]), list(o[4:])
            for n, (kind, sg) in enumerate(spec):
                if kind == 'p' and args[2 * n] in chosen:
                    spec[n] = ('k', sg)
                    args[2 * n] = chosen[args[2 * n]]
            o = o[:3] + (tuple(spec),) + tuple(args)
        elif o[0] == 'mac24' and o[3] in chosen:
            o = ('mac24p',) + o[1:3] + (chosen[o[3]],) + o[4:]
        out.append(o)
        for x in dests(o):
            if x in chosen:
                out.append(('shl', chosen[x], x, 8))
    return out


import os as _os
DEBUG_EXPAND = _os.environ.get('FAST_EXPAND', '')


def expand_fused(ops, kinds):
    """Debug aid: rewrite fused accumulator ops back into primitive IR."""
    out = []
    n = [0]

    def t():
        n[0] += 1
        return f'z{n[0]}'
    for o in ops:
        if o[0] in ('mac24', 'mac24p') and 'mac' in kinds:
            H, L, a, b = o[1:]
            if o[0] == 'mac24p':
                a = s32(a) >> 8 & M32
            lo, sh = t(), t()
            out += [('mulhi', H, a, b), ('mullo', lo, a, b), ('shl', sh, lo, 1), ('and', L, sh, 0xffffff)]
        elif o[0] == 'acc' and 'acc' in kinds:
            sl, sh = 0, 0
            for g, h, l in acc_terms(o):
                v, w = t(), t()
                out += [('add' if g > 0 else 'sub', v, sl, l), ('add' if g > 0 else 'sub', w, sh, h)]
                sl, sh = v, w
            c = t()
            out += [('sar', c, sl, 24), ('add', o[1], sh, c), ('and', o[2], sl, 0xffffff)]
        else:
            out.append(o)
    return out


_MUL = _re.compile(r'^(?:(R\d+) = (R\d+) \* (R\d+) \((?:SSI|SSF)\)|MRF = (?:MRF [+-] )?(R\d+) \* (R\d+) \(SSF\))$')
_INC = _re.compile(r'^(R\d+) = (R\d+) [+-] 1$')
_FRAME_LOAD = _re.compile(r'^(R\d+) = DM \((-?\d+),I6\)$')
_FRAME_STORE = _re.compile(r'^DM \((-?\d+),I6\) = (R\d+)$')
# Sn is PEy's half of Rn (SIMD pair code): tracked as Rn
_RTOK = _re.compile(r'\b[RS](\d+)\b')


def _parts(s):
    """Top-level ' , ' parts of an instruction (commas inside parentheses kept)."""
    out, depth, cur = [], 0, ''
    for ch in s:
        depth += ch == '('
        depth -= ch == ')'
        if ch == ',' and depth == 0:
            out.append(cur.strip())
            cur = ''
        else:
            cur += ch
    out.append(cur.strip())
    return out


def _effects(s):
    """(registers written, registers read, frame slots written, frame slots
    read) of one instruction text; None for control flow or unknown forms."""
    if any(w in s for w in ('JUMP', 'CALL', 'RTS', 'DO (', 'RFRAME', 'CJUMP')):
        return None
    wr, rd, sw, sr = set(), set(), set(), set()
    for part in _parts(s):
        cond = part.startswith('IF ')
        if cond:
            part = part.split(' ', 3)[3] if part.startswith('IF NOT ') else part.split(' ', 2)[2]
        if '=' not in part:
            rd |= {f'R{n}' for n in _RTOK.findall(part)}
            continue
        lhs, rhs = (x.strip() for x in part.split('=', 1))
        rd |= {f'R{n}' for n in _RTOK.findall(rhs)}
        m = _re.match(r'^DM \((-?(?:0x)?[0-9A-Fa-f]+),I6\)$', lhs)
        if m:
            sw.add(int(m.group(1), 0))
            rd |= {f'R{n}' for n in _RTOK.findall(lhs)}
            continue
        if lhs.startswith(('DM', 'PM')):
            rd |= {f'R{n}' for n in _RTOK.findall(lhs)}
            continue
        m = _re.match(r'^DM \((-?(?:0x)?[0-9A-Fa-f]+),I6\)$', rhs)
        if m:
            sr.add(int(m.group(1), 0))
        if _re.match(r'^[RS]\d+$', lhs):
            wr.add('R' + lhs[1:])
            if cond:
                rd.add('R' + lhs[1:])
        elif lhs in ('I6', 'I7'):
            return None
    return wr, rd, sw, sr


def _absorbs(s):
    """A single unconditional compute that can carry a memory move (SHARC
    multifunction: ALU or multiplier compute , DM(n,I6) access)."""
    if ' , ' in s or s.startswith('IF '):
        return False
    return bool(_COMPUTE.match(s) or _MUL.match(s) or _INC.match(s))


def pack_far(lines, reach=12):
    """Fold the remaining single frame loads into an earlier compute and
    single frame stores into a later one in the same straight-line run
    (never across labels or control flow, never into an instruction that
    follows a label: it may end a hardware loop), keeping every read and
    write of the moved register and frame slot in order."""
    lines = list(lines)

    def instr(k):
        s = _split(lines[k])[0].strip()
        return s[:-1] if s.endswith(';') and not s.endswith(':') else None

    def after_label(j):
        return j == 0 or instr(j - 1) is None
    # FAST_PF_LIMIT=n stops after n folds per function (for bisecting)
    budget = [int(_os.environ.get('FAST_PF_LIMIT', '-1'))]

    def allow():
        if budget[0] == 0:
            return False
        budget[0] -= 1
        return True
    k = 0
    while k < len(lines):
        s = instr(k)
        ml = _FRAME_LOAD.match(s) if s else None
        ms = _FRAME_STORE.match(s) if s else None
        if ml and -32 <= int(ml.group(2)) <= 31 and not after_label(k):
            reg, slot = ml.group(1), int(ml.group(2))
            for j in range(k - 1, max(-1, k - 1 - reach), -1):
                t = instr(j)
                if t is None:
                    break
                eff = _effects(t)
                if eff is None:
                    break
                wr, rd, sw, sr = eff
                if slot in sw:
                    break
                if _absorbs(t) and reg not in wr and not after_label(j) and allow():
                    mem, _, _ = _mem_text(ml, True)
                    tag = _split(lines[j])[1]
                    lines[j] = f'    {t} , {mem};' + (f'\x00{tag}' if tag else '')
                    del lines[k]
                    k -= 1
                    break
                if reg in wr or reg in rd:
                    break
        elif ms and -32 <= int(ms.group(1)) <= 31 and not after_label(k):
            # (an instruction right after a label may end a hardware loop:
            # it stays where it is)
            slot, reg = int(ms.group(1)), ms.group(2)
            for j in range(k + 1, min(len(lines), k + 1 + reach)):
                t = instr(j)
                if t is None:
                    break
                eff = _effects(t)
                if eff is None:
                    break
                wr, rd, sw, sr = eff
                if slot in sw or slot in sr:
                    break
                if _absorbs(t) and not after_label(j) and allow():
                    mem, _, _ = _mem_text(ms, False)
                    tag = _split(lines[j])[1]
                    lines[j] = f'    {t} , {mem};' + (f'\x00{tag}' if tag else '')
                    del lines[k]
                    k -= 1
                    break
                if reg in wr:
                    break
        k += 1
    return pad_hw_loops(lines)


_XFER = _re.compile(r'^([MI]4|M0) = ([RS]\d+)$')


def pack_moves(lines, reach=8):
    """Fold a lone `M4 = Rx` / `I4 = Rx` into a nearby single compute as the
    SHARC Type 5a 'compute , ureg = ureg' form (selas encodes it bit-identical
    to the stock firmware's own instructions, e.g. 707e12200224). Moving it
    earlier needs Rx unwritten from that compute on and the DAG register
    untouched in between; moving it later needs nothing in between to use the
    DAG register or rewrite Rx. Same run/label rules as pack_far."""
    lines = list(lines)

    def instr(k):
        s = _split(lines[k])[0].strip()
        return s[:-1] if s.endswith(';') and not s.endswith(':') else None

    def after_label(j):
        return j == 0 or instr(j - 1) is None

    def touches(t, dag):
        return _re.search(r'\b' + dag + r'\b', t) is not None
    k = 0
    while k < len(lines):
        s = instr(k)
        m = _XFER.match(s) if s else None
        if not m or after_label(k):
            k += 1
            continue
        dag, reg = m.group(1), m.group(2)
        rr = 'R' + reg[1:]          # Sx is written with Rx (SIMD)
        done = False
        # earlier compute
        for j in range(k - 1, max(-1, k - 1 - reach), -1):
            t = instr(j)
            if t is None:
                break
            eff = _effects(t)
            if eff is None or touches(t, dag) or rr in eff[0]:
                break
            if _absorbs(t) and not after_label(j):
                tag = _split(lines[j])[1]
                lines[j] = f'    {t} , {dag} = {reg};' + (f'\x00{tag}' if tag else '')
                del lines[k]
                done = True
                break
        if not done:
            for j in range(k + 1, min(len(lines), k + 1 + reach)):
                t = instr(j)
                if t is None:
                    break
                eff = _effects(t)
                if eff is None or touches(t, dag):
                    break
                if _absorbs(t) and not after_label(j):
                    tag = _split(lines[j])[1]
                    lines[j] = f'    {t} , {dag} = {reg};' + (f'\x00{tag}' if tag else '')
                    del lines[k]
                    done = True
                    break
                if rr in eff[0]:
                    break
        if not done:
            k += 1
    return pad_hw_loops(lines)


def compile_function(name, machine, ops, layout, source_base, fallback=None, generic=None, extern=True, hints=None):
    if DEBUG_EXPAND:
        ops = expand_fused(ops, DEBUG_EXPAND.split(','))
    ops = lower_generic(ops, machine)
    ops = pair_multiplies(ops)
    if not _os.environ.get('FAST_NO_HWLOOP'):
        ops = hw_loops(ops)
    if not _os.environ.get('FAST_NO_MR'):
        ops = mr_fuse(ops)
    if not _os.environ.get('FAST_NO_PRESHIFT'):
        ops = preshift(ops)
    ops = hoist_constants(ops)
    if not _os.environ.get('FAST_NO_LOCALIZE'):
        # a split copy that would not get a register only adds a frame slot:
        # those constants keep the shared function-wide variable
        split = localize_constants(ops)
        color = allocate(split)
        spilled = {v for o in split for v in [dest(o)] if isinstance(v, str) and '@' in v and v not in color}
        ops = localize_constants(ops, frozenset(spilled)) if spilled else split
    e = Emitter(name, machine, layout, ops, source_base)
    e.fallback = fallback
    e.generic = generic
    e.hints = hints
    body = e.emit()
    f = e.frame
    pre, post = [], []

    def w(lines, text):
        lines.append('    ' + text + ';')
    for k in range(8):
        w(pre, f'DM ({-(3 + k)},I6) = R{8 + k}')
    # I0..I3 are only used with pre-modify (no update) addressing, so the
    # circular-buffer L registers never apply; they are left untouched.
    for k in range(4):
        w(pre, f'R0 = I{k}')
        w(pre, f'DM ({f.slot(f"s:I{k}")},I6) = R0')
    w(pre, 'R0 = M4')
    w(pre, f'DM ({f.slot("s:M4")},I6) = R0')
    if e.uses_mr:
        # the caller's multiplier result register is preserved
        for part in ('MR0F', 'MR1F', 'MR2F'):
            w(pre, f'R0 = {part}')
            w(pre, f'DM ({f.slot("s:" + part)},I6) = R0')
    w(pre, 'I0 = R4')
    w(pre, 'R0 = md_shared_scratch_x.')
    w(pre, 'I1 = R0')
    w(pre, 'R0 = md_shared_scratch_y.')
    w(pre, 'I2 = R0')
    w(pre, 'I3 = R8')
    w(pre, 'R0 = I5')
    w(pre, f'DM ({f.slot("s:I5")},I6) = R0')
    if e.i5_base is not None:
        off = {'BLK': layout['blk_byte'], 'XP': layout.get('x_private_byte'), 'EXT': layout.get('external_byte')}[e.i5_base]
        w(pre, f'R0 = 0x{off:X}')
        w(pre, 'R0 = R0 + R4')
        w(pre, 'I5 = R0')
    if machine.private_memory:
        w(pre, f'R1 = DM ({layout["virtual_voice_byte"] // 4},I0)')
        w(pre, 'R0 = LSHIFT R1 BY 6')
        w(pre, 'R2 = 0x800')
        w(pre, 'R0 = R0 + R2')
        w(pre, f'DM ({f.slot("w:xp")},I6) = R0')
        w(pre, f'DM ({f.slot("w:yw")},I6) = R0')
        w(pre, 'R2 = 0x600')
        w(pre, 'R0 = R1 * R2 (SSI)')
        w(pre, 'R2 = 0x135600')
        w(pre, 'R0 = R0 + R2')
        w(pre, f'DM ({f.slot("w:ext")},I6) = R0')
    for k in range(4):
        w(post, f'R0 = DM ({f.slot(f"s:I{k}")},I6)')
        w(post, f'I{k} = R0')
    w(post, f'R0 = DM ({f.slot("s:M4")},I6)')
    w(post, 'M4 = R0')
    if e.uses_mr:
        # MR1F first: writing it sign-extends into MR2
        for part in ('MR1F', 'MR2F', 'MR0F'):
            w(post, f'R0 = DM ({f.slot("s:" + part)},I6)')
            w(post, f'{part} = R0')
    w(post, f'R0 = DM ({f.slot("s:I5")},I6)')
    w(post, 'I5 = R0')
    for k in range(8):
        w(post, f'R{8 + k} = DM ({-(3 + k)},I6)')
    size = f.next + 2
    if not _os.environ.get('FAST_NO_FORWARD'):
        body = forward_spills(body)
    pre, body, post = remap_frame(pre, body, post, f.next)
    if not _os.environ.get('FAST_NO_PACK'):
        body = pack(body)
        if not _os.environ.get('FAST_NO_PACKFAR'):
            body = pack_far(body)
        if not _os.environ.get('FAST_NO_PACKMOVE'):
            body = pack_moves(body)
    text = ['.EXTERN md_shared_scratch_x.;', '.EXTERN md_shared_scratch_y.;']
    for sym in sorted(e.externs):
        text.append(f'.EXTERN {sym};')
    text += ['.SECTION/SW seg_swco;', f'.GLOBAL {name}.;', f'{name}.:']
    text.append(f'    I7=MODIFY (I7,-0x{size:X})(NW);')
    text += pre
    tags = []
    for k, line in enumerate(body):
        if '\x00' in line:
            line, tag = line.split('\x00')
            body[k] = line
            tags.append((len(text) + k, tag))
    e.tags = dict(tags)
    text += body
    text.append(f'{e.L("__exit")}:')
    text += post
    text += ['    I12=DM (M7,I6);', '    JUMP (M14,I12) (DB);', '    RFRAME;', '    NOP;', '.ENDSEG;', '']
    return '\n'.join(text), e
