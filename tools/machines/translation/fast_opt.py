"""Optimizer for the fast IR. Every rewrite preserves the IR's 32-bit semantics.

Passes: CFG construction, interval/constant propagation (with widening),
constant folding, AGU specialization for constant modifiers, static memory
region resolution of generic reads/writes, algebraic simplification, local
copy propagation and global dead-code elimination.
"""
from collections import defaultdict
M32 = 0xffffffff
PURE = {'mov', 'add', 'sub', 'and', 'or', 'xor', 'shl', 'shr', 'sar', 'min', 'max', 'lt', 'ltu', 'eq',
        'mulhi', 'mullo', 'load', 'agu', 'global_load', 'field_load', 'satlo'}
CONTROL = {'label', 'jmp', 'jz', 'jnz', 'ret'}
TOP = (0, M32)


def s32(x):
    x &= M32
    return x - (1 << 32) if x & 0x80000000 else x


def fold(k, a, b=None, *rest):
    if k == 'mov':
        return a & M32
    if k == 'add':
        return (a + b) & M32
    if k == 'sub':
        return (a - b) & M32
    if k == 'and':
        return a & b & M32
    if k == 'or':
        return (a | b) & M32
    if k == 'xor':
        return (a ^ b) & M32
    if k == 'shl':
        return (a << b) & M32
    if k == 'shr':
        return (a & M32) >> b
    if k == 'sar':
        return (s32(a) >> b) & M32
    if k == 'min':
        return min(s32(a), s32(b)) & M32
    if k == 'max':
        return max(s32(a), s32(b)) & M32
    if k == 'lt':
        return int(s32(a) < s32(b))
    if k == 'ltu':
        return int((a & M32) < (b & M32))
    if k == 'eq':
        return int((a & M32) == (b & M32))
    if k == 'mulhi':
        return ((s32(a) * s32(b)) >> 23) & M32
    if k == 'satlo':
        return satlo(a, b, *rest)
    if k == 'mullo':
        return (a * b) & M32
    raise KeyError(k)


def agu_fold(old, m, step):
    old &= M32
    nxt = (old + step) & 0xffffff
    if m != 0xffffff:
        mask = 1
        if m > 1:
            mask = m
            for s in (1, 2, 4, 8, 16):
                mask |= mask >> s
        base = old & ~mask & M32
        offset = s32((old & mask) + s32(step))
        if offset < 0:
            offset = offset + s32(m) + 1
        elif (offset & M32) > m:
            offset = offset - s32(m) - 1
        nxt = (base + offset) & M32
    return nxt


def fill(m):
    mask = 1
    if m > 1:
        mask = m
        for s in (1, 2, 4, 8, 16):
            mask |= mask >> s
    return mask


# ops with two results (o[1], o[2]); sources start at o[3]
MULTI = {'mulpair', 'mac24', 'mac24p', 'acc'}


def satlo(h, l, c1, c2, k):
    """('satlo', d, h, l, c1, c2, k): k if c1 < h, else 0 if h < c2, else l
    (signed compares) -- a limited accumulator move's low word."""
    if s32(c1) < s32(h):
        return k & M32
    if s32(h) < s32(c2):
        return 0
    return l & M32


def dest(o):
    if o[0] in PURE or o[0] in ('dread', 'sarv') or o[0] in MULTI:
        return o[1]
    return None


def dests(o):
    if o[0] in MULTI:
        return [o[1], o[2]]
    d = dest(o)
    return [] if d is None else [d]


def sources(o):
    k = o[0]
    if k in ('label', 'jmp', 'ret', 'global_load', 'field_load'):
        return []
    if k in ('jz', 'jnz'):
        return [o[1]]
    if k == 'load':
        return [o[3]]
    if k == 'store':
        return [o[2], o[3]]
    if k == 'dread':
        return [o[3]]
    if k == 'dwrite':
        return [o[2], o[3]]
    if k in ('global_store', 'field_store'):
        return [o[2]]
    if k == 'fault':
        return [o[1]]
    if k == 'acc':
        return list(o[4:])
    if k in MULTI:
        return list(o[3:])
    return list(o[2:])


def replace_sources(o, f):
    k = o[0]
    if k in ('label', 'jmp', 'ret', 'global_load', 'field_load'):
        return o
    if k in ('jz', 'jnz'):
        return (k, f(o[1]), o[2])
    if k == 'load':
        return (k, o[1], o[2], f(o[3]))
    if k == 'store':
        return (k, o[1], f(o[2]), f(o[3]))
    if k == 'dread':
        return (k, o[1], o[2], f(o[3]), o[4])
    if k == 'dwrite':
        return (k, o[1], f(o[2]), f(o[3]))
    if k in ('global_store', 'field_store'):
        return (k, o[1], f(o[2]))
    if k == 'fault':
        return (k, f(o[1]))
    if k in MULTI:
        return (k, o[1], o[2]) + tuple(f(x) for x in o[3:])
    return (k, o[1]) + tuple(f(x) for x in o[2:])


# ------------------------------------------------------------------ CFG
def blocks(ops):
    """Split into basic blocks: list of (start, end) index ranges."""
    starts = {0}
    for i, o in enumerate(ops):
        if o[0] == 'label':
            starts.add(i)
        if o[0] in ('jmp', 'jz', 'jnz', 'ret'):
            starts.add(i + 1)
    starts = sorted(s for s in starts if s < len(ops))
    ranges = [(s, starts[k + 1] if k + 1 < len(starts) else len(ops)) for k, s in enumerate(starts)]
    label_block = {}
    for bi, (s, e) in enumerate(ranges):
        for i in range(s, e):
            if ops[i][0] == 'label':
                label_block[ops[i][1]] = bi
            else:
                break
    succ = []
    for bi, (s, e) in enumerate(ranges):
        last = ops[e - 1]
        out = []
        if last[0] == 'jmp':
            out = [label_block[last[1]]]
        elif last[0] in ('jz', 'jnz'):
            out = [label_block[last[2]]]
            if bi + 1 < len(ranges):
                out.append(bi + 1)
        elif last[0] == 'ret':
            out = []
        elif bi + 1 < len(ranges):
            out = [bi + 1]
        succ.append(out)
    pred = [[] for _ in ranges]
    for bi, out in enumerate(succ):
        for t in out:
            pred[t].append(bi)
    return ranges, succ, pred


# ------------------------------------------------------------------ intervals
# An abstract value is (lo, hi, s): every concrete value is (s*VV + c) mod 2^32
# for some c in [lo, hi], where VV is the voice's virtual_voice field. s is
# non-zero only after a 'guard_vv' entry check proved 0 <= VV <= VVMAX; plain
# values have s == 0 and 0 <= lo <= hi <= M32.
VVMAX = 15


def norm(lo, hi, s=0):
    if s == 0:
        if lo > hi:
            return TOP3
        if lo >> 32 == hi >> 32:
            base = (lo >> 32) << 32
            return (lo - base, hi - base, 0)
        return TOP3
    clo, chi = lo + min(0, s * VVMAX), hi + max(0, s * VVMAX)
    if 0 <= clo and chi <= M32:
        return (lo, hi, s)
    return TOP3


TOP3 = (0, M32, 0)


def conc(v):
    lo, hi, s = v
    if s == 0:
        return (lo, hi)
    clo, chi = lo + min(0, s * VVMAX), hi + max(0, s * VVMAX)
    if 0 <= clo and chi <= M32:
        return (clo, chi)
    return TOP


def sval(env, x):
    if isinstance(x, int):
        x &= M32
        return (x, x, 0)
    return env.get(x, TOP3)


def ival(env, x):
    return conc(sval(env, x))


def known(lo, hi):
    """Known-one bits K and maybe-one bits M of every value in [lo, hi]."""
    if lo == hi:
        return lo, lo
    low = (1 << (lo ^ hi).bit_length()) - 1
    k = lo & ~low
    return k, k | low


def signed_range(a):
    """(lo, hi) as signed integers, or None if the interval straddles 2^31."""
    lo, hi = a
    if hi < 0x80000000:
        return lo, hi
    if lo >= 0x80000000:
        return lo - (1 << 32), hi - (1 << 32)
    return None


def from_signed(lo, hi):
    if lo >= 0:
        return norm(lo, hi)
    if hi < 0:
        return norm(lo + (1 << 32), hi + (1 << 32))
    return TOP3


def plain(k, args):
    """Transfer of a pure op on plain intervals (list of (lo, hi))."""
    if all(a[0] == a[1] for a in args):
        v = fold(k, *[a[0] for a in args])
        return (v, v, 0)
    if k == 'mov':
        return args[0] + (0,)
    if k == 'and':
        (ka, ma), (kb, mb) = known(*args[0]), known(*args[1])
        lo, hi = ka & kb, min(ma & mb, args[0][1], args[1][1])
        return norm(lo, max(lo, hi))
    if k == 'or':
        (ka, ma), (kb, mb) = known(*args[0]), known(*args[1])
        lo = max(ka | kb, args[0][0], args[1][0])
        hi = min(ma | mb, args[0][1] + args[1][1])
        return norm(lo, max(lo, hi))
    if k == 'xor':
        (ka, ma), (kb, mb) = known(*args[0]), known(*args[1])
        return norm(0, ma | mb)
    if k == 'add':
        return norm(args[0][0] + args[1][0], args[0][1] + args[1][1])
    if k == 'sub':
        return norm(args[0][0] - args[1][1], args[0][1] - args[1][0])
    if k in ('shl', 'shr', 'sar'):
        if args[1][0] != args[1][1]:
            return TOP3
        n = args[1][0]
        if k == 'shl':
            return norm(args[0][0] << n, args[0][1] << n) if (args[0][1] << n) <= M32 else TOP3
        if k == 'shr':
            return norm(args[0][0] >> n, args[0][1] >> n)
        r = signed_range(args[0])
        if r is None:
            return TOP3
        return from_signed(r[0] >> n, r[1] >> n)
    if k in ('min', 'max', 'lt'):
        a, b = signed_range(args[0]), signed_range(args[1])
        if a is None or b is None:
            return (0, 1, 0) if k == 'lt' else TOP3
        if k == 'lt':
            if a[1] < b[0]:
                return (1, 1, 0)
            if a[0] >= b[1]:
                return (0, 0, 0)
            return (0, 1, 0)
        f = min if k == 'min' else max
        return from_signed(f(a[0], b[0]), f(a[1], b[1]))
    if k == 'ltu':
        a, b = args
        if a[1] < b[0]:
            return (1, 1, 0)
        if a[0] >= b[1]:
            return (0, 0, 0)
        return (0, 1, 0)
    if k == 'eq':
        a, b = args
        if a[1] < b[0] or b[1] < a[0]:
            return (0, 0, 0)
        return (0, 1, 0)
    if k == 'mulhi':
        a, b = signed_range(args[0]), signed_range(args[1])
        if a is None or b is None:
            return TOP3
        p = [x * y for x in a for y in b]
        return from_signed(min(p) >> 23, max(p) >> 23)
    return TOP3


def symbolic(k, o, env):
    """Transfer keeping the virtual-voice term; None when it does not apply."""
    args = [sval(env, x) for x in o[2:]]
    if not any(a[2] for a in args):
        return None
    if k == 'mov':
        return args[0]
    if k in ('add', 'sub'):
        a, b = args
        # constants act as signed offsets
        def c(v):
            if v[2] == 0 and v[0] == v[1] and v[0] >= 0x80000000:
                return (v[0] - (1 << 32), v[1] - (1 << 32), 0)
            return v
        a, b = c(a), c(b)
        if k == 'add':
            return norm(a[0] + b[0], a[1] + b[1], a[2] + b[2])
        return norm(a[0] - b[1], a[1] - b[0], a[2] - b[2])
    if k == 'and':
        x, m = (args[0], args[1]) if args[0][2] else (args[1], args[0])
        if m[2] or m[0] != m[1] or x[0] < 0:
            return None
        mask = m[0]
        cx = conc(x)
        if cx[1] <= mask and ((mask + 1) & mask) == 0:
            return x
        low = ((mask + 1) & mask) == 0 and mask + 1
        if low and x[2] % low == 0:
            r = plain('and', [(x[0], x[1]), (mask, mask)])
            return r
        inv = (~mask) & M32
        low = ((inv + 1) & inv) == 0 and inv + 1
        if low and x[2] % low == 0:
            r = plain('and', [(x[0], x[1]), (mask, mask)])
            return norm(r[0], r[1], x[2]) if r[2] == 0 and r != TOP3 else None
        return None
    if k == 'or':
        x, y = (args[0], args[1]) if args[0][2] else (args[1], args[0])
        if y[2] or x[0] < 0:
            return None
        width = y[1].bit_length()
        low = (1 << width) - 1
        kx, mx = known(x[0], x[1])
        if x[2] % (1 << width) == 0 and (mx & low) == 0:
            return norm(x[0] + y[0], x[1] + y[1], x[2])
        return None
    if k == 'shl':
        x, n = args
        if n[2] or n[0] != n[1] or x[0] < 0:
            return None
        return norm(x[0] << n[0], x[1] << n[0], x[2] << n[0])
    if k == 'mullo':
        x, m = (args[0], args[1]) if args[0][2] else (args[1], args[0])
        if m[2] or m[0] != m[1] or x[0] < 0 or m[0] >= 1 << 24:
            return None
        return norm(x[0] * m[0], x[1] * m[0], x[2] * m[0])
    return None


class Ctx:
    """Analysis context: virtual-voice bound and the machine's memory map
    (for memory cells: '@BASE:index' holds the last constant-index store)."""

    def __init__(self, machine=None, vv_bounded=False):
        self.m = machine
        self.vv_bounded = vv_bounded

    def cell_space(self, base, idx):
        """(space, lowest, highest) DSP addresses that may alias base[idx]."""
        m = self.m
        priv = m is not None and m.private_memory
        if base == 'SX':
            return ('x', idx, idx)
        if base == 'SY':
            return ('y', idx, idx)
        if base == 'Y':
            return ('y', 0x800 + idx, 0x800 + idx + (64 * VVMAX if priv else 0))
        if base == 'XP':
            return ('x', 0x800 + idx, 0x800 + idx + 64 * VVMAX)
        if base == 'BLK':
            return ('y', 0x100 + idx, 0x120 + idx)
        if base == 'EXT':
            return ('xy', 0x135600 + idx, 0x135600 + idx + 1536 * VVMAX)
        return None


NOCTX = Ctx()


def cell(base, idx):
    return f'@{base}:{idx}'


def kill_cells(env, test):
    for key in [k for k in env if k[0] == '@']:
        base, idx = key[1:].split(':')
        if test(base, int(idx)):
            del env[key]


def transfer(o, env, ctx=NOCTX):
    k = o[0]
    if k == 'guard_y':
        # raw word Y[k] - s*VV lies in [lo, hi] past this entry check
        env[cell('Y', o[1])] = norm(o[2], o[3], o[4])
        return
    if k == 'store':
        base, idx, val = o[1:]
        iv = ival(env, idx)
        if iv[0] == iv[1]:
            v = sval(env, val)
            if v == TOP3:
                env.pop(cell(base, iv[0]), None)
            else:
                env[cell(base, iv[0])] = v
        else:
            kill_cells(env, lambda b, i: b == base and iv[0] <= i <= iv[1])
        return
    if k == 'dwrite':
        space, addr = o[1], o[2]
        a = ival(env, addr)
        lo, hi = (a[0], a[1]) if a[1] <= 0xffffff else (0, 0xffffff)

        def hit(b, i):
            if ctx.m is None:
                return True
            r = ctx.cell_space(b, i)
            if r is None:
                return True
            return space in r[0] and not (r[2] < lo or r[1] > hi)
        kill_cells(env, hit)
        return
    d = dest(o)
    if d is None:
        return
    if k == 'field_load':
        env[d] = (0, 0, 1) if ctx.vv_bounded and o[2] == 'virtual_voice' else TOP3
        return
    if k == 'load':
        iv = ival(env, o[3])
        env[d] = env.get(cell(o[2], iv[0]), TOP3) if iv[0] == iv[1] else TOP3
        return
    if k == 'global_load':
        env[d] = TOP3
        return
    if k == 'dread':
        env[d] = (0, 0xffffff, 0)
        return
    if k == 'agu':
        old, m, step = (ival(env, x) for x in o[2:])
        if old[0] == old[1] and m[0] == m[1] and step[0] == step[1]:
            v = agu_fold(old[0], m[0], step[0])
            env[d] = (v, v, 0)
            return
        if m[0] == m[1]:
            mv = m[0]
            if mv == 0xffffff:
                if step[0] == step[1]:
                    lo, hi = old[0] + step[0], old[1] + step[0]
                    if old[1] <= 0xffffff and (lo >> 24) == (hi >> 24):
                        env[d] = (lo & 0xffffff, hi & 0xffffff, 0)
                        return
                env[d] = (0, 0xffffff, 0)
                return
            if step[0] == step[1] and old[1] <= 0xffffff:
                env[d] = agu_range(old, mv, s32(step[0]))
                return
        env[d] = TOP3
        return
    if k == 'satlo':
        args = [ival(env, x) for x in o[2:]]
        if all(a[0] == a[1] for a in args):
            v = satlo(*[a[0] for a in args])
            env[d] = (v, v, 0)
        else:
            env[d] = norm(0, max(args[1][1], args[4][1]))
        return
    if k in MULTI or k == 'sarv':
        env[d] = TOP3
        if k in MULTI:
            env[o[2]] = (0, 0xffffff, 0) if k != 'mulpair' else TOP3
        return
    r = symbolic(k, o, env)
    if r is None:
        r = plain(k, [ival(env, x) for x in o[2:]])
    env[d] = r


def agu_range(old, m, step):
    """Interval of md_agu(old, m, step) for old in [lo, hi], constant m, step."""
    mask = fill(m)
    lo, hi = old
    if (lo & ~mask) == (hi & ~mask):
        llo, lhi = lo & mask, hi & mask
    else:
        llo, lhi = 0, mask
    blo, bhi = lo & ~mask, hi & ~mask
    olo, ohi = llo + step, lhi + step
    parts = []
    if olo < 0:
        parts.append((olo + m + 1, min(ohi, -1) + m + 1))
    if olo <= m and ohi >= 0:
        parts.append((max(olo, 0), min(ohi, m)))
    if ohi > m:
        parts.append((max(olo, m + 1) - m - 1, ohi - m - 1))
    rlo = min(p[0] for p in parts)
    rhi = max(p[1] for p in parts)
    return norm(blo + rlo, bhi + rhi)


def join_value(x, y):
    if x == y:
        return x
    if x[2] == y[2]:
        return norm(min(x[0], y[0]), max(x[1], y[1]), x[2])
    a, b = conc(x), conc(y)
    return (min(a[0], b[0]), max(a[1], b[1]), 0)


def join(a, b, keys):
    out = {}
    for k in keys:
        x, y = a.get(k, TOP3), b.get(k, TOP3)
        out[k] = join_value(x, y)
    return out


def widen_value(old, new):
    if old == new:
        return new
    if old[2] != new[2]:
        return TOP3
    lo = new[0] if new[0] >= old[0] else (0 if new[2] == 0 else old[0] - (1 << 24))
    if new[1] <= old[1]:
        hi = new[1]
    else:
        hi = (1 << max(new[1], 1).bit_length()) - 1
        hi = min([x for x in THRESHOLDS if x >= new[1]] + [hi])
    return norm(lo, hi, new[2])


# region limits used as widening thresholds (one less than each boundary)
THRESHOLDS = [0x1f, 0x7f, 0xff, 0x13f, 0x15f, 0x1ff, 0x7ff, 0x83f, 0xbff, 0x1ffff, 0x14ffff, 0x1fffff, 0xffff7f, 0xffffff]


def find_loops(ops, ranges, pred):
    """Counted DSP loops: header block -> (latch block, counter variable)."""
    label_block = {}
    for bi, (s, e) in enumerate(ranges):
        for i in range(s, e):
            if ops[i][0] == 'label':
                label_block[ops[i][1]] = bi
            else:
                break
    loops = {}
    for bi, (s, e) in enumerate(ranges):
        last = ops[e - 1]
        if last[0] != 'jnz' or not isinstance(last[1], str) or not last[1].startswith('loop_'):
            continue
        h = label_block[last[2]]
        c = last[1]
        if h > bi or e - 2 < s or ops[e - 2] != ('sub', c, c, 1):
            continue
        body = set(range(h, bi + 1))
        if any(p not in body for b in body if b != h for p in pred[b]):
            continue
        defs = [i for b in body for i in range(*ranges[b]) if dest(ops[i]) == c]
        if defs != [e - 2]:
            continue
        loops[h] = (bi, c, body)
    return loops


def inductions(ops, ranges, loops, h):
    """Variables only advanced by constants inside loop h:
    var -> [(step, enclosing inner loop headers)] for each update site."""
    latch, c, body = loops[h]
    inner = {g: v for g, v in loops.items() if g != h and g in body}
    # every other back edge inside the body must belong to a counted inner loop
    for b in body:
        last = ops[ranges[b][1] - 1]
        if last[0] in ('jmp', 'jz', 'jnz'):
            for g, (gl, gc, gb) in loops.items():
                pass
    defs = defaultdict(list)
    for b in body:
        for i in range(*ranges[b]):
            d = dest(ops[i])
            if d is not None:
                defs[d].append((b, i))
            if ops[i][0] in MULTI:
                defs[ops[i][2]].append((b, i))
    result = {}
    for v, where in defs.items():
        if v == c:
            continue
        sites = []
        for b, i in where:
            step = offset_of(ops, ranges[b][0], i, v)
            if step is None or abs(step) >= 1 << 20:
                sites = None
                break
            sites.append((step, [g for g, (gl, gc, gb) in inner.items() if b in gb], b, i))
        if sites:
            result[v] = sorted(sites, key=lambda x: x[3])
    return result


def offset_of(ops, start, i, v):
    """k if ops[i] sets v to (old v) + k through a chain of constant adds,
    copies and 24-bit masks inside one block (v not redefined meanwhile)."""
    o = ops[i]
    if dest(o) != v:
        return None
    k = 0
    x, at = v, i + 1
    cur = o
    pos = i
    while True:
        if cur[0] == 'add' and isinstance(cur[3], int) and isinstance(cur[2], str):
            k += s32(cur[3])
            x = cur[2]
        elif cur[0] == 'add' and isinstance(cur[2], int) and isinstance(cur[3], str):
            k += s32(cur[2])
            x = cur[3]
        elif cur[0] == 'and' and cur[3] == 0xffffff and isinstance(cur[2], str):
            x = cur[2]
        elif cur[0] == 'mov' and isinstance(cur[2], str):
            x = cur[2]
        else:
            return None
        if x == v:
            # the v read here must be the value before this update
            if any(dest(ops[j]) == v for j in range(pos + 1, i)):
                return None
            return k if dest(ops[i]) == v else None
        # find the definition of x earlier in the block
        j = pos - 1
        while j >= start and dest(ops[j]) != x:
            j -= 1
        if j < start:
            return None
        cur, pos = ops[j], j


def dominators(succ, pred):
    n = len(succ)
    full = (1 << n) - 1
    dom = [full] * n
    dom[0] = 1
    changed = True
    while changed:
        changed = False
        for b in range(1, n):
            new = full
            for p in pred[b]:
                new &= dom[p]
            if not pred[b]:
                new = 0
            new |= 1 << b
            if new != dom[b]:
                dom[b] = new
                changed = True
    return dom


def back_edges_ok(ranges, succ, loops, h):
    """Inside loop h, every back edge is the latch of h or of a counted inner loop."""
    latch, c, body = loops[h]
    for b in body:
        for t in succ[b]:
            if t <= b and t in body:
                if t == h and b == latch:
                    continue
                if t in loops and loops[t][0] == b and t in body:
                    continue
                return False
    return True


def analyze(ops, machine=None):
    """Abstract state at entry of every op (None if unreachable)."""
    ranges, succ, pred = blocks(ops)
    _, live_in, _ = liveness(ops)
    vv_bounded = bool(ops) and ops[0][0] == 'guard_vv'
    ctx = Ctx(machine, vv_bounded)
    loops = find_loops(ops, ranges, pred)
    ind = {h: (inductions(ops, ranges, loops, h) if back_edges_ok(ranges, succ, loops, h) else {}) for h in loops}
    dom = dominators(succ, pred) if loops else None
    n = len(ranges)
    out = [None] * n
    entry = [None] * n
    visits = [0] * n
    back_target = {t for b in range(n) for t in succ[b] if t <= b}
    import heapq
    work = [0]
    queued = {0}
    while work:
        bi = heapq.heappop(work)
        queued.discard(bi)
        keys = live_in[bi]
        if bi == 0:
            new = {}
        else:
            ins = [p for p in pred[bi] if out[p] is not None]
            if not ins:
                continue
            new = None
            for p in ins:
                st = {k: out[p].get(k, TOP3) for k in keys}
                for k, v in out[p].items():
                    if k[0] == '@':
                        st[k] = v
                if new is None:
                    new = st
                else:
                    cells = {k for k in new if k[0] == '@'} & {k for k in st if k[0] == '@'}
                    new = join(new, st, keys | cells)
        overridden = set()
        if bi in loops:
            latch, c, body = loops[bi]
            outside = [p for p in pred[bi] if p not in body and out[p] is not None]
            if outside:
                ent = None
                for p in outside:
                    st = {k: out[p].get(k, TOP3) for k in keys | {c} | set(ind[bi])}
                    ent = st if ent is None else join(ent, st, set(st))
                cnt = conc(ent.get(c, TOP3))
                if 1 <= cnt[0] and cnt[1] <= 0x10000:
                    trips = cnt[1]
                    if c in keys:
                        new[c] = (1, trips, 0)
                        overridden.add(c)
                    for v, sites in ind[bi].items():
                        if v not in keys:
                            continue
                        e = ent.get(v, TOP3)
                        if e == TOP3:
                            continue
                        if all(not nests and (dom[latch] >> b) & 1 for st, nests, b, i in sites):
                            # every update runs exactly once per iteration, in this order
                            net, part, lo_p, hi_p = 0, 0, 0, 0
                            for st, nests, b, i in sites:
                                part += st
                                lo_p, hi_p = min(lo_p, part), max(hi_p, part)
                            net = part
                            lo = e[0] + min(0, (trips - 1) * net)
                            hi = e[1] + max(0, (trips - 1) * net)
                            clo, chi = conc((lo + min(lo_p, 0), hi + max(hi_p, 0), e[2]))
                            if 0 <= clo and chi <= 0xffffff:
                                new[v] = norm(lo, hi, e[2])
                                overridden.add(v)
                            continue
                        if not (all(st >= 0 for st, *_ in sites) or all(st <= 0 for st, *_ in sites)):
                            continue
                        step = 0
                        ok = True
                        for st, nests, b, i in sites:
                            mult = 1
                            for g in nests:
                                gl, gc, gb = loops[g]
                                # inner loop counters must be bounded where they start
                                gi = conc(out_start(ops, ranges, gb, g, gc))
                                if not (1 <= gi[0] and gi[1] <= 0x10000):
                                    ok = False
                                    break
                                mult *= gi[1]
                            step += st * mult
                        if not ok:
                            continue
                        span = (trips - 1) * step
                        whole = trips * step
                        lo, hi = (e[0], e[1] + span) if step >= 0 else (e[0] + span, e[1])
                        clo, chi = conc((e[0] + min(0, whole), e[1] + max(0, whole), e[2]))
                        if e != TOP3 and 0 <= clo and chi <= 0xffffff:
                            new[v] = norm(lo, hi, e[2])
                            overridden.add(v)
        if entry[bi] is not None:
            visits[bi] += 1
            if bi in back_target and visits[bi] > 3:
                new = {k: (v if k in overridden else widen_value(entry[bi].get(k, v), v)) for k, v in new.items()}
            if new == entry[bi]:
                continue
        entry[bi] = new
        env = dict(new)
        s, e = ranges[bi]
        for i in range(s, e):
            transfer(ops[i], env, ctx)
        out[bi] = env
        for t in succ[bi]:
            if t not in queued:
                heapq.heappush(work, t)
                queued.add(t)
    states = [None] * len(ops)
    for bi, (s, e) in enumerate(ranges):
        if entry[bi] is None:
            continue
        env = dict(entry[bi])
        for i in range(s, e):
            states[i] = dict(env)
            transfer(ops[i], env, ctx)
    return states


def out_start(ops, ranges, body, h, c):
    """Placeholder bound for a nested loop counter: its constant initialiser
    immediately before the header, if any."""
    s, e = ranges[h - 1] if h > 0 else (0, 0)
    for i in reversed(range(s, e)):
        o = ops[i]
        if dest(o) == c:
            if o[0] == 'mov' and isinstance(o[2], int):
                return (o[2] & M32, o[2] & M32, 0)
            break
    return TOP3


# ------------------------------------------------------------------ memory map
class Regions:
    """md_read_<space>/md_write_<space> dispatch order from generate.py, for
    one value of the virtual voice (private-memory machines)."""

    def __init__(self, machine):
        self.m = machine

    def read(self, space, a, vv=0):
        m = self.m
        if space == 'src':
            if 0x20000 <= a < 0x200000:
                return ('SRC', 0x20000)
            if 0x200000 <= a < 0xffff80:
                return ('ZERO', 0)
            return None
        limit = m.scratch_x_words if space == 'x' else m.scratch_y_words
        if a < limit:
            return ('S' + space.upper(), 0)
        if space == 'x' and (256 <= a < 0x800 or 0xc00 <= a < 0x20000):
            return ('BOOTX', 0)
        if m.buffered_output and space == 'y' and a == 0x1ff:
            return ('CONST_FDB42B', 0)
        if m.private_memory:
            if 0x135600 + vv * 1536 <= a < 0x135c00 + vv * 1536:
                return ('EXT', 0x135600 + vv * 1536)
            if space == 'x' and 0x800 + vv * 64 <= a < 0x840 + vv * 64:
                return ('XP', 0x800 + vv * 64)
            if space == 'y' and 0x800 + vv * 64 <= a < 0x840 + vv * 64:
                return ('Y', 0x800 + vv * 64)
        elif space == 'y' and 0x800 <= a < 0x800 + m.words:
            return ('Y', 0x800)
        if space == 'y' and not m.buffered_output and 0x100 <= a < 0x140:
            return ('BLK31', 0x100)
        if 0x148000 <= a < 0x150000:
            return ('SINE', 0x148000)
        for base, values in m.tables.items():
            if base <= a < base + len(values):
                return ('T%x' % base, base)
        if m.requires_source_image:
            if 0x20000 <= a < 0x200000:
                return ('SRC', 0x20000)
            if 0x200000 <= a < 0xffff80:
                return ('ZERO', 0)
        return None

    def write(self, space, a, vv=0):
        m = self.m
        limit = m.scratch_x_words if space == 'x' else m.scratch_y_words
        if a < limit:
            return ('S' + space.upper(), 0)
        if m.buffered_output and space == 'y' and a == 0x1ff:
            return None
        if m.private_memory:
            if 0x135600 + vv * 1536 <= a < 0x135c00 + vv * 1536:
                return ('EXT', 0x135600 + vv * 1536)
            if space == 'x' and 0x800 + vv * 64 <= a < 0x840 + vv * 64:
                return ('XP', 0x800 + vv * 64)
            if space == 'y' and 0x800 + vv * 64 <= a < 0x840 + vv * 64:
                return ('Y', 0x800 + vv * 64)
        elif space == 'y' and 0x800 <= a < 0x800 + m.words:
            return ('Y', 0x800)
        if space == 'y' and not m.buffered_output and 0x100 <= a < 0x140:
            return ('BLK31', 0x100)
        return None

    def uniform(self, fn, space, lo, hi, raw=False, vv=0):
        """Single region for every address in [lo, hi] (after & 0xffffff)."""
        if hi > 0xffffff or lo > hi or lo < 0:
            return None
        if raw and hi >= 0xffff80:
            if lo >= 0xffff80:
                return ('ZERO', 0)
            return None
        points = {lo, hi}
        cuts = [0x20, 0x80, 0x100, 0x140, 0x160, 0x1ff, 0x200, 0x800, 0xc00, 0x20000, 0x148000, 0x150000,
                0x200000, 0xffff80, self.m.scratch_x_words, self.m.scratch_y_words, 0x800 + self.m.words, 1,
                0x135600 + vv * 1536, 0x135c00 + vv * 1536, 0x800 + vv * 64, 0x840 + vv * 64]
        for base, values in self.m.tables.items():
            cuts += [base, base + len(values)]
        for c in cuts:
            for p in (c - 1, c):
                if lo <= p <= hi:
                    points.add(p)
        found = {fn(space, p, vv) for p in points}
        if len(found) != 1:
            return None
        r = found.pop()
        if r is None:
            return None
        if r[0] == 'BLK31' and (lo >> 5) != (hi >> 5):
            # address & 31 is only a linear index inside one 32-word half
            return None
        return r

    def cut_points(self, vv):
        cuts = {0, 0x20, 0x80, 0x100, 0x120, 0x140, 0x160, 0x1ff, 0x200, 0x800, 0xc00, 0x20000, 0x148000, 0x150000,
                0x200000, 0xffff80, 0x1000000, self.m.scratch_x_words, self.m.scratch_y_words, 0x800 + self.m.words,
                0x135600 + vv * 1536, 0x135c00 + vv * 1536, 0x800 + vv * 64, 0x840 + vv * 64}
        for base, values in self.m.tables.items():
            cuts |= {base, base + len(values)}
        return sorted(c for c in cuts if 0 <= c <= 0x1000000)

    def clean(self, fn, space, a_lo, a_hi, vv_bounded):
        """Largest [A, B) around profiled addresses [a_lo, a_hi] (virtual voice
        0) on which the chain always selects the same region, for every
        virtual voice: (name, A, B, origin0, t) with the interval and origin
        shifted by t*VV, or None."""
        r0 = fn(space, a_lo, 0)
        if r0 is None or fn(space, a_hi, 0) != r0:
            return None
        voices = range(VVMAX + 1) if self.m.private_memory else (0,)
        if self.m.private_memory and not vv_bounded:
            return None
        t = 0
        if len(voices) > 1:
            r1 = fn(space, a_lo + 0, 1)
            # the window law: origin(vv) = origin0 + t*vv for the same name
            for step in (0, 64, 1536):
                if fn(space, a_lo + step, 1) == (r0[0], r0[1] + step):
                    t = step
                    break
            else:
                return None
        lo, hi = 0, 0x1000000
        for vv in voices:
            want = (r0[0], r0[1] + t * vv)
            cuts = self.cut_points(vv)
            a = a_lo + t * vv
            b = a_hi + t * vv
            # segment containing a, extended while the region stays the same
            i = max(k for k, c in enumerate(cuts) if c <= a)
            j = max(k for k, c in enumerate(cuts) if c <= b)
            if any(fn(space, cuts[k], vv) != want for k in range(i, j + 1)):
                return None
            while i > 0 and fn(space, cuts[i - 1], vv) == want:
                i -= 1
            while j + 1 < len(cuts) - 1 and fn(space, cuts[j + 1], vv) == want:
                j += 1
            A, B = cuts[i], cuts[j + 1]
            lo, hi = max(lo, A - t * vv), min(hi, B - t * vv)
        if lo > a_lo or hi <= a_hi:
            return None
        return r0[0], lo, hi, r0[1], t

    def classify(self, fn, space, v, raw, vv_bounded):
        """Region and index law for an abstract address v = (lo, hi, s):
        (name, origin0, t) with origin(vv) = origin0 + t*vv, or None."""
        lo, hi, s = v
        if self.m.private_memory:
            if not vv_bounded:
                return None
            voices = range(VVMAX + 1)
        else:
            if s:
                return None
            voices = (0,)
        found = []
        for vv in voices:
            r = self.uniform(fn, space, lo + s * vv, hi + s * vv, raw, vv)
            if r is None:
                return None
            found.append(r)
        names = {r[0] for r in found}
        if len(names) != 1:
            return None
        o0 = found[0][1]
        t = found[1][1] - o0 if len(found) > 1 else 0
        if any(r[1] != o0 + t * vv for vv, r in zip(voices, found)):
            return None
        return found[0][0], o0, t


def resolve(ops, machine, states):
    regions = Regions(machine)
    vv_bounded = bool(ops) and ops[0][0] == 'guard_vv'
    out = []
    n = [0]

    def t():
        n[0] += 1
        return f'q{n[0]}'

    def index(addr, v, origin, step):
        """IR computing (addr & 0xffffff) - origin - step*VV."""
        lo, hi, s = v
        if s == step and lo == hi:
            return [], (lo - origin) & M32
        code = []
        idx = t()
        code.append(('and', idx, addr, 0xffffff))
        if origin:
            k = t()
            code.append(('sub', k, idx, origin))
            idx = k
        if step:
            if origin:
                # one subtraction of the entry-computed window base
                code = code[:-1]
                idx = code[-1][1] if code else addr
                k = t()
                code.append(('sub', k, idx, vv_base(step, origin)))
                return code, k
            k = t()
            code.append(('sub', k, idx, vv_base(step)))
            idx = k
        return code, idx

    bases = {}

    def vv_base(step, origin=0):
        """Entry-computed origin + step*VV (one variable per window law)."""
        name = f'_vvs{step}' + (f'_{origin:x}' if origin else '')
        if name not in bases:
            if step & (step - 1) == 0:
                code = [('field_load', '_vvf', 'virtual_voice'), ('shl', name, '_vvf', step.bit_length() - 1)]
            else:
                code = [('field_load', '_vvf', 'virtual_voice'), ('mullo', name, '_vvf', step)]
            if origin:
                code.append(('add', name, name, origin))
            bases[name] = code
        return name

    for i, o in enumerate(ops):
        env = states[i]
        if env is None:
            out.append(o)
            continue
        if o[0] == 'dread':
            d, space, addr, raw = o[1:]
            r = regions.classify(regions.read, space, sval(env, addr), raw, vv_bounded)
            if r is not None and r[0] not in ('BOOTX', 'CONST_FDB42B'):
                base, origin, step = r
                if base == 'ZERO':
                    out.append(('mov', d, 0))
                    continue
                code, idx = index(addr, sval(env, addr), origin, step)
                out += code
                if base == 'BLK31':
                    k = t()
                    out.append(('and', k, idx, 31))
                    idx = k
                    base = 'BLK'
                v = t()
                out.append(('load', v, base, idx))
                if base == 'SRC' or base.startswith('T'):
                    out.append(('mov', d, v))
                else:
                    out.append(('and', d, v, 0xffffff))
                continue
        if o[0] == 'dwrite':
            space, addr, s = o[1:]
            r = regions.classify(regions.write, space, sval(env, addr), False, vv_bounded)
            if r is not None:
                base, origin, step = r
                code, idx = index(addr, sval(env, addr), origin, step)
                out += code
                if base == 'BLK31':
                    k = t()
                    out.append(('and', k, idx, 31))
                    idx = k
                    base = 'BLK'
                v = t()
                out.append(('shl', v, s, 8))
                out.append(('sar', v, v, 8))
                out.append(('store', base, idx, v))
                continue
        out.append(o)
    if bases:
        # define the window bases right after the entry guards
        n = 0
        while n < len(out) and out[n][0] in ('guard_vv', 'guard_y'):
            n += 1
        pre = [x for name in sorted(bases) if not any(dest(y) == name for y in out) for x in bases[name]]
        out = out[:n] + pre + out[n:]
    return out


# ------------------------------------------------------------------ rewrites
def const_rewrite(ops, states):
    out = []
    for i, o in enumerate(ops):
        env = states[i]
        if env is None:
            if o[0] == 'label':
                out.append(o)
            continue

        def f(x):
            if isinstance(x, int):
                return x & M32
            v = env.get(x, TOP3)
            return v[0] if v[0] == v[1] and v[2] == 0 else x
        o2 = replace_sources(o, f)
        k = o2[0]
        if k in ('jz', 'jnz') and isinstance(o2[1], int):
            taken = (o2[1] == 0) == (k == 'jz')
            if taken:
                out.append(('jmp', o2[2]))
            continue
        if k == 'load' and isinstance(o2[3], int):
            v = env.get(cell(o2[2], o2[3] & M32))
            if v is not None and v[0] == v[1] and v[2] == 0:
                out.append(('mov', o2[1], v[0]))
                continue
        if k in PURE and k not in ('load', 'global_load', 'field_load') and all(isinstance(x, int) for x in o2[2:]):
            if k == 'agu':
                out.append(('mov', o2[1], agu_fold(*o2[2:])))
            else:
                out.append(('mov', o2[1], fold(k, *o2[2:])))
            continue
        out.append(o2)
    return out


def specialize_agu(ops):
    out = []
    n = [0]

    def t():
        n[0] += 1
        return f'g{n[0]}'
    for o in ops:
        if o[0] == 'agu' and isinstance(o[3], int):
            d, old, m, step = o[1:]
            if m == 0xffffff:
                s = t()
                out.append(('add', s, old, step))
                out.append(('and', d, s, 0xffffff))
                continue
            mask = fill(m)
            if mask == m and isinstance(step, int) and s32(step) <= m + 1 and -s32(step) <= m + 1:
                hi, lo, s = t(), t(), t()
                out.append(('and', hi, old, (~mask) & M32))
                out.append(('add', s, old, step))
                out.append(('and', lo, s, mask))
                out.append(('or', d, hi, lo))
                continue
        out.append(o)
    return out


def simplify(ops):
    """Peephole identities using single definitions inside straight-line code."""
    defs = {}
    count = {}
    for o in ops:
        d = dest(o)
        if d is not None:
            count[d] = count.get(d, 0) + 1
    for o in ops:
        d = dest(o)
        if d is not None and count[d] == 1:
            defs[d] = o
    out = []
    for o in ops:
        k = o[0]
        if k == 'and' and isinstance(o[3], int) and isinstance(o[2], str) and o[2] in defs:
            inner = defs[o[2]]
            if inner[0] == 'and' and isinstance(inner[3], int) and not (isinstance(inner[2], str) and count.get(inner[2], 0) > 1 and False):
                pass
        out.append(o)
    return out


def copy_prop(ops):
    """Local forward copy propagation within basic blocks."""
    ranges, _, _ = blocks(ops)
    out = list(ops)
    for s, e in ranges:
        copies = {}
        for i in range(s, e):
            o = out[i]

            def f(x):
                return copies.get(x, x) if isinstance(x, str) else x
            o = replace_sources(o, f)
            d = dest(o)
            if d is not None:
                # kill copies that read or write d
                copies = {a: b for a, b in copies.items() if a != d and b != d}
                if o[0] == 'mov' and o[2] != d:
                    copies[d] = o[2]
            out[i] = o
    return out


def liveness(ops):
    ranges, succ, pred = blocks(ops)
    live_in = [set() for _ in ranges]
    use_def = []
    for s, e in ranges:
        use, defs = set(), set()
        for i in range(s, e):
            for x in sources(ops[i]):
                if isinstance(x, str) and x not in defs:
                    use.add(x)
            d = dest(ops[i])
            if d is not None:
                defs.add(d)
            if ops[i][0] in MULTI:
                defs.add(ops[i][2])
        use_def.append((use, defs))
    changed = True
    while changed:
        changed = False
        for bi in reversed(range(len(ranges))):
            out = set()
            for t in succ[bi]:
                out |= live_in[t]
            use, defs = use_def[bi]
            new = use | (out - defs)
            if new != live_in[bi]:
                live_in[bi] = new
                changed = True
    live_out = []
    for bi in range(len(ranges)):
        out = set()
        for t in succ[bi]:
            out |= live_in[t]
        live_out.append(out)
    return ranges, live_in, live_out


def dce(ops):
    ranges, live_in, live_out = liveness(ops)
    keep = [True] * len(ops)
    for bi, (s, e) in enumerate(ranges):
        live = set(live_out[bi])
        for i in reversed(range(s, e)):
            o = ops[i]
            d = dest(o)
            if d is not None and o[0] in PURE and d not in live:
                keep[i] = False
                continue
            if o[0] in MULTI and o[1] not in live and o[2] not in live:
                keep[i] = False
                continue
            for x in dests(o):
                live.discard(x)
            for x in sources(o):
                if isinstance(x, str):
                    live.add(x)
    return [o for o, k in zip(ops, keep) if k]


def cleanup_labels(ops):
    """Remove jumps to the immediately following label and unreferenced labels."""
    used = set()
    for o in ops:
        if o[0] == 'jmp':
            used.add(o[1])
        if o[0] in ('jz', 'jnz'):
            used.add(o[2])
    out = []
    for i, o in enumerate(ops):
        if o[0] == 'jmp':
            j = i + 1
            nxt = set()
            while j < len(ops) and ops[j][0] == 'label':
                nxt.add(ops[j][1])
                j += 1
            if o[1] in nxt:
                continue
        out.append(o)
    used = set()
    for o in out:
        if o[0] == 'jmp':
            used.add(o[1])
        if o[0] in ('jz', 'jnz'):
            used.add(o[2])
    out = [o for o in out if o[0] != 'label' or o[1] in used]
    # drop unreachable code after unconditional jumps / returns until a label
    final = []
    dead = False
    for o in out:
        if o[0] == 'label':
            dead = False
        if not dead:
            final.append(o)
        if o[0] in ('jmp', 'ret'):
            dead = True
    return final


# ------------------------------------------------------------------ value numbering
# Every writer stores sign-extended 24-bit words: translated stores and md_write
# (sx24), the shared-scratch initializers, and the TRX-BD/SD scratch writes
# (saturated, or canonicalized by tools/machines/resident/fast_relink.py).
CANONICAL = ('Y', 'BLK', 'XP', 'EXT', 'SX', 'SY')


def pow2(c):
    """k when c == 2**k (k < 31), else None."""
    if c is None or c <= 0 or c & (c - 1) or c >= 1 << 31:
        return None
    return c.bit_length() - 1


def is_mask(c):
    return isinstance(c, int) and c != 0 and ((c + 1) & c) == 0


def lvn(ops, states):
    """Local value numbering: CSE, copy folding and mask/sign-extension algebra."""
    ranges, _, _ = blocks(ops)
    out = list(ops)
    for s, e in ranges:
        vn_of = {}        # var -> vn
        expr_of = {}      # vn -> (op, args...) with args as vns or ('c', int)
        holder = {}       # vn -> var currently holding it
        table = {}        # expression key -> vn
        consts = {}       # vn -> int
        counter = [0]

        def new_vn():
            counter[0] += 1
            return counter[0]

        def vn(x):
            if isinstance(x, int):
                key = ('const', x & M32)
                if key not in table:
                    v = new_vn()
                    table[key] = v
                    consts[v] = x & M32
                    expr_of[v] = key
                return table[key]
            if x not in vn_of:
                v = new_vn()
                vn_of[x] = v
                holder[v] = x
                expr_of[v] = ('var', x)
            return vn_of[x]

        def operand(v):
            if v in consts:
                return consts[v]
            h = holder.get(v)
            return h

        def s24(v):
            ex = expr_of.get(v)
            if ex and ex[0] == 'var' and ex[1] in ('x0', 'x1', 'y0', 'y1'):
                return True
            if v in consts:
                c = consts[v]
                return c < 0x800000 or c >= 0xff800000
            if not ex:
                return False
            if ex[0] == 'sar' and ex[2] in consts and consts[ex[2]] >= 8:
                return True
            if ex[0] == 'load' and ex[1] in CANONICAL:
                # voice-private memory only ever holds sign-extended words
                return True
            if ex[0] == 'sub' and ex[1] in consts and consts[ex[1]] == 0 and s24(ex[2]) and False:
                return True
            if ex[0] == 'and' and ex[2] in consts and consts[ex[2]] <= 0x7fffff:
                return True
            if ex[0] == 'max' and ex[2] in consts and consts[ex[2]] == 0xff800000:
                inner = expr_of.get(ex[1])
                return bool(inner and inner[0] == 'min' and inner[2] in consts and consts[inner[2]] == 0x7fffff)
            return False

        def tz(v):
            """Known trailing zero bits of value number v."""
            if v in consts:
                c = consts[v]
                return 32 if c == 0 else (c & -c).bit_length() - 1
            ex = expr_of.get(v)
            if not ex:
                return 0
            if ex[0] == 'shl' and ex[2] in consts:
                return min(32, tz(ex[1]) + consts[ex[2]])
            if ex[0] == 'mullo':
                return min(32, tz(ex[1]) + tz(ex[2]))
            if ex[0] == 'and':
                return max(tz(ex[1]), tz(ex[2]))
            return 0

        def kill(var):
            v = vn_of.pop(var, None)
            if v is not None and holder.get(v) == var:
                # another variable may still hold the same value
                alt = next((x for x, y in vn_of.items() if y == v), None)
                if alt is None:
                    holder.pop(v, None)
                else:
                    holder[v] = alt

        mem_version = {}
        for i in range(s, e):
            o = out[i]
            env = states[i] if states else None
            k = o[0]
            d = dest(o)
            if k in ('store', 'dwrite', 'dread', 'global_store', 'field_store') or k in CONTROL:
                if k == 'store':
                    mem_version[o[1]] = mem_version.get(o[1], 0) + 1
                if k == 'dwrite':
                    for b in ('SX', 'SY', 'Y', 'BLK', 'XP', 'EXT'):
                        mem_version[b] = mem_version.get(b, 0) + 1
                if k == 'field_store':
                    mem_version['F'] = mem_version.get('F', 0) + 1
                if k == 'global_store':
                    mem_version['G'] = mem_version.get('G', 0) + 1
                # rewrite sources to current holders of their values
                def f(x):
                    if isinstance(x, int):
                        return x
                    v = vn_of.get(x)
                    if v is None:
                        return x
                    if v in consts:
                        return consts[v]
                    return holder.get(v, x)
                out[i] = replace_sources(o, f)
                if d is not None:
                    kill(d)
                    vn(d)
                continue
            if d is None or k not in PURE:
                continue
            args = [vn(x) for x in o[2:]] if k not in ('load', 'global_load', 'field_load') else None
            if k == 'load':
                key = ('load', o[2], vn(o[3]), mem_version.get(o[2], 0))
            elif k == 'global_load':
                key = ('global_load', o[2], mem_version.get('G', 0))
            elif k == 'field_load':
                key = ('field_load', o[2], mem_version.get('F', 0))
            else:
                key = None
            result = None   # ('copy', vn) or rewritten op
            if args is not None:
                a = args
                c = [consts.get(x) for x in a]
                if all(x is not None for x in c) and k != 'agu':
                    result = ('const', fold(k, *c))
                elif k == 'mov':
                    result = ('copy', a[0])
                elif k in ('add', 'or', 'xor', 'sub', 'shl', 'shr', 'sar') and c[-1] == 0:
                    result = ('copy', a[0])
                elif k in ('add', 'or', 'xor') and c[0] == 0:
                    result = ('copy', a[1])
                elif k == 'and' and c[1] == M32:
                    result = ('copy', a[0])
                elif k == 'mulhi' and 0xff800000 in c and s24(a[1] if c[0] == 0xff800000 else a[0]):
                    # times -1.0
                    x = a[1] if c[0] == 0xff800000 else a[0]
                    src = operand(x)
                    if src is not None:
                        out[i] = ('sub', d, 0, src)
                        k, a, c = 'sub', [vn(0), x], [0, consts.get(x)]
                elif k == 'and' and is_mask(c[1]) and tz(a[0]) >= c[1].bit_length():
                    result = ('const', 0)
                elif k == 'and' and is_mask(c[1]):
                    m = c[1]
                    iv = ival(env, o[2]) if env else TOP
                    inner = expr_of.get(a[0])
                    if iv[1] <= m:
                        result = ('copy', a[0])
                    elif inner and inner[0] == 'and' and inner[2] in consts:
                        a = [inner[1], vn(consts[inner[2]] & m)]
                        if is_mask(consts[inner[2]]) and consts[inner[2]] <= m:
                            result = ('copy', a[0] if False else args[0])
                    elif inner and inner[0] == 'sar' and inner[2] in consts and m <= (0xffffffff >> consts[inner[2]]):
                        # low bits of an arithmetic shift equal those of a logical shift
                        sh = expr_of.get(inner[1])
                        if consts[inner[2]] == 8 and sh and sh[0] == 'shl' and sh[2] in consts and consts[sh[2]] == 8 and m <= 0xffffff:
                            a = [sh[1], vn(m)]
                elif k == 'shl' and c[1] == 8:
                    inner = expr_of.get(a[0])
                    if inner and inner[0] == 'and' and inner[2] in consts and consts[inner[2]] == 0xffffff:
                        a = [inner[1], a[1]]
                    elif inner and inner[0] == 'sar' and inner[2] in consts and consts[inner[2]] == 8:
                        sh = expr_of.get(inner[1])
                        if sh and sh[0] == 'shl' and sh[2] in consts and consts[sh[2]] == 8:
                            a = [sh[1], a[1]]
                elif k == 'mullo' and (pow2(c[0]) is not None or pow2(c[1]) is not None):
                    x, n = (a[1], pow2(c[0])) if pow2(c[0]) is not None else (a[0], pow2(c[1]))
                    src = operand(x)
                    if src is not None:
                        out[i] = ('shl', d, src, n)
                        k, a, c = 'shl', [x, vn(n)], [consts.get(x), n]
                elif k == 'mulhi' and any(cc is not None and pow2(cc) is not None and pow2(cc) <= 23 for cc in c):
                    x, n = (a[1], pow2(c[0])) if c[0] is not None and pow2(c[0]) is not None else (a[0], pow2(c[1]))
                    src = operand(x)
                    if src is not None:
                        out[i] = ('sar', d, src, 23 - n)
                        k, a, c = 'sar', [x, vn(23 - n)], [consts.get(x), 23 - n]
                        if n == 23:
                            result = ('copy', x)
                elif k == 'shl' and c[1] is not None and (expr_of.get(a[0]) or ('',))[0] == 'shl' \
                        and expr_of[a[0]][2] in consts and consts[expr_of[a[0]][2]] + c[1] < 32:
                    inner = expr_of[a[0]]
                    n = consts[inner[2]] + c[1]
                    src = operand(inner[1])
                    if src is not None:
                        out[i] = ('shl', d, src, n)
                        a, c = [inner[1], vn(n)], [consts.get(inner[1]), n]
                elif k == 'sar' and c[1] == 8:
                    inner = expr_of.get(a[0])
                    if inner and inner[0] == 'shl' and inner[2] in consts and consts[inner[2]] == 8 and s24(inner[1]):
                        result = ('copy', inner[1])
                    elif inner and inner[0] == 'shl' and inner[2] in consts and consts[inner[2]] == 8:
                        iv = ival(env, operand(inner[1])) if env and operand(inner[1]) is not None and not isinstance(operand(inner[1]), int) else TOP
                        if iv[1] <= 0x7fffff:
                            result = ('copy', inner[1])
                if result is None:
                    if k in ('add', 'and', 'or', 'xor', 'min', 'max', 'eq', 'mulhi', 'mullo'):
                        key = (k,) + tuple(sorted(a))
                    else:
                        key = (k,) + tuple(a)
            if result is None and key in table and (table[key] in consts or holder.get(table[key]) is not None):
                result = ('copy', table[key])
            kill(d)
            if result is not None and result[0] == 'const':
                v = vn(result[1])
                vn_of[d] = v
                if holder.get(v) is None:
                    holder[v] = d
                out[i] = ('mov', d, result[1])
                continue
            if result is not None and result[0] == 'copy':
                v = result[1]
                src = operand(v)
                if src is not None:
                    vn_of[d] = v
                    if holder.get(v) is None:
                        holder[v] = d
                    out[i] = ('mov', d, src)
                    continue
            v = new_vn()
            vn_of[d] = v
            holder[v] = d
            if args is not None:
                expr_of[v] = (k,) + tuple(a)
                ops_args = []
                ok = True
                for x in a:
                    src = operand(x)
                    if src is None:
                        ok = False
                        break
                    ops_args.append(src)
                if ok:
                    out[i] = (k, d) + tuple(ops_args)
                    table[key] = v
                else:
                    expr_of[v] = ('var', d)
            else:
                expr_of[v] = key
                srcs = []
                ok = True
                if k == 'load':
                    idx = operand(vn(o[3]))
                    if idx is None:
                        ok = False
                    else:
                        out[i] = ('load', d, o[2], idx)
                if ok:
                    table[key] = v
    return out


UNROLL = [0]


def unresolved(ops, machine, lo=0, hi=None):
    """Generic accesses in ops[lo:hi] that resolve() cannot map statically."""
    states = analyze(ops, machine)
    regions = Regions(machine)
    vv = bool(ops) and ops[0][0] == 'guard_vv'
    n = 0
    for i in range(lo, len(ops) if hi is None else hi):
        o = ops[i]
        if states[i] is None or o[0] not in ('dread', 'dwrite'):
            continue
        if o[0] == 'dread':
            r = regions.classify(regions.read, o[2], sval(states[i], o[3]), o[4], vv)
            if r is not None and r[0] not in ('BOOTX', 'CONST_FDB42B'):
                continue
        else:
            if regions.classify(regions.write, o[1], sval(states[i], o[2]), False, vv) is not None:
                continue
        n += 1
    return n


def unroll(ops, budget, machine=None):
    """Fully unroll counted loops (constant trip count <= 16) that still
    contain generic memory accesses, so each copy sees precise addresses."""
    ranges, succ, pred = blocks(ops)
    loops = find_loops(ops, ranges, pred)
    refs = defaultdict(int)
    for o in ops:
        if o[0] == 'jmp':
            refs[o[1]] += 1
        elif o[0] in ('jz', 'jnz'):
            refs[o[2]] += 1
    chosen = []
    for h, (latch, c, body) in sorted(loops.items(), key=lambda kv: -len(kv[1][2])):
        if any(h in cb for _, _, cb in chosen) or any(b in body for b, _, _ in chosen):
            continue
        s0 = ranges[h][0]
        e0 = ranges[latch][1]
        if s0 == 0 or ops[s0 - 1][0] != 'mov' or ops[s0 - 1][1] != c or not isinstance(ops[s0 - 1][2], int):
            continue
        n = ops[s0 - 1][2] & M32
        if not 2 <= n <= 16:
            continue
        if [p for p in pred[h] if p not in body] != [h - 1] or not back_edges_ok(ranges, succ, loops, h):
            continue
        header_labels = [ops[i][1] for i in range(*ranges[h]) if ops[i][0] == 'label']
        if any(refs[l] != (1 if l == ops[e0 - 1][2] else 0) for l in header_labels):
            continue
        if not any(ops[i][0] in ('dread', 'dwrite') for i in range(s0, e0)):
            continue
        size = (e0 - s0) * (n - 1)
        if size > budget:
            continue
        if machine is not None:
            before = unresolved(ops, machine, s0, e0)
            trial = expand(ops, [(h, (s0, e0, n), body)])
            after = unresolved(trial, machine, s0, s0 + (e0 - s0 - 1) * n)
            if before == 0 or after > before * n // 2:
                continue
        budget -= size
        chosen.append((h, (s0, e0, n), body))
    if not chosen:
        return ops, False
    return expand(ops, chosen), True


def expand(ops, chosen):
    out = []
    pos = 0
    for h, (s0, e0, n), body in sorted(chosen, key=lambda x: x[1][0]):
        out += ops[pos:s0]
        part = ops[s0:e0 - 1]           # without the latch jnz
        labels = {o[1] for o in part if o[0] == 'label'}
        for j in range(n):
            UNROLL[0] += 1
            tag = f'_u{UNROLL[0]}'

            def ren(x):
                return x + tag if isinstance(x, str) and x in labels else x
            for o in part:
                if o[0] == 'label' or o[0] == 'jmp':
                    out.append((o[0], ren(o[1])))
                elif o[0] in ('jz', 'jnz'):
                    out.append((o[0], o[1], ren(o[2])))
                else:
                    out.append(o)
        pos = e0
    out += ops[pos:]
    return out


def optimize(ops, machine, rounds=4):
    budget = max(2000, 2 * len(ops))
    r = 0
    while r < rounds:
        r += 1
        if r >= 2:
            before = len(ops)
            ops, changed = unroll(ops, budget, machine)
            budget -= len(ops) - before
            if changed and r == rounds:
                rounds += 1
        states = analyze(ops, machine)
        ops = const_rewrite(ops, states)
        ops = specialize_agu(ops)
        states = analyze(ops, machine)
        ops = resolve(ops, machine, states)
        states = analyze(ops, machine)
        ops = lvn(ops, states)
        ops = sink_updates(ops)
        ops = copy_prop(ops)
        ops = dce(ops)
        ops = cleanup_labels(ops)
    ops = fuse_acc(ops, machine)
    ops = dce(ops)
    ops = sat_fuse(ops)
    ops = bool_arith(ops, machine)
    return ops


def sat_fuse(ops):
    """The low word of a limited accumulator move,
        p = c1 < h;  n = h < c2;  o = p | n;  z = o == 0;
        r = ((0 - p) & k) | ((0 - z) & l)
    is satlo(h, l, c1, c2, k): k if p, else 0 if n, else l (p and n
    exclude neither: p wins in both). Matched exactly within one block with
    every intermediate used only inside the pattern and h, l unchanged."""
    ranges = blocks(ops)[0]
    live_out = liveness(ops)[2]
    uses = defaultdict(list)
    for i, o in enumerate(ops):
        for x in sources(o):
            if isinstance(x, str):
                uses[x].append(i)
    out = list(ops)
    for bi, (s, e) in enumerate(ranges):
        local = {}
        for i in range(s, e):
            for x in dests(ops[i]):
                local[x] = i
        defs = defaultdict(int)
        for i in range(s, e):
            for x in dests(ops[i]):
                defs[x] += 1

        def op(x):
            return ops[local[x]] if isinstance(x, str) and x in local and defs[x] == 1 else None

        def pair(o, k):
            # o = (k, d, a, b) commutative: the two operands
            return (o[2], o[3]) if o is not None and o[0] == k else None
        for i in range(s, e):
            r = ops[i]
            if r[0] != 'or' or defs[r[1]] != 1:
                continue
            for A, B in (r[2:4], r[3:1:-1]):
                a, b = op(A), op(B)
                if a is None or b is None or a[0] != 'and' or b[0] != 'and':
                    continue
                # a = (0 - p) & k
                for mp, kk in (a[2:4], a[3:1:-1]):
                    nm = op(mp)
                    if not isinstance(kk, int) or nm is None or nm[0] != 'sub' or nm[2] != 0:
                        continue
                    p = nm[3]
                    po = op(p)
                    if po is None or po[0] != 'lt' or not isinstance(po[2], int):
                        continue
                    c1, h = po[2], po[3]
                    # b = (0 - z) & l
                    for mz, l in (b[2:4], b[3:1:-1]):
                        zm = op(mz)
                        if zm is None or zm[0] != 'sub' or zm[2] != 0:
                            continue
                        z = zm[3]
                        zo = op(z)
                        if zo is None or zo[0] != 'eq' or 0 not in zo[2:4]:
                            continue
                        oo = op(zo[3] if zo[2] == 0 else zo[2])
                        if oo is None or oo[0] != 'or' or p not in oo[2:4]:
                            continue
                        n = oo[3] if oo[2] == p else oo[2]
                        no = op(n)
                        if no is None or no[0] != 'lt' or no[2] != h or not isinstance(no[3], int):
                            continue
                        c2 = no[3]
                        inner = {A: [i], B: [i], mp: [local[A]], mz: [local[B]], p: sorted([local[mp], local[oo[1]]]),
                                 n: [local[oo[1]]], oo[1]: [local[z]], z: [local[mz]]}
                        if len(set(inner)) != 8 or any(sorted(uses[x]) != u for x, u in inner.items()) \
                                or any(x in live_out[bi] for x in inner):
                            continue
                        first = min(local[x] for x in inner)
                        if any(ops[j] is not None and set(dests(ops[j])) & {h, l} for j in range(first, i)):
                            continue
                        for x in inner:
                            out[local[x]] = None
                        out[i] = ('satlo', r[1], h, l, c1, c2, kk)
                        break
                    else:
                        continue
                    break
                else:
                    continue
                break
    return [o for o in out if o is not None]


def bool_arith(ops, machine=None):
    """Comparisons whose operands are provably small become arithmetic:
    with a, b in [-2^30, 2^30) (signed) or [0, 2^31) (unsigned) a - b cannot
    overflow, so a < b is its sign bit (sub + shr 31), and x == 0 for
    x in [0, 2^31) is the sign bit of x - 1. A comparison feeding only
    `0 - c` (a 0/-1 mask) becomes sar 31 of the difference. Comparisons that
    only steer the next conditional jump are left alone (compare-and-jump)."""
    states = analyze(ops, machine)
    ranges, _, _ = blocks(ops)
    _, _, live_out = liveness(ops)
    start = {}
    for bi, (s, e) in enumerate(ranges):
        for i in range(s, e):
            start[i] = (bi, s, e)
    uses, defs = defaultdict(list), defaultdict(int)
    names = set()
    for i, o in enumerate(ops):
        for x in sources(o):
            if isinstance(x, str):
                uses[x].append(i)
                names.add(x)
        for x in dests(o):
            defs[x] += 1
            names.add(x)
    fresh = [0]

    def tmp():
        while True:
            fresh[0] += 1
            n = f'zb{fresh[0]}'
            if n not in names:
                return n
    lim_s, lim_u = 1 << 30, 1 << 31

    def signed_ok(env, s, i, x):
        if _s24(ops, s, i, x, states, wide=True):
            return True
        r = signed_range(ival(env, x))
        return r is not None and -lim_s <= r[0] and r[1] < lim_s

    def unsigned_ok(env, s, i, x):
        return ival(env, x)[1] < lim_u or _small(ops, s, i, x, states)
    out = list(ops)
    for i, o in enumerate(ops):
        if o[0] not in ('lt', 'ltu', 'eq') or states[i] is None:
            continue
        d, a, b = o[1:]
        env = states[i]
        bi, s, e = start[i]
        q = ops[i + 1] if i + 1 < len(ops) else None
        if q is not None and q[0] in ('jz', 'jnz') and q[1] == d and d not in live_out[bi]:
            continue
        if o[0] == 'eq':
            if b == 0 and not isinstance(a, int):
                x = a
            elif a == 0 and not isinstance(b, int):
                x = b
            else:
                continue
            if not unsigned_ok(env, s, i, x):
                continue
            diff = ('sub', None, x, 1)
        elif o[0] == 'lt':
            if not (signed_ok(env, s, i, a) and signed_ok(env, s, i, b)):
                continue
            diff = ('sub', None, a, b)
        else:
            if not (unsigned_ok(env, s, i, a) and unsigned_ok(env, s, i, b)):
                continue
            diff = ('sub', None, a, b)
        t = tmp()
        diff = (diff[0], t) + diff[2:]
        u = uses[d]
        if defs[d] == 1 and len(u) == 1 and u[0] < e and u[0] > i and ops[u[0]][0] == 'sub' \
                and ops[u[0]][2] == 0 and ops[u[0]][3] == d and d not in live_out[bi]:
            out[i] = [diff]
            out[u[0]] = ('sar', ops[u[0]][1], t, 31)
            continue
        out[i] = [diff, ('shr', d, t, 31)]
    flat = []
    for o in out:
        if isinstance(o, list):
            flat += o
        else:
            flat.append(o)
    return flat


# ------------------------------------------------------------------ accumulator fusion
def _block_defuse(ops, s, e, live_out):
    """Within ops[s:e]: def position -> list of use positions of that definition;
    a definition that may reach the block end while live-out gets a None use."""
    last = {}
    uses = {}
    for i in range(s, e):
        o = ops[i]
        if o is None:
            continue
        for x in sources(o):
            if isinstance(x, str) and x in last:
                uses[last[x]].append(i)
        for x in dests(o):
            last[x] = i
            uses[i] = uses.get(i, [])
    for x, i in last.items():
        if x in live_out:
            uses[i].append(None)
    return last, uses


def fuse_acc(ops, machine=None):
    """56-bit accumulator arithmetic in compact forms:
    mac24(H, L, a, b) = (mulhi(a, b), (mullo(a, b) << 1) & 0xffffff), with a
    a sign-extended 24-bit value (mac24p: a constant already shifted left 8);
    acc(dh, dl, h0, l0, h1, l1, ...) = (sum h + (sum l >> 24), sum l & 0xffffff)
    for terms l < 2^24 (fewer than 64 terms, so sum l cannot wrap)."""
    states = analyze(ops, machine)
    ranges, live_in, live_out = liveness(ops)
    out = list(ops)
    for bi, (s, e) in enumerate(ranges):
        _mac24(out, s, e, live_out[bi], states)
        _acc(out, s, e, live_out[bi], states)
    return [o for o in out if o is not None]


def _s24(ops, s, i, x, states, wide=False):
    """x (at op i) is provably a sign-extended 24-bit (wide: 25-bit) value."""
    lim = 1 << (24 if wide else 23)
    if isinstance(x, int):
        v = s32(x)
        return -lim <= v < lim
    if x in ('x0', 'x1', 'y0', 'y1'):
        return True
    env = states[i]
    if env is not None:
        r = signed_range(ival(env, x))
        if r is not None and -lim <= r[0] and r[1] < lim:
            return True
    # structural: defined in this block by sar >= 8 (or saturation)
    for j in range(i - 1, s - 1, -1):
        o = ops[j]
        if o is not None and x in dests(o):
            if o[0] == 'sar' and isinstance(o[3], int) and o[3] >= (8 if not wide else 7):
                return True
            if o[0] == 'max' and o[3] == 0xff800000 and isinstance(o[2], str):
                m = _def_in(ops, s, j, o[2])
                return m is not None and ops[m][0] == 'min' and ops[m][3] == 0x7fffff
            if o[0] == 'load' and o[2] in CANONICAL:
                return True
            return False
    return False


def _mac24(out, s, e, live_out, states):
    last, uses = _block_defuse(out, s, e, live_out)
    pos = {}
    for i in range(s, e):
        if out[i] is not None:
            for x in dests(out[i]):
                pos.setdefault(x, []).append(i)

    def single_def(x, i):
        return x is not None and len(pos.get(x, [])) == 1 and pos[x][0] == i
    for i in range(s, e):
        o = out[i]
        if o is None or o[0] != 'and' or o[3] != 0xffffff or not isinstance(o[2], str):
            continue
        l2 = o[1]
        # L1 = shl(L, 1) used only here; L = mullo(a, b) used only by L1
        j = _def_in(out, s, i, o[2])
        if j is None or out[j][0] != 'shl' or out[j][3] != 1 or uses.get(j) != [i]:
            continue
        k = _def_in(out, s, j, out[j][2])
        if k is None or out[k][0] != 'mullo' or uses.get(k) != [j]:
            continue
        a, b = out[k][2], out[k][3]
        # matching mulhi in the block with the same operands and unchanged inputs
        h = None
        for q in range(s, e):
            p = out[q]
            if p is not None and p[0] == 'mulhi' and {p[2], p[3]} == {a, b} and \
                    not _redefined(out, min(q, k), max(q, k), (a, b)):
                h = q
                break
        if h is None:
            continue
        H = out[h][1]
        first = min(h, i)
        # the moved results must not be read or redefined between first and their old definition
        if not _quiet(out, first, h, H) or not _quiet(out, first, i, l2) or _redefined(out, first, max(h, k), (a, b)):
            continue
        if not single_def(H, h) or not single_def(l2, i):
            continue
        if _s24(out, s, k, a, states) and _s24(out, s, k, b, states, wide=True):
            x, y = a, b
        elif _s24(out, s, k, b, states) and _s24(out, s, k, a, states, wide=True):
            x, y = b, a
        else:
            continue
        if isinstance(x, int):
            new = ('mac24p', H, l2, (x << 8) & M32, y)
        else:
            new = ('mac24', H, l2, x, y)
        for q in (h, i, j, k):
            out[q] = None
        out[first] = new
        last, uses = _block_defuse(out, s, e, live_out)
        pos = {}
        for q in range(s, e):
            if out[q] is not None:
                for z in dests(out[q]):
                    pos.setdefault(z, []).append(q)


def _def_in(out, s, i, x):
    for j in range(i - 1, s - 1, -1):
        if out[j] is not None and x in dests(out[j]):
            return j
    return None


def _redefined(out, a, b, names):
    names = {x for x in names if isinstance(x, str)}
    return any(out[q] is not None and set(dests(out[q])) & names for q in range(a + 1, b))


def _quiet(out, a, b, x):
    """No use or definition of x strictly between positions a and b."""
    for q in range(a, b):
        o = out[q]
        if o is None or q == a and False:
            continue
        if q > a and (x in [y for y in sources(o) if isinstance(y, str)] or x in dests(o)):
            return False
    return True


def _small(out, s, i, x, states):
    """x < 2^24 at op i."""
    if isinstance(x, int):
        return x < 1 << 24
    j = _def_in(out, s, i, x)
    if j is not None:
        o = out[j]
        if o[0] == 'and' and isinstance(o[3], int) and o[3] < 1 << 24:
            return True
        if o[0] in ('mac24', 'mac24p', 'acc') and o[2] == x:
            return True
    env = states[i]
    if env is not None and ival(env, x)[1] < 1 << 24:
        return True
    return False


def acc_terms(o):
    """[(sign, h, l)] of an 'acc' op: ('acc', dh, dl, negmask, h0, l0, h1, l1, ...)."""
    mask = o[3]
    n = (len(o) - 4) // 2
    return [(-1 if mask >> k & 1 else 1, o[4 + 2 * k], o[5 + 2 * k]) for k in range(n)]


def make_acc(dh, dl, terms):
    mask = 0
    flat = []
    for k, (sg, h, l) in enumerate(terms):
        if sg < 0:
            mask |= 1 << k
        flat += [h, l]
    return ('acc', dh, dl, mask) + tuple(flat)


def _acc(out, s, e, live_out, states):
    changed = True
    while changed:
        changed = False
        last, uses = _block_defuse(out, s, e, live_out)
        for i in range(s, e):
            o = out[i]
            if o is None or o[0] != 'add':
                continue
            dh = o[1]
            # dh = T + C with C = S >> 24
            for ci, ti in ((o[2], o[3]), (o[3], o[2])):
                if not isinstance(ci, str) or not isinstance(ti, str):
                    continue
                c = _def_in(out, s, i, ci)
                if c is None or out[c][0] != 'shr' or out[c][3] != 24 or uses.get(c) != [i]:
                    continue
                t = _def_in(out, s, i, ti)
                if t is None or out[t][0] != 'add' or uses.get(t) != [i]:
                    continue
                sv = out[c][2]
                si = _def_in(out, s, c, sv)
                if si is None or out[si][0] != 'add':
                    continue
                us = uses.get(si, [])
                if len(us) != 2 or c not in us:
                    continue
                li = [u for u in us if u != c][0]
                if li is None or out[li][0] != 'and' or out[li][2] != sv or out[li][3] != 0xffffff:
                    continue
                dl = out[li][1]
                lo = [out[si][2], out[si][3]]
                hi = [out[t][2], out[t][3]]
                if not all(_small(out, s, si, x, states) for x in lo):
                    continue
                first = min(si, t)
                q = None
                for cand in (max(i, li), min(i, li)):
                    if _redefined(out, first, cand, [x for x in lo + hi]):
                        continue
                    # the result defined at the other position moves to cand
                    other, odest = (li, dl) if cand == i else (i, dh)
                    if other < cand and not _quiet(out, other, cand, odest):
                        continue
                    if other > cand and not _quiet(out, cand, other, odest):
                        continue
                    if other > cand and _redefined(out, first, other, [x for x in lo + hi]) and False:
                        continue
                    q = cand
                    break
                if q is None:
                    continue
                for x in (c, t, si, i, li):
                    out[x] = None
                out[q] = make_acc(dh, dl, [(1, hi[0], lo[0]), (1, hi[1], lo[1])])
                changed = True
                break
            if changed:
                break
        if changed:
            continue
        last, uses = _block_defuse(out, s, e, live_out)
        for i in range(s, e):
            o = out[i]
            if o is None or o[0] != 'acc' or len(acc_terms(o)) >= 63:
                continue
            terms = acc_terms(o)
            for n, (sg, h, l) in enumerate(terms):
                # a negated product term: (-H - (L != 0), -L & mask)
                if isinstance(h, str) and isinstance(l, str):
                    jl = _def_in(out, s, i, l)
                    jh = _def_in(out, s, i, h)
                    if jl is not None and jh is not None and uses.get(jl) == [i] and uses.get(jh) == [i]:
                        ol, oh = out[jl], out[jh]
                        if ol[0] == 'and' and ol[3] == 0xffffff and isinstance(ol[2], str) and oh[0] == 'sub' \
                                and isinstance(oh[2], str) and isinstance(oh[3], str):
                            jn = _def_in(out, s, jl, ol[2])
                            jh1 = _def_in(out, s, jh, oh[2])
                            jb = _def_in(out, s, jh, oh[3])
                            if None not in (jn, jh1, jb) and uses.get(jn) == [jl] and uses.get(jh1) == [jh] \
                                    and uses.get(jb) == [jh]:
                                on, o1, ob = out[jn], out[jh1], out[jb]
                                if on[0] == 'sub' and on[2] == 0 and o1[0] == 'sub' and o1[2] == 0 and \
                                        ob[0] == 'ltu' and ob[2] == 0 and ob[3] == on[3] and isinstance(on[3], str):
                                    H, L = o1[3], on[3]
                                    if _small(out, s, jn, L, states) and \
                                            not _redefined(out, min(jn, jh1, jb), i, [H, L]):
                                        terms[n] = (-sg, H, L)
                                        for x in (jl, jh, jn, jh1, jb):
                                            out[x] = None
                                        out[i] = make_acc(o[1], o[2], terms)
                                        changed = True
                                        break
                # a single-use sum h = pa + pb splits into two terms
                if isinstance(h, str):
                    j = _def_in(out, s, i, h)
                    if j is not None and out[j][0] in ('add', 'sub') and uses.get(j) == [i]:
                        pa, pb = out[j][2], out[j][3]
                        if not _redefined(out, j, i, [pa, pb]):
                            terms[n] = (sg, pa, l)
                            terms.append((sg if out[j][0] == 'add' else -sg, pb, 0))
                            out[i] = make_acc(o[1], o[2], terms)
                            out[j] = None
                            changed = True
                            break
            if changed:
                break
        if changed:
            continue
        # merge an acc whose results feed only one other acc
        last, uses = _block_defuse(out, s, e, live_out)
        for i in range(s, e):
            o = out[i]
            if o is None or o[0] != 'acc':
                continue
            users = [u for u in uses.get(i, []) if u is not None]
            if None in uses.get(i, []) or len(set(users)) != 1:
                continue
            u = users[0]
            p = out[u]
            if p[0] != 'acc' or len(acc_terms(p)) + len(acc_terms(o)) > 64:
                continue
            pairs = acc_terms(p)
            hits = [k for k, (sg, h, l) in enumerate(pairs) if (h, l) == (o[1], o[2])]
            if len(hits) != 1 or sum(x in (o[1], o[2]) for _, h, l in pairs for x in (h, l)) != 2:
                continue
            if _redefined(out, i, u, [x for x in o[4:] if isinstance(x, str)]):
                continue
            sg = pairs[hits[0]][0]
            del pairs[hits[0]]
            inner = [(sg * g, h, l) for g, h, l in acc_terms(o)]
            out[u] = make_acc(p[1], p[2], inner + pairs)
            out[i] = None
            changed = True
            break


# ------------------------------------------------------------------ post-increment sinking
def sink_updates(ops):
    """'t = r; ...; r = f(r); ...; use(t)' -> 'use(r); r = f(r)' inside a block,
    when nothing between the update and the last use of t reads r or the
    update's temporaries. Removes the copy of a post-incremented pointer."""
    ranges, live_in, live_out = liveness(ops)
    out = list(ops)
    for bi, (s, e) in enumerate(ranges):
        i = s
        while i < e:
            o = out[i]
            if o is None or o[0] != 'mov' or not isinstance(o[2], str) or o[1] == o[2]:
                i += 1
                continue
            t, r = o[1], o[2]
            if _try_sink(out, s, e, i, t, r, live_out[bi]):
                continue
            i += 1
    return [o for o in out if o is not None]


def _try_sink(out, s, e, i, t, r, live_out):
    # uses of t after i (t must die inside the block, not redefined)
    uses = []
    for q in range(i + 1, e):
        o = out[q]
        if o is None:
            continue
        if t in [x for x in sources(o) if isinstance(x, str)]:
            uses.append(q)
        if t in dests(o):
            return False
    if not uses or t in live_out:
        return False
    # first redefinition of r after i
    j = next((q for q in range(i + 1, e) if out[q] is not None and r in dests(out[q])), None)
    if j is None or j > uses[-1] or out[j][0] in CONTROL:
        return False
    if any(out[q] is not None and out[q][0] in CONTROL for q in range(i + 1, uses[-1] + 1)):
        return False
    # the update chain: ops in (i, j] whose results feed only the definition of r
    chain = [j]
    need = {x for x in sources(out[j]) if isinstance(x, str)}
    for q in range(j - 1, i, -1):
        o = out[q]
        if o is None:
            continue
        d = dests(o)
        if d and set(d) & need:
            if len(d) != 1 or o[0] not in PURE or o[0] in ('load', 'global_load', 'field_load'):
                return False
            chain.append(q)
            need |= {x for x in sources(o) if isinstance(x, str)}
    chain.sort()
    temps = {dests(out[q])[0] for q in chain}
    inputs = need - temps
    if inputs - {r}:
        # other inputs must not change before the moved chain runs
        for q in range(chain[0], uses[-1] + 1):
            o = out[q]
            if o is not None and q not in chain and set(dests(o)) & (inputs - {r}):
                return False
    last = uses[-1]
    for q in range(chain[0], last + 1):
        o = out[q]
        if o is None or q in chain:
            continue
        srcs = {x for x in sources(o) if isinstance(x, str)}
        if set(dests(o)) & (temps | {r}) or srcs & temps or (q > j and r in srcs):
            return False
    # chain temps (other than r) must be dead after the chain
    for q in range(chain[-1] + 1, e):
        o = out[q]
        if o is None:
            continue
        if {x for x in sources(o) if isinstance(x, str)} & (temps - {r}):
            return False
    if live_out & (temps - {r}):
        return False
    region = list(range(i, last + 1))
    new = []
    for q in region:
        if q == i or q in chain or out[q] is None:
            continue
        o = out[q]
        if q in uses:
            o = replace_sources(o, lambda x: r if x == t else x)
        new.append(o)
    new += [replace_sources(out[q], lambda x: r if x == t else x) for q in chain]
    new += [None] * (len(region) - len(new))
    out[i:last + 1] = new
    return True
