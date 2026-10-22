"""Two voices of one machine per instruction stream (SHARC+ SIMD, PEx/PEy).

`<prefix>_render_pair(pair, sine, scratch)` renders two voices of the same
machine at once. PEx computes voice A, PEy voice B (the S complements of the
same registers), so register pressure is unchanged.

Memory contract (voice structs word-interleaved, A at even words, B at odd):

* `pair` (R4): the two voice structs interleaved word by word;
* `scratch` (R12): private scratch of 2 * (scratch_x_words + scratch_y_words)
  words (merge mode: twice that plus one word -- lane B's written marks
  follow). The y scratch is interleaved at word 2 * scratch_x_words. The x
  scratch is either interleaved at word 0, or -- when its varying-index
  accesses dominate (`halves`) -- kept as two lane halves: lane A works on
  md_shared_scratch_x itself and lane B on a private copy at word 0.
  On entry the shared scratch is copied into the private lanes, and on exit
  lane B's words are copied back. With the value-range analysis
  (`scratch_runs`) only the words direct accesses can touch are copied in,
  and only those direct stores can write are copied back; generic reads of
  other scratch words read the shared scratch, which nothing writes during
  the render. All this equals rendering A then B sequentially when
  (verified by the pair gate, fast_pair_gate, with the shared scratch
  randomized before every block) the machine's outputs and scratch writes
  do not depend on scratch contents it did not write in the same block, and
  both voices write the same scratch words -- P-I-SD: 196 X and 139 Y words,
  every block;
* read-only tables (sine, source tables) are shared, not interleaved.

Returns R0 = 0 when rendered, 1 (and no change to either voice or the shared
scratch) when an entry guard fails for either voice: the caller then renders
the two voices with the scalar renders.

Emission (on top of fast_sharc.Emitter):

* frame slots are word pairs (PEx at a, PEy at a + 1);
* constant-index voice and scratch accesses address both lanes at once
  (offsets doubled); varying indices into interleaved state are kept
  doubled where that pays (`shadow_indices`, exact mod 2^32);
* a varying index, a shared table or a global goes through the lane flag
  (BTF, clear in PEx and set in PEy from entry on; only BIT TST/XOR change
  it): two conditional accesses, `IF NOT TF` at voice A's address (M0),
  `IF TF` at voice B's (M4)
* data branches: diamonds are if-converted to per-lane selects and
  conditional stores (`if_convert`); a remaining branch (or software loop
  counter) is taken in SIMD when both lanes agree, else the per-lane
  fallback runs two SISD copies of the function, lane A then lane B, from
  the branch to its join point (`rejoin_points`), and SIMD resumes there
  (`lane_divergence`, PairEmitter with `lane`);
* merge mode (`PAIR_MERGE`): lane B's scratch writes are marked word by
  word; at exit each written word takes B's value where B wrote, else A's;
* generic memory sites: both lanes inside the profiled window take the
  inline path (a SIMD jump is taken only if its condition holds in both
  PEs); otherwise SIMD is left and each lane runs the full region chain on
  its own interleaved view, then SIMD resumes.
"""
import re
from collections import defaultdict

from tools.machines.translation import fast_sharc as S
from tools.machines.translation.fast_opt import sources, dests, replace_sources, M32, PURE, MULTI as MULTI_OPS

# ('csel', d, c, new, old, ran_if_nonzero): a pure op with a destination for
# liveness and allocation (like fast_sharc's 'mra'); only the pair path has it
PURE.add('csel')

# Pairing contract per machine (fast_pair_contract, scalar renders only):
# voice B's render after voice A's must not depend on A's scratch writes
# (else the machine is not paired); where the two voices can write
# different scratch words in a block, the pair render runs in merge mode.
PAIR_UNPAIRABLE = frozenset({'E12-RS', 'P-I-CC', 'P-I-RC'})
PAIR_MERGE = frozenset({'E12-BC', 'E12-BD', 'E12-BR', 'E12-CB', 'E12-CC', 'E12-CH', 'E12-CP',
    'E12-HT', 'E12-LT', 'E12-OH', 'E12-RC', 'E12-SD', 'E12-SH', 'E12-TA', 'E12-TR'})

PEYEN = '0x200000'
BDCST1 = '0x800000'
BTF = '0x40000'
LANE_A, LANE_B = 'NOT TF', 'TF'       # conditions true in PEx only / PEy only
INTERLEAVED = ('Y', 'BLK', 'XP', 'EXT', 'SX', 'SY')
VOICE = ('Y', 'BLK', 'XP', 'EXT')      # the two voices' own structs: lane A at I0, lane B at I5 + 1
SINE_ADDR = 0x80550000                 # the shared sine table (firmware and gates)
# the pair render's frame: a fixed DDR block (reserved by fast_relink), I6 at
# its top word; the frame grows down in words
PAIR_FRAME, PAIR_FRAME_WORDS = 0x8183d800, 256
PAIR_FRAME_TOP = PAIR_FRAME + 4 * (PAIR_FRAME_WORDS - 1)


# ------------------------------------------------------------------ IR: if-conversion
SMALL_BRANCH = 8        # ops up to which a branch outside loops is if-converted
LOOP_BRANCH = 8         # ... and inside loops (a larger one stays a branch: both arms
                        # every iteration plus the register pressure of the selects
                        # cost more than taking the lane fallback when lanes disagree)


def if_convert(ops):
    """Data branches -> both sides executed, results selected per lane:

        jz/jnz c, L; <A>; label L
        jz/jnz c, L1; <A>; jmp L2; label L1; <B>; label L2

    (labels used by these jumps only) with A, B of pure ops (constant-index
    loads included), multiply-accumulates and constant-index stores become A
    and B on fresh names, then ('csel', d, c, new, old, nz) per assigned
    variable and ('cstore', base, idx, val, c, nz) per store; nz = 1 if the
    side ran when c != 0 (the condition's own csel comes last). Branches of
    at most LOOP_BRANCH ops inside loops and SMALL_BRANCH ops elsewhere are
    converted; a larger one is cheaper as a branch (lanes that disagree take
    the per-lane fallback). Repeated until nothing changes (nested branches)."""
    while True:
        new = _if_convert_once(ops)
        if len(new) == len(ops) and new == ops:
            return new
        ops = new


def _side_ok(block):
    import os
    for q in block:
        if q[0] == 'load':
            if not isinstance(q[3], int) or os.environ.get('FAST_IFC_NOLOAD'):
                return False
        elif q[0] in MULTI_OPS and os.environ.get('FAST_IFC_NOMULTI'):
            return False
        elif q[0] == 'store':
            if not isinstance(q[2], int) or q[1] not in INTERLEAVED:
                return False
        elif not (q[0] in PURE or q[0] in MULTI_OPS):
            return False
    return True


def _if_convert_once(ops):
    uses = defaultdict(int)
    for o in ops:
        if o[0] == 'jmp':
            uses[o[1]] += 1
        if o[0] in ('jz', 'jnz'):
            uses[o[2]] += 1
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}
    weight = ir_weights(ops)
    names = {x for o in ops for x in dests(o) + [s for s in sources(o) if isinstance(s, str)]}
    n = [0]

    def fresh(base):
        while True:
            n[0] += 1
            t = f'{base}~{n[0]}'
            if t not in names:
                names.add(t)
                return t

    def side(block, c, when, out):
        ren = {}
        for q in block:
            q = replace_sources(q, lambda x: ren.get(x, x) if isinstance(x, str) else x)
            if q[0] == 'store' and q[1] in VOICE:
                # a voice word: lane-gated conditional stores are not
                # possible, so keep the old word unless this side ran
                old, new = fresh('cs'), fresh('cs')
                out += [('load', old, q[1], q[2]), ('csel', new, c, q[3], old, when), ('store', q[1], q[2], new)]
                continue
            if q[0] == 'store':
                out.append(('cstore', q[1], q[2], q[3], c, when))
                continue
            ds = dests(q)
            for d in ds:
                ren[d] = fresh(d)
            q = (q[0],) + tuple(ren[x] if p in (1, 2) and x in ds and (p == 1 or q[0] in MULTI_OPS) else x
                                for p, x in enumerate(q[1:], 1))
            out.append(q)
        return ren
    out = []
    i = 0
    while i < len(ops):
        o = ops[i]
        done = False
        if o[0] in ('jz', 'jnz') and o[1] != 1 and not (isinstance(o[1], str) and o[1].startswith('loop_')) \
                and pos.get(o[2], -1) > i:
            j = pos[o[2]]
            c = o[1]
            when = 1 if o[0] == 'jz' else 0     # the fall-through side runs when c != 0 for jz
            a_ops = ops[i + 1:j]
            b_ops, end = [], j
            if a_ops and a_ops[-1][0] == 'jmp' and uses[o[2]] == 1 and pos.get(a_ops[-1][1], -1) > j \
                    and not S._os.environ.get('FAST_IFC_NODIAMOND'):
                end = pos[a_ops[-1][1]]
                b_ops = ops[j + 1:end]
                a_ops = a_ops[:-1]
            size = len(a_ops) + len(b_ops)
            loop_max = int(S._os.environ.get('FAST_IFC_LOOPMAX', LOOP_BRANCH))
            if (weight[i] > 1 and size <= loop_max or size <= SMALL_BRANCH) and _side_ok(a_ops) and _side_ok(b_ops):
                ren_a = side(a_ops, c, when, out)
                ren_b = side(b_ops, c, 1 - when, out)
                assigned = list(dict.fromkeys(list(ren_a) + list(ren_b)))
                assigned.sort(key=lambda d: d == c)
                for d in assigned:
                    out.append(('csel', d, c, ren_a.get(d, d), ren_b.get(d, d), when))
                # this branch is one use of its end label; it stays for others
                if uses[ops[end][1]] > 1:
                    out.append(ops[end])
                i = end + 1
                done = True
        if not done:
            out.append(o)
            i += 1
    return out


# ------------------------------------------------------------------ IR: doubled indices
SCALED = '~x2'          # suffix of a variable holding 2 * v (mod 2^32)
_LINEAR = ('mov', 'add', 'sub', 'and', 'or', 'xor')


def _var(x):
    return isinstance(x, str) and not x.startswith('loop_')


def _index_of(o, split=()):
    """The variable index of a varying-index access to interleaved state
    (not counting regions kept as two lane halves, `split`)."""
    if o[0] == 'load' and o[2] in INTERLEAVED and o[2] not in split and _var(o[3]):
        return o[3]
    if o[0] in ('store', 'cstore') and o[1] in INTERLEAVED and o[1] not in split and _var(o[2]):
        return o[2]
    return None


def _scaled_sources(o):
    """Operand positions of o that scale with its destination, or None."""
    k = o[0]
    if k in _LINEAR:
        return list(range(2, len(o)))
    if k == 'shl' and isinstance(o[3], int):
        return [2]
    if k == 'csel':
        return [3, 4]
    return None


def _two(x):
    return x + SCALED if isinstance(x, str) else (2 * x) & M32


def shadow(ops, need, states=None, split=()):
    """Give every variable in `need` a shadow v~x2 = 2v mod 2^32, defined after
    each definition of v: from the shadows of its sources when the op is
    linear (mov/add/sub/and/or/xor, constants doubled; shl by a constant;
    csel) and all its variable sources are in `need` -- doubling mod 2^32
    commutes exactly with these -- else as v + v. Index uses of v read the
    shadow. Any other use reads `v~x2 >> 1` where the value-range analysis
    (`states`, fast_opt.analyze of `ops`) proves v < 2^31 there, which is
    then exact; elsewhere it keeps v. Dead originals are then removed."""
    from tools.machines.translation.fast_opt import dce, liveness, ival
    out = []
    _, live_in, _ = liveness(ops)
    for v in sorted(need & live_in[0]):
        out.append(('add', _two(v), v, v))
    n = [0]
    for i, o in enumerate(ops):
        x = _index_of(o, split)
        if x in need:
            o = o[:2] + (_two(x),) + o[3:] if o[0] in ('store', 'cstore') else o[:3] + (_two(x),)
        env = states[i] if states else None
        if env is not None:
            conv = {}
            for y in sources(o):
                if _var(y) and y in need and not y.endswith(SCALED) and y not in conv and ival(env, y)[1] < 1 << 31:
                    n[0] += 1
                    conv[y] = f'{y}~h{n[0]}'
                    out.append(('shr', conv[y], _two(y), 1))
            if conv:
                o = replace_sources(o, lambda y: conv.get(y, y))
        out.append(o)
        if o[0] in MULTI_OPS:
            continue
        for d in dests(o):
            if d not in need:
                continue
            pos = _scaled_sources(o)
            src = ops[i]
            if pos is not None and all(not _var(src[p]) or src[p] in need for p in pos):
                out.append(tuple(_two(y) if p in pos or p == 1 else y for p, y in enumerate(src)))
            else:
                out.append(('add', _two(d), d, d))
    before = None
    while before != len(out):
        before = len(out)
        out = dce(out)
    return out


def ir_weights(ops):
    """Rough execution weight per op: 32x per enclosing backward branch."""
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}
    w = [1] * len(ops)
    for i, o in enumerate(ops):
        if o[0] in ('jz', 'jnz', 'jmp'):
            t = pos.get(o[2] if o[0] != 'jmp' else o[1])
            if t is not None and t <= i:
                for q in range(t, i + 1):
                    w[q] *= 32
    return w


def shadow_indices(ops, machine=None, split=()):
    """`shadow` for the address-like variables that pay off. Address-like
    (greatest fixed point): every use an index, a linear op, a stored value
    or a generic-access address (this keeps signal data out). Their connected
    components (through linear ops) that contain a varying index are kept
    when they lower the loop-weighted op count, an unshadowed index use
    costing one doubling."""
    from tools.machines.translation.fast_opt import analyze
    states = analyze(ops, machine)

    def cost(need):
        out = shadow(ops, need, states, split)
        w = ir_weights(out)
        total = 0
        for wi, o in zip(w, out):
            if o[0] == 'label':
                continue
            x = _index_of(o, split)
            total += wi * (2 if x and not x.endswith(SCALED) else 1)
        return total
    defs, uses = defaultdict(list), defaultdict(list)
    for o in ops:
        if o[0] not in MULTI_OPS:
            for d in dests(o):
                defs[d].append(o)
        tagged = replace_sources(o, lambda x: (x,))       # marks source positions
        for p, x in enumerate(tagged):
            if isinstance(x, tuple) and _var(x[0]):
                uses[x[0]].append((o, p))
    addr = {v for v in set(defs) | set(uses) if _var(v)}
    changed = True
    while changed:
        changed = False
        for v in sorted(addr):
            ok = True
            for o, p in uses[v]:
                if not ok:
                    break
                if _index_of(o, split) == v and (o[0] == 'load' or p == 2):
                    continue
                if o[0] in ('store', 'cstore', 'dread', 'dwrite', 'global_store', 'field_store'):
                    continue
                pos = _scaled_sources(o)
                ok = pos is not None and p in pos
            if not ok:
                addr.discard(v)
                changed = True
    parent = {v: v for v in addr}

    def find(v):
        while parent[v] != v:
            parent[v] = parent[parent[v]]
            v = parent[v]
        return v
    for o in ops:
        pos = _scaled_sources(o) if o[0] not in MULTI_OPS else None
        if pos and o[1] in addr:
            for p in pos:
                if o[p] in addr:
                    parent[find(o[p])] = find(o[1])
    comps = defaultdict(set)
    for v in addr:
        comps[find(v)].add(v)
    seeds = {x for o in ops for x in [_index_of(o, split)] if x}
    need = set()
    best = cost(need)
    for root in sorted(comps, key=lambda r: sorted(comps[r])[0]):
        comp = comps[root]
        if not comp & seeds:
            continue
        c = cost(need | comp)
        if c < best:
            need, best = need | comp, c
    # index variables left out (not address-like) still get v + v shadows
    # when that is cheaper
    for v in sorted(seeds - need):
        c = cost(need | {v})
        if c < best:
            need, best = need | {v}, c
    shadow_indices.chosen = need
    return shadow(ops, need, states, split)


# ------------------------------------------------------------------ scratch words
def scratch_runs(ops, machine, states):
    """Word runs [(lo, hi)] of the x/y scratch that direct accesses can touch
    (C) and can write (W), from the value-range analysis; a generic write
    makes its whole space touched and written."""
    from tools.machines.translation.fast_opt import ival
    size = {'SX': machine.scratch_x_words, 'SY': machine.scratch_y_words}
    touch = {b: set() for b in size}
    write = {b: set() for b in size}
    for i, o in enumerate(ops):
        if o[0] == 'dwrite':
            b = 'SX' if o[1] == 'x' else 'SY' if o[1] == 'y' else None
            if b:
                touch[b] |= set(range(size[b]))
                write[b] |= set(range(size[b]))
            continue
        if o[0] not in ('load', 'store', 'cstore'):
            continue
        base, idx = (o[2], o[3]) if o[0] == 'load' else (o[1], o[2])
        if base not in size:
            continue
        if isinstance(idx, int):
            lo = hi = idx
        else:
            lo, hi = ival(states[i], idx) if states[i] is not None else (0, size[base] - 1)
        if not 0 <= lo <= hi < size[base]:
            lo, hi = 0, size[base] - 1
        touch[base] |= set(range(lo, hi + 1))
        if o[0] != 'load':
            write[base] |= set(range(lo, hi + 1))

    def runs(words):
        out = []
        for x in sorted(words):
            if out and x == out[-1][1] + 1:
                out[-1][1] = x
            else:
                out.append([x, x])
        return [tuple(r) for r in out]
    return {b: runs(touch[b]) for b in size}, {b: runs(write[b]) for b in size}


def variable_scratch_writes(ops):
    """Scratch stores whose word can differ between two voices of the
    machine in the same block: an index, or a branch / cstore condition
    deciding whether the store runs, that depends on voice state (loads
    from the voice or the scratch, voice fields, globals, generic reads),
    and generic writes. Fixpoint taint over the whole function (loops)."""
    voice = ('load', 'field_load', 'global_load', 'dread')
    taint = set()
    changed = True
    while changed:
        changed = False
        for o in ops:
            if o[0] in ('label', 'jmp', 'jz', 'jnz', 'ret', 'store', 'cstore', 'dwrite', 'global_store', 'field_store'):
                continue
            srcs = [x for x in sources(o) if isinstance(x, str)]
            hot = o[0] in voice or any(x in taint for x in srcs)
            for d in dests(o):
                if hot and d not in taint:
                    taint.add(d)
                    changed = True
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}
    inside = [0] * (len(ops) + 1)
    for i, o in enumerate(ops):
        if o[0] in ('jz', 'jnz') and o[1] in taint:
            t = pos[o[2]]
            lo, hi = (i + 1, t) if t > i else (t, i)
            inside[lo] += 1
            inside[hi] -= 1
    n, out = 0, []
    for i, o in enumerate(ops):
        n += inside[i]
        if o[0] == 'dwrite':
            out.append(o)
        elif o[0] in ('store', 'cstore') and o[1] in ('SX', 'SY'):
            if n > 0 or o[2] in taint or o[0] == 'cstore' and o[4] in taint:
                out.append(o)
    return out


def copy_runs(lines, runs, src, dsts, label):
    """Copy the elements of each run [lo, hi]: `src` and each of `dsts` is
    (I register, post-modify M register, words per element), the I register
    at element 0; every source word goes to every destination (a register
    listed twice advances twice). Runs of 4 or fewer are unrolled (no short
    hardware loops)."""
    def w(t):
        lines.append('    ' + t + ';')
    pos = 0
    for k, (lo, hi) in enumerate(runs):
        if lo != pos:
            for ireg, _, step in [src] + dsts:
                w(f'{ireg}=MODIFY ({ireg},{step * (lo - pos)})(NW)')
        n = hi - lo + 1
        body = [f'R0 = DM ({src[0]},{src[1]})'] + [f'DM ({di},{dm}) = R0' for di, dm, _ in dsts]
        if n <= 4:
            for _ in range(n):
                for t in body:
                    w(t)
        else:
            lines.append(f'    LCNTR = 0x{n:X} , DO (PC,{label}{k})UNTIL LCE;')
            for t in body[:-1]:
                w(t)
            lines.append(f'{label}{k}:')
            w(body[-1])
        pos = hi + 1


# ------------------------------------------------------------------ text passes
def hoist_lane_index(lines, reach=8):
    """Move a lane access's index doubling `R1 = Ra + Ra` (followed by
    `M0 = R1`) up over instructions that neither use R1/S1 nor write Ra, in
    the same straight-line run, so that pack_moves can fold `M0 = R1` and
    `M4 = S1` into the computes it passed."""
    import re
    lines = list(lines)
    dbl = re.compile(r'^R1 = (R\d+) \+ \1$')

    def instr(k):
        t = S._split(lines[k])[0].strip()
        return t[:-1] if t.endswith(';') and not t.endswith(':') else None
    for k in range(1, len(lines) - 1):
        t = instr(k)
        m = dbl.match(t) if t else None
        if not m or instr(k + 1) != 'M0 = R1':
            continue
        ra, j = m.group(1), k
        while j - 1 >= 0 and k - j < reach:
            u = instr(j - 1)
            # never become the first instruction of a run: after a label it
            # may be a hardware loop's last instruction
            if u is None or j - 2 < 0 or instr(j - 2) is None:
                break
            eff = S._effects(u)
            if eff is None or 'R1' in eff[0] or 'R1' in eff[1] or ra in eff[0]:
                break
            # the doubling sets ALU flags: never between a compare and the
            # conditional that reads them (TF conditions are not ALU flags)
            if u.startswith('IF ') and not u.startswith(('IF TF ', 'IF NOT TF ')):
                break
            j -= 1
        if j < k:
            lines.insert(j, lines.pop(k))
    return lines


# ------------------------------------------------------------------ frame
class PairFrame(S.Frame):
    """Slots are word pairs below the return words: slot k (k >= 11, in
    order of first use) is PEx at -(2k - 18), PEy one word above.
    remap_pair_frame then puts the hottest pairs nearest I6."""

    def slot(self, name):
        if name not in self.slots:
            self.slots[name] = self.next
            self.next += 1
        return -(2 * self.slots[name] - 18)

    def size(self):
        return 2 * (self.next - 11) + 4


def remap_pair_frame(parts, body, slow_labels=()):
    """Renumber pair slots by execution weight in `body` (fast_sharc's
    line_weights), the hottest at -4/-3, so that as many as possible sit
    within the 6-bit reach of multifunction instructions. A pair's PEx
    (even) and PEy (odd) words move together. The per-lane slow paths after
    the body jump back into it; they are weighted 1, not as loops."""
    starts = {l + ':' for l in slow_labels}
    cut = next((k for k, l in enumerate(body) if S._split(l)[0].strip() in starts), len(body))
    weight = S.line_weights(body[:cut]) + [1] * (len(body) - cut)
    score = defaultdict(int)
    seen = set()
    for lines, ws in [(body, weight)] + [(p, [0] * len(p)) for p in parts]:
        for k, l in enumerate(lines):
            for m in S._FRAME.finditer(S._split(l)[0]):
                n = -int(m.group(1))
                if n >= 3:                      # (-1, -2: return linkage)
                    pair = n + (n & 1)          # PEy -(2j-1) -> PEx -2j
                    seen.add(pair)
                    score[pair] += ws[k]
    order = sorted(seen, key=lambda p: (-score[p], p))
    new = {old: 4 + 2 * i for i, old in enumerate(order)}

    def move(m):
        n = -int(m.group(1))
        if n < 3:
            return m.group(0)
        pair = n + (n & 1)
        return f'DM ({-(new[pair] - (pair - n))},I6)'

    def fix(lines):
        return [S._FRAME.sub(move, l) for l in lines]
    return [fix(p) for p in parts], fix(body)


SHORT_REGION = 40      # ops; a shorter divergent region inside a loop joins at the loop exit


def guard_saves(ops):
    """Constant-index voice words written before the last entry guard: a
    failing guard returns 1 (the caller renders scalar on the same slots), so
    they are saved on entry and put back. Any other effect before a guard is
    refused."""
    last = max((i for i, o in enumerate(ops) if o[0] in ('guard_vv', 'guard_y')), default=0)
    saves = []
    for o in ops[:last]:
        if o[0] in ('store', 'cstore') and o[1] in ('SX', 'SY'):
            continue                            # the private scratch copies
        if o[0] in ('store', 'cstore') and o[1] in VOICE and isinstance(o[2], int):
            if (o[1], o[2]) not in saves:
                saves.append((o[1], o[2]))
            continue
        assert o[0] not in ('store', 'cstore', 'dwrite', 'global_store', 'field_store'), \
            ('an effect before an entry guard', o)
    return saves


def bail_branches(ops, halves=()):
    """Data branches (op indices) where lanes that disagree can still undo
    everything, so the pair returns 1 and the caller renders scalar; and
    the constant-index voice words written before the last of them (saved
    on entry, restored on that return). Walking the straight-line prefix:
    stores to the private scratch copies are invisible until the exit copy;
    a constant-index voice store is saved; anything else with an effect
    (other stores, generic writes, global or voice-field stores, lane A's
    shared x scratch) or a loop ends the prefix."""
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}
    heads = {pos[o[2] if o[0] != 'jmp' else o[1]] for i, o in enumerate(ops)
             if o[0] in ('jz', 'jnz', 'jmp') and pos.get(o[2] if o[0] != 'jmp' else o[1], i + 1) <= i}
    branches, saves, pending = set(), [], []
    for i, o in enumerate(ops):
        if i in heads or o[0] in ('hwloop', 'dwrite', 'global_store', 'field_store', 'ret'):
            break
        if o[0] in ('store', 'cstore'):
            base, idx = o[1], o[2]
            if base in ('SX', 'SY') and base not in halves:
                continue
            if base in ('Y', 'BLK', 'XP', 'EXT') and isinstance(idx, int):
                if (base, idx) not in pending and (base, idx) not in saves:
                    pending.append((base, idx))
                continue
            break
        if o[0] in ('jz', 'jnz') and o[1] != 1:
            branches.add(i)
            saves += pending
            pending = []
    return branches, saves


def rejoin_points(ops):
    """For every data branch (op index): the op index where the two lanes
    are back on one path -- the start of the branch block's immediate
    post-dominator, moved past any hardware loop around it (SIMD cannot be
    re-entered inside one) -- or None (only the exit joins). A region of
    fewer than SHORT_REGION ops inside a loop joins at that loop's exit
    instead: lanes that disagree once tend to disagree every iteration, and
    each round trip through the fallback costs more than such a region."""
    from tools.machines.translation.fast_opt import blocks
    ranges, succ, pred = blocks(ops)
    n = len(ranges)
    full = (1 << n) - 1
    pdom = [full] * n
    for b in range(n):
        if not succ[b]:
            pdom[b] = 1 << b
    changed = True
    while changed:
        changed = False
        for b in reversed(range(n)):
            if not succ[b]:
                continue
            new = full
            for t in succ[b]:
                new &= pdom[t]
            new |= 1 << b
            if new != pdom[b]:
                pdom[b], changed = new, True
    spans = []                  # hardware loops: (hwloop op, end marker)
    stack = []
    for i, o in enumerate(ops):
        if o[0] == 'hwloop':
            stack.append(i)
        elif o[0] == 'jnz' and o[1] == 1:
            spans.append((stack.pop(), i))
    pos = {o[1]: i for i, o in enumerate(ops) if o[0] == 'label'}
    back = [(pos[o[2] if o[0] != 'jmp' else o[1]], i) for i, o in enumerate(ops)
            if o[0] in ('jz', 'jnz', 'jmp') and pos.get(o[2] if o[0] != 'jmp' else o[1], i + 1) <= i]
    block_of = {}
    for b, (s0, e0) in enumerate(ranges):
        for i in range(s0, e0):
            block_of[i] = b
    out = {}
    for i, o in enumerate(ops):
        if o[0] not in ('jz', 'jnz') or o[1] == 1:
            continue
        b = block_of[i]
        strict = [q for q in range(n) if pdom[b] >> q & 1 and q != b]
        if not strict:
            out[i] = None
            continue
        p = max(strict, key=lambda q: bin(pdom[q]).count('1'))
        j = ranges[p][0]
        if j - i < SHORT_REGION:
            loops = [(t, q) for t, q in back if t <= i <= q and t <= j <= q]
            if loops:
                j = min(loops, key=lambda x: x[1] - x[0])[1] + 1
        for h, e in sorted(spans):
            if h <= i <= e or h < j <= e:
                outer = max((e2 for h2, e2 in spans if h2 <= h and e2 >= e), default=e)
                j = max(j, outer + 1) if h <= i <= e else min(j, h)
        out[i] = j if j > i else None
    return out


class LaneFrame:
    """A lane copy's view of the pair frame: its lane's word of every slot."""

    def __init__(self, frame, lane):
        self.base, self.lane = frame, lane

    def slot(self, name):
        return self.base.slot(name) + self.lane


def entry_label(name, lane, index):
    return f'.L_{name}_l{lane}_e{index}'


def rejoin_label(name, index):
    return f'.L_{name}_rejoin{index}'


NEG = {'EQ': 'NE', 'NE': 'EQ', 'LT': 'GE', 'GE': 'LT'}
# lines that leave the ALU flags alone: loads, stores, DAG transfers and
# conditional register transfers (an unconditional `Rd = Rs` is a PASS)
_NO_FLAGS = re.compile(r'^(?:IF (?:NOT )?\w+ [RS]\d+ = [RS]\d+|(?:IF (?:NOT )?\w+ )?[RS]\d+ = DM \([^)]*\)'
                       r'|(?:IF (?:NOT )?\w+ )?DM \([^)]*\) = [RS]\d+|[MI]\d+ = (?:[RS]\d+|-?\d+));$')


# ------------------------------------------------------------------ emitter
class PairEmitter(S.Emitter):
    """SIMD pair code; with `lane` = 0 or 1 instead a SISD copy of the same
    function (same register allocation and frame) computing that lane alone,
    entered at a branch where the lanes disagreed (`entries`)."""

    def __init__(self, *a, **k):
        super().__init__(*a, **k)
        self.frame = PairFrame()
        self.pframe = self.frame
        self.i5_base = None      # I5 holds lane B's voice struct
        self.slowpairs = []
        self.halves = {}         # region -> I register of lane B's half (base - 1 word)
        self.copied = None       # {'x'/'y': word runs in the private scratch} when trimmed
        self.merge = None        # merge mode: byte offset of the lane-B written marks
        self.lane = None
        self.div = []            # SIMD: (handler label, entry op index, enclosing hw loops)
        self.hwstack = []        # op indices of the enclosing hardware loops
        self.entries = set()     # lane copy: entry op indices
        self.start = 0           # lane copy: first op emitted
        self.base_name = None    # lane copy: the pair function's name
        self.ret_label = None    # lane copy: where `ret` goes
        self.swloops = []
        self.rejoin_of = {}      # data branch op index -> rejoin op index (or None)
        self.rejoin_ids = {}     # rejoin op index -> id (p:rj)
        self.bail = set()        # branches before any side effect: return 1 instead
        self.bail_saves = []     # (region, index) voice words to restore then

    def ibase(self, base):
        return super().ibase(base)

    def op(self, o):
        if self.lane is not None:
            if self.index < self.start:
                return
            # the check goes after the join's labels (the arms jump to them)
            rid = None
            if o[0] != 'label':
                j = self.index
                while j > 0 and self.ops[j - 1][0] == 'label':
                    j -= 1
                rid = next((self.rejoin_ids[q] for q in range(j, self.index + 1) if q in self.rejoin_ids), None)
            if rid is not None:
                # back on one path for the divergence(s) joining here
                skip = self.fresh('rj')
                self.w(f'R0 = DM ({self.pframe.slot("p:rj")},I6)')
                self.w(f'R1 = {rid}')
                self.w('COMP (R0,R1)')
                self.w(f'IF NE JUMP {skip}')
                self.w(f'JUMP {self.base_label(("__adone" if self.lane == 0 else "__bdone") + str(rid))}')
                self.lines.append(f'{skip}:')
                self.flags = self.pending = self.mr_holds = None
            if self.index in self.entries:
                self.lines.append(f'{entry_label(self.base_name, self.lane, self.index)}:')
                self.flags = self.pending = self.mr_holds = None
        elif self.index in self.rejoin_ids:
            self.lines.append(f'{rejoin_label(self.name, self.index)}:')
            self.flags = self.pending = self.mr_holds = None
        super().op(o)

    @property
    def uniform_counters(self):
        """Software loop counters whose every definition is a constant or
        their own decrement: the same trip count in both lanes."""
        if not hasattr(self, '_uniform'):
            defs = defaultdict(list)
            for o in self.ops:
                for d in dests(o):
                    if isinstance(d, str) and d.startswith('loop_'):
                        defs[d].append(o)
            self._uniform = {v for v, ds in defs.items()
                             if all(q[0] == 'mov' and isinstance(q[2], int) or q == ('sub', v, v, 1) for q in ds)}
        return self._uniform

    def cond_flags(self, c):
        """ALU flags of c (NE iff c != 0), unless still set by the previous
        select on c with only loads and transfers emitted since."""
        last = getattr(self, 'flags_of', None)
        if last and not S._os.environ.get('FAST_PAIR_NO_FLAGREUSE') and last[0] == c and all(_NO_FLAGS.match(S._split(l)[0].strip()) for l in self.lines[last[1]:]):
            lim = S._os.environ.get('FAST_PAIR_REUSE_LIMIT')
            self.nreuse = getattr(self, 'nreuse', 0) + 1
            if lim is None or self.nreuse <= int(lim):
                return
        rc = self.reg(c, 'R0')
        self.w(f'R0 = PASS {rc}')

    def live_regs(self, index):
        """Registers holding variables live before op `index`."""
        if not hasattr(self, '_live'):
            from tools.machines.translation.fast_opt import liveness
            ranges, _, live_out = liveness(self.ops)
            self._live = {}
            for b, (s0, e0) in enumerate(ranges):
                live = set(live_out[b])
                for i in reversed(range(s0, e0)):
                    o = self.ops[i]
                    for d in dests(o):
                        live.discard(d)
                    live |= {x for x in sources(o) if isinstance(x, str)}
                    self._live[i] = set(live)
        regs = {self.color[v] for v in self._live.get(index, ()) if v in self.color}
        return sorted(regs, key=lambda r: int(r[1:]))

    def base_label(self, tag):
        return f'.L_{self.base_name}_{tag}'

    def pair_base(self, base):
        """(I register, doubled word offset) of an interleaved region."""
        ireg, off = self.ibase(base)
        return ireg, 2 * off

    # memory -----------------------------------------------------------------
    def _shared_ptr(self, base, scratch='R2'):
        """I4 at a shared read-only table; returns ('I4', word offset of index 0)."""
        if base == 'SINE':
            return 'I3', 0
        self.w(f'{scratch} = {self.base_symbol(base)}')
        self.w(f'I4 = {scratch}')
        return 'I4', 0

    def _lane_access(self, ireg, idx, write, value=None, target=None, interleaved=True, extra=0, ireg_b=None):
        """Two conditional accesses: PEx at voice A's element, PEy at B's.
        With `ireg_b` the region is two lane halves: PEx at ireg + idx, PEy
        at ireg_b + idx (+ 1), no doubling. In a lane copy, one plain access
        at this lane's element."""
        if self.lane is not None:
            lane = self.lane
            if isinstance(idx, int):
                k = (2 * idx if interleaved and ireg_b is None else idx) + extra + lane * (interleaved or ireg_b is not None)
                self.w(f'M4 = {k}')
            else:
                ri = idx if idx in ('R0', 'R1', 'R2') else self.reg(idx, 'R1')
                if interleaved and ireg_b is None and not idx.endswith(SCALED):
                    self.w(f'R1 = {ri} + {ri}')
                    ri = 'R1'
                add = extra + lane * (interleaved or ireg_b is not None)
                if add:
                    assert ri != 'R0', idx
                    self.w(f'R0 = 0x{add & M32:X}')
                    self.w(f'R1 = {ri} + R0')
                    ri = 'R1'
                self.w(f'M4 = {ri}')
            i = ireg_b if ireg_b is not None and lane else ireg
            self.w(f'DM (M4,{i}) = {value}' if write else f'{target} = DM (M4,{i})')
            return
        if ireg_b is not None:
            if isinstance(idx, int):
                # (the same offset from both lanes' bases: one M register)
                self.w(f'M4 = {idx + extra}')
                self._lane_pair(ireg, write, value, target, ireg_b, m_a='M4')
                return
            else:
                ri = idx if idx in ('R0', 'R1', 'R2') else self.reg(idx, 'R1')
                if extra:
                    assert ri != 'R0', idx
                    self.w(f'R0 = 0x{extra & M32:X}')
                    self.w(f'R1 = {ri} + R0')
                    ri = 'R1'
                self.w(f'M0 = {ri}')
                self.w(f'M4 = S{ri[1:]}')
            self._lane_pair(ireg, write, value, target, ireg_b)
            return
        if isinstance(idx, int):
            k = (2 * idx if interleaved else idx) + extra
            ky = k if interleaved else k - 1
            # conditional accesses only through DM(Mm,Ii): see fast_toolchain
            self.w(f'M0 = {k}')
            self.w(f'M4 = {ky}')
            self._lane_pair(ireg, write, value, target)
            return
        ri = idx if idx in ('R0', 'R1', 'R2') else self.reg(idx, 'R1')
        # (R2 may hold a store value; R0 is free unless it is the index)
        assert not (extra and ri == 'R0'), (idx, extra)
        if interleaved and isinstance(idx, str) and idx.endswith(SCALED):
            if extra:
                self.w(f'R0 = 0x{extra & M32:X}')
                self.w(f'R1 = {ri} + R0')
                ri = 'R1'
            self.w(f'M0 = {ri}')
            self.w(f'M4 = S{ri[1:]}')
            self._lane_pair(ireg, write, value, target)
            return
        if interleaved:
            self.w(f'R1 = {ri} + {ri}')
            if extra:
                self.w(f'R0 = 0x{extra & M32:X}')
                self.w('R1 = R1 + R0')
            self.w('M0 = R1')
        else:
            if extra:
                self.w(f'R0 = 0x{extra & M32:X}')
                self.w(f'R1 = {ri} + R0')
                ri = 'R1'
            self.w(f'M0 = {ri}')
            self.w(f'R1 = {ri} - 1')
        self.w('M4 = S1')
        self._lane_pair(ireg, write, value, target)

    def voice_word(self, k, target=None, value=None):
        """Word k of each lane's own voice struct (I0 / I5 + 1)."""
        self._lane_access('I0', k, value is not None, value=value, target=target, ireg_b='I5')

    def mark(self, region, k=None):
        """Merge mode: after a lane-B write of `region` (SX/SY) at offset M4
        (or `k`) from its lane-B base, set its written mark (nonzero)."""
        if self.merge is None or region not in ('SX', 'SY') or self.lane == 0:
            return
        if k is not None:
            self.w(f'M4 = {k}')
        self.w(f'R0 = DM ({self.pframe.slot("p:d" + region)},I6)')
        self.w('I4 = R0')
        self.w(f'{"" if self.lane else f"IF {LANE_B} "}DM (M4,I4) = R0')

    def _lane_pair(self, ireg, write, value, target, ireg_b=None, m_a='M0'):
        """PEx's access at Ii + M0, PEy's at Ij + M4 (+ 1), Ij = ireg_b or Ii."""
        for cond, m, i in ((LANE_A, m_a, ireg), (LANE_B, 'M4', ireg_b or ireg)):
            self.w(f'IF {cond} DM ({m},{i}) = {value}' if write else f'IF {cond} {target} = DM ({m},{i})')

    def load(self, d, base, idx):
        if d in self.remat:
            return
        t = self.target(d)
        if base in self.halves:
            ireg, off = self.ibase(base)
            self._lane_access(ireg, idx, False, target=t, extra=off, ireg_b=self.halves[base])
        elif base in INTERLEAVED:
            ireg, off = self.pair_base(base)
            if isinstance(idx, int):
                self.w(f'{t} = DM ({off + 2 * idx + (self.lane or 0)},{ireg})')
            else:
                self._lane_access(ireg, idx, False, target=t, extra=off)
        else:
            ireg, off = self._shared_ptr(base)
            self._lane_access(ireg, idx, False, target=t, interleaved=False, extra=off)
        self.commit(d, t)

    def store(self, base, idx, s):
        assert base in INTERLEAVED, base
        if base in self.halves:
            ireg, off = self.ibase(base)
            r = self.reg(s, 'R2')
            if r in ('R0', 'R1'):
                self.w(f'R2 = PASS {r}')
                r = 'R2'
            self._lane_access(ireg, idx, True, value=r, extra=off, ireg_b=self.halves[base])
            self.mark(base)
            return
        ireg, off = self.pair_base(base)
        if isinstance(idx, int):
            r = self.reg(s, 'R2')
            self.w(f'DM ({off + 2 * idx + (self.lane or 0)},{ireg}) = {r}')
            self.mark(base, off + 2 * idx + (1 if self.lane else 0))
            return
        r = self.reg(s, 'R2')
        if r in ('R0', 'R1'):           # the index computation uses them
            self.w(f'R2 = PASS {r}')
            r = 'R2'
        self._lane_access(ireg, idx, True, value=r, extra=off)
        self.mark(base)

    def rematerializable(self, ops, ranges):
        return {}            # loads are lane-split here; keep them as variables

    # ops -------------------------------------------------------------------
    def _op(self, o):
        k = o[0]
        if k == 'guard_vv':
            ok = self.fresh('vv')
            self.voice_word(self.layout["virtual_voice_byte"] // 4, 'R1')
            self.w(f'R0 = 0x{o[1]:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF LT JUMP {ok}')          # both voices in range
            self.w(f'JUMP {self.L("__fail")}')
            self.lines.append(f'{ok}:')
            return
        if k == 'guard_y':
            ok = self.fresh('gy')
            kk, lo, hi, s = o[1:]
            self.voice_word(kk, 'R1')
            if s:
                self.voice_word(self.layout["virtual_voice_byte"] // 4, 'R2')
                self.w(f'R0 = 0x{s:X}')
                self.w('R2 = R2 * R0 (SSI)')
                self.w('R1 = R1 - R2')
            if lo:
                self.w(f'R0 = 0x{lo:X}')
                self.w('R1 = R1 - R0')
            self.w(f'R0 = 0x{hi - lo + 1:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF LT JUMP {ok}')
            self.w(f'JUMP {self.L("__fail")}')
            self.lines.append(f'{ok}:')
            return
        # (the counter of a software loop is data too when its trip count
        # can differ between the voices)
        data_branch = k in ('jz', 'jnz') and o[1] != 1 and o[1] not in self.uniform_counters
        if self.lane is not None:
            if self._lane_op(o):
                return
        elif k == 'hwloop':
            self.hwstack.append(self.index)
        if self.lane is None and k == 'jnz' and o[1] == 1:
            super()._op(o)
            self.hwstack.pop()
            return
        if data_branch and self.lane is None:
            # taken / not taken when both lanes agree (a SIMD jump needs its
            # condition in both PEs); otherwise the per-lane fallback
            if self.pending is not None:
                x = self.pending if k == 'jnz' else NEG[self.pending]
                self.pending = None
            else:
                if self.flags != o[1] or isinstance(o[1], int):
                    r = self.reg(o[1], 'R1')
                    self.w(f'{r} = PASS {r}')
                x = 'NE' if k == 'jnz' else 'EQ'
            agree, div = self.fresh('agree'), self.fresh('div')
            at = S._os.environ.get('FAST_PAIR_FORCE_AT')            # (test: the fallback at branch k)
            if not S._os.environ.get('FAST_PAIR_FORCE_DIVERGE') and (at is None or int(at) != len(self.div)):
                self.w(f'IF {x} JUMP {self.L(o[2])}')
                self.w(f'IF {NEG[x]} JUMP {agree}')
            self.w(f'JUMP {div}')
            self.lines.append(f'{agree}:')
            entry = self.index - 1 if self.index - 1 in self.fuse else self.index
            self.div.append((div, entry, list(self.hwstack), self.index))
            return
        if k == 'csel':
            # register transfers (they leave the ALU flags alone): the flags
            # of c serve a run of selects on the same condition
            d, c, new, old, when = o[1:]
            cond = 'NE' if when else 'EQ'
            if S._os.environ.get('FAST_PAIR_OLD_CSEL'):
                rn = self.reg(new, 'R1')
                ro = self.reg(old, 'R2')
                if ro != 'R2':
                    self.w(f'R2 = PASS {ro}')
                rc = self.reg(c, 'R0')
                self.w(f'R0 = PASS {rc}')
                self.w(f'IF {cond} R2 = PASS {rn}')
                self.commit(d, 'R2')
                self.flags_of = None
                return
            self.cond_flags(c)
            rn = self.reg(new, 'R1')
            ro = self.reg(old, 'R2')
            t = self.color.get(d, 'R2')
            # (only the conditional form is a transfer: selas assembles a
            # plain `Rd = Rs` as `Rd = PASS Rs`, which sets the flags)
            if ro == t:
                self.w(f'IF {cond} {t} = {rn}')
            elif rn == t:
                self.w(f'IF {NEG[cond]} {t} = {ro}')
            else:
                self.w(f'IF {cond} {t} = {rn}')
                self.w(f'IF {NEG[cond]} {t} = {ro}')
            if d not in self.color:
                self.commit(d, 'R2')
            if d == c:
                self.flags_of = None
            else:
                self.flags_of = (c, len(self.lines))
            return
        if k == 'cstore':
            base, idx, val, c, when = o[1:]
            assert base in INTERLEAVED and base not in self.halves and isinstance(idx, int), o
            ireg, off = self.pair_base(base)
            r = self.reg(val, 'R2')
            self.cond_flags(c)
            self.w(f'M4 = {off + 2 * idx + (self.lane or 0)}')
            self.w(f'IF {"NE" if when else "EQ"} DM (M4,{ireg}) = {r}')
            self.flags_of = (c, len(self.lines))
            return
        if k == 'field_load':
            t = self.target(o[1])
            self.voice_word(self.layout[o[2] + "_byte"] // 4, t)
            self.commit(o[1], t)
            return
        if k == 'field_store':
            r = self.reg(o[2], 'R2')
            if r in ('R0', 'R1'):
                self.w(f'R2 = PASS {r}')
                r = 'R2'
            self.voice_word(self.layout[o[1] + "_byte"] // 4, value=r)
            return
        if k == 'global_load':
            t = self.target(o[1])
            self.externs.add(o[2] + '.')
            self.w(f'R1 = {o[2]}.')
            self.w('I4 = R1')
            self.w('M4 = 0')
            self.w(f'IF {LANE_A} {t} = DM (M4,I4)')
            self.w('M4 = -1')
            self.w(f'IF {LANE_B} {t} = DM (M4,I4)')
            self.commit(o[1], t)
            return
        if k == 'global_store':
            # sequential order: voice B's store is the one that remains
            r = self.reg(o[2], 'R2')
            self.externs.add(o[1] + '.')
            self.w(f'R1 = {o[1]}.')
            self.w('I4 = R1')
            self.w('M4 = -1')
            self.w(f'IF {LANE_B} DM (M4,I4) = {r}')
            return
        super()._op(o)

    def _lane_op(self, o):
        """Lane-copy forms of the ops that differ from the SIMD code; True
        when emitted."""
        k = o[0]
        if k in ('guard_vv', 'guard_y'):
            raise AssertionError('lane copies start after the entry guards')
        if k == 'hwloop':
            # a software loop on a frame counter: the fallback can enter it
            # mid-body with the hardware count of the SIMD loop
            head = self.ops[self.index + 1]
            assert head[0] == 'label'
            self.swloops.append((self.index, head[1]))
            self.w(f'R0 = 0x{o[1]:X}')
            self.w(f'DM ({self.frame.slot(f"hw:{self.index}")},I6) = R0')
            return True
        if k == 'jnz' and o[1] == 1:
            i, head = self.swloops.pop()
            assert head == o[2], o
            slot = self.frame.slot(f'hw:{i}')
            self.w(f'R0 = DM ({slot},I6)')
            self.w('R0 = R0 - 1')
            self.w(f'DM ({slot},I6) = R0')
            self.w(f'IF NE JUMP {self.L(head)}')
            self.flags = None
            return True
        if k == 'ret':
            self.w(f'JUMP {self.ret_label}')
            return True
        if k == 'global_load':
            t = self.target(o[1])
            self.externs.add(o[2] + '.')
            self.w(f'R1 = {o[2]}.')
            self.w('I4 = R1')
            self.w(f'{t} = DM (I4,M5)')
            self.commit(o[1], t)
            return True
        if k == 'global_store':
            r = self.reg(o[2], 'R2')
            self.externs.add(o[1] + '.')
            self.w(f'R1 = {o[1]}.')
            self.w('I4 = R1')
            self.w(f'DM (I4,M5) = {r}')
            return True
        return False

    def lane_divergence(self):
        """SIMD: the per-lane fallback after a branch whose lanes disagreed.
        Leave SIMD, hand the enclosing hardware loops' counts to the lane
        copies' frame counters (both lanes' words) and pop those loops, save
        lane B's registers (the S complements), run lane A's copy from that
        branch; lane A's `ret` comes to __laneb, which loads lane B's
        registers into R and runs lane B's copy from the same branch."""
        if not self.div:
            return
        pf = self.pframe
        for n, (div, entry, loops, br) in enumerate(self.div, 1):
            self.lines.append(f'{div}:')
            if br in self.bail:
                # nothing written yet (the saved voice words put back):
                # return 1, the caller renders scalar
                self.lines.append(f'{self.name}_div{n}.:')
                self.w(f'BIT CLR MODE1 {PEYEN}')
                self.w('NOP')
                self.w(f'JUMP {self.L("__fail")}')
                continue
            self.lines.append(f'{self.name}_div{n}.:')        # global: the gate counts per branch
            self.w(f'BIT CLR MODE1 {PEYEN}')
            self.w('NOP')
            self.w(f'DM ({pf.slot("lb:R0")},I6) = R0')
            for i in reversed(loops):
                slot = pf.slot(f'hw:{i}')
                nxt = self.fresh('pop')
                self.w('R0 = CURLCNTR')
                self.w(f'DM ({slot},I6) = R0')
                self.w(f'DM ({slot + 1},I6) = R0')
                self.w(f'JUMP {nxt} (LA)')
                self.lines.append(f'{nxt}:')
            for r in self.live_regs(entry):
                k = r[1:]
                self.w(f'R0 = S{k}')
                self.w(f'DM ({pf.slot(f"lb:S{k}")},I6) = R0')
            self.w(f'R0 = {n}')
            self.w(f'DM ({pf.slot("p:div")},I6) = R0')
            j = self.rejoin_of.get(br)
            self.w(f'R0 = {self.rejoin_ids[j] if j is not None else 0}')
            self.w(f'DM ({pf.slot("p:rj")},I6) = R0')
            self.w(f'R0 = DM ({pf.slot("lb:R0")},I6)')
            self.w(f'JUMP {entry_label(self.name, 0, entry)}')
        self.lines.append(f'{self.L("__laneb")}:')
        self.lines.append(f'{self.name}_laneb.:')       # global: the gate counts divergences
        self.w(f'R0 = DM ({pf.slot("p:div")},I6)')
        stubs = []
        for n, (div, entry, loops, br) in enumerate(self.div, 1):
            if br in self.bail:
                continue
            stub = self.fresh('lb')
            stubs.append((stub, entry))
            self.w(f'R1 = {n}')                  # (p:div holds the handler's number)
            self.w('COMP (R0,R1)')
            self.w(f'IF EQ JUMP {stub}')
        for stub, entry in stubs:
            self.lines.append(f'{stub}:')
            for r in self.live_regs(entry):
                self.w(f'{r} = DM ({pf.slot(f"lb:S{r[1:]}")},I6)')
            self.w(f'JUMP {entry_label(self.name, 1, entry)}')
        if not self.rejoin_ids:
            return
        for j, rid in sorted(self.rejoin_ids.items(), key=lambda x: x[1]):
            live = self.live_regs(j)
            # lane A reached the join: keep its registers, run lane B
            self.lines.append(f'{self.L(f"__adone{rid}")}:')
            for r in live:
                self.w(f'DM ({pf.slot(f"la:{r}")},I6) = {r}')
            self.w(f'JUMP {self.L("__laneb")}')
            # lane B reached it: B's registers to S, A's back to R, SIMD on
            # again at the join
            self.lines.append(f'{self.L(f"__bdone{rid}")}:')
            for r in live:
                self.w(f'S{r[1:]} = {r}')
            for r in live:
                self.w(f'{r} = DM ({pf.slot(f"la:{r}")},I6)')
            self.w(f'BIT CLR ASTATx {BTF}')
            self.w(f'BIT SET ASTATy {BTF}')
            self.w(f'BIT SET MODE1 {PEYEN}')
            self.w('NOP')
            self.w(f'JUMP {rejoin_label(self.name, j)}')

    # generic memory sites ----------------------------------------------------
    def dread(self, d, space, addr, raw):
        if self.lane is not None:
            ra = self.reg(addr, 'R1')
            if ra != 'R1':
                self.w(f'R1 = PASS {ra}')
            done = self.fresh('ldone')
            self.w('R1 = FEXT R1 BY 0:24')
            if raw:
                nz = self.fresh('raw')
                self.w('R0 = 0xFFFF80')
                self.w('COMPU (R1,R0)')
                self.w(f'IF LT JUMP {nz}')
                self.w('R0 = 0')
                self.w(f'JUMP {done}')
                self.lines.append(f'{nz}:')
            self.lane_read_chain(space, done, self.lane)
            self.lines.append(f'{done}:')
            self.flags = None
            self.commit(d, 'R0')
            return
        ra = self.reg(addr, 'R1')
        if ra != 'R1':
            self.w(f'R1 = PASS {ra}')
        back, slow = self.fresh('back'), self.fresh('pslow')
        w = self.hint_window(space, False) if space in ('x', 'y', 'src') else None
        if S._os.environ.get('FAST_PAIR_SLOW_GENERIC'):
            w, space_fast = None, False
        else:
            space_fast = True
        fast = self.fresh('pfast')
        if w is not None:
            self._window_both(w, fast, slow)
            self.lines.append(f'{fast}:')
            self._window_load(w)
        elif space in ('x', 'y') and space_fast and (self.copied is None or len(self.copied[space]) <= 4):
            limit = self.m.scratch_x_words if space == 'x' else self.m.scratch_y_words
            if self.copied is None:
                self.w(f'R0 = 0x{limit:X}')
                self.w('COMPU (R1,R0)')
                self.w(f'IF LT JUMP {fast}')
            else:
                # only words of the private copy: both lanes inside one run
                for lo, hi in self.copied[space]:
                    self.w(f'R0 = 0x{lo:X}')
                    self.w('R2 = R1 - R0')
                    self.w(f'R0 = 0x{hi - lo + 1:X}')
                    self.w('COMPU (R2,R0)')
                    self.w(f'IF LT JUMP {fast}')
            self.w(f'JUMP {slow}')
            self.lines.append(f'{fast}:')
            self._lane_access('I1' if space == 'x' else 'I2', 'R1', False, target='R0',
                              ireg_b=self.halves.get('SX') if space == 'x' else None)
            self.w('R0 = FEXT R0 BY 0:24')
        else:
            self.w(f'JUMP {slow}')
        self.lines.append(f'{back}:')
        self.commit(d, 'R0')
        self.slowpairs.append((slow, back, 'read', space, raw, w))

    def dwrite(self, space, addr, s):
        if self.lane is not None:
            rs = self.reg(s, 'R2')
            if rs != 'R2':
                self.w(f'R2 = PASS {rs}')
            ra = self.reg(addr, 'R1')
            if ra != 'R1':
                self.w(f'R1 = PASS {ra}')
            done = self.fresh('ldone')
            self.w('R1 = FEXT R1 BY 0:24')
            self.lane_write_chain(space, done, self.lane)
            self.lines.append(f'{done}:')
            self.flags = None
            return
        rs = self.reg(s, 'R2')
        if rs != 'R2':
            self.w(f'R2 = PASS {rs}')
        ra = self.reg(addr, 'R1')
        if ra != 'R1':
            self.w(f'R1 = PASS {ra}')
        back, slow, fast = self.fresh('back'), self.fresh('pslow'), self.fresh('pfast')
        w = self.hint_window(space, True)
        self.sx24('R2', 'R2')
        if S._os.environ.get('FAST_PAIR_SLOW_GENERIC'):
            self.w(f'JUMP {slow}')
        elif w is not None:
            self._window_both(w, fast, slow)
            self.lines.append(f'{fast}:')
            self._window_store(w)
        else:
            limit = self.m.scratch_x_words if space == 'x' else self.m.scratch_y_words
            self.w(f'R0 = 0x{limit:X}')
            self.w('COMPU (R1,R0)')
            self.w(f'IF LT JUMP {fast}')
            self.w(f'JUMP {slow}')
            self.lines.append(f'{fast}:')
            self._lane_access('I1' if space == 'x' else 'I2', 'R1', True, value='R2',
                              ireg_b=self.halves.get('SX') if space == 'x' else None)
            self.mark('SX' if space == 'x' else 'SY')
        self.lines.append(f'{back}:')
        self.slowpairs.append((slow, back, 'write', space, False, w))

    def _window_both(self, w, fast, slow):
        """R1 = address - A per lane; both inside [0, B - A) -> fast, else
        R1 restored and -> the per-lane slow path."""
        name, A, B, origin0, step = w
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
        self.w(f'IF LT JUMP {fast}')
        restore = self.fresh('pr')
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
        self.w(f'JUMP {slow}')

    def _window_target(self, w):
        """(I register, word offset of window element 0, interleaved?)."""
        name, A, B, origin0, step = w
        if name == 'SINE':
            return 'I3', A - 0x148000, False
        if name in ('EXT', 'XP', 'Y'):
            ireg, off = self.ibase(name)
            return ireg, off + (A - origin0), 'voice'
        if name == 'SRC':
            self.w(f'R2 = 0x{self.source_base + 4 * (A - 0x20000):X}')
            self.w('I4 = R2')
            return 'I4', 0, False
        sym = f'source_{name[1:]}.'
        self.externs.add(sym)
        self.w(f'R2 = {sym}')
        self.w('I4 = R2')
        return 'I4', A - int(name[1:], 16), False

    def _window_load(self, w):
        ireg, off, inter = self._window_target(w)
        if inter == 'voice':
            self._lane_access(ireg, 'R1', False, target='R0', extra=off, ireg_b='I5')
        else:
            self._lane_access(ireg, 'R1', False, target='R0', interleaved=inter, extra=off)
        if w[0] != 'SRC' and not w[0].startswith('T'):
            self.w('R0 = FEXT R0 BY 0:24')

    def _window_store(self, w):
        ireg, off, inter = self._window_target(w)
        assert inter == 'voice', w
        self._lane_access(ireg, 'R1', True, value='R2', extra=off, ireg_b='I5')

    # per-lane slow paths (SISD) -----------------------------------------------
    def emit_slow(self):
        self.lane_divergence()
        for slow, back, kind, space, raw, w in self.slowpairs:
            self.lines.append(f'{slow}:')
            self.w(f'BIT CLR MODE1 {PEYEN}')
            self.w('NOP')
            if kind == 'write':
                self.w(f'DM ({self.frame.slot("p:tmp2")},I6) = R2')
            for lane in (0, 1):
                done = self.fresh(f'l{lane}done')
                if lane == 1:
                    self.w('R1 = S1')
                    if kind == 'write':
                        self.w('R2 = S2')
                self.w('R1 = FEXT R1 BY 0:24')
                if kind == 'read':
                    if raw:
                        nz = self.fresh('raw')
                        self.w('R0 = 0xFFFF80')
                        self.w('COMPU (R1,R0)')
                        self.w(f'IF LT JUMP {nz}')
                        self.w('R0 = 0')
                        self.w(f'JUMP {done}')
                        self.lines.append(f'{nz}:')
                    self.lane_read_chain(space, done, lane)
                else:
                    self.lane_write_chain(space, done, lane)
                self.lines.append(f'{done}:')
                if kind == 'read':
                    if lane == 0:
                        self.w(f'DM ({self.frame.slot("p:tmp")},I6) = R0')
                    else:
                        self.w('S0 = R0')
                        self.w(f'R0 = DM ({self.frame.slot("p:tmp")},I6)')
            self.w(f'BIT CLR ASTATx {BTF}')       # (a C call could have
            self.w(f'BIT SET ASTATy {BTF}')       # changed the lane flag)
            self.w(f'BIT SET MODE1 {PEYEN}')
            self.w('NOP')
            self.w(f'JUMP {back}')

    def _lane_scratch(self, space, lane):
        """I register whose (word lane) holds element 0 of this lane's x/y scratch."""
        if space == 'x' and 'SX' in self.halves and lane:
            return self.halves['SX']
        return 'I1' if space == 'x' else 'I2'

    def _lane_region(self, base, done, write, lane):
        """R1 = word index into a voice region (R2 = value): its lane's word."""
        ireg, off = self.ibase(base)
        self.w(f'R0 = {"I5" if lane else ireg}')
        self.w('R1 = LSHIFT R1 BY 2')
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        if off + lane:
            self.w(f'I4=MODIFY (I4,{off + lane})(NW)')
        if write:
            self.sx24('R2', 'R2')
            self.w('DM (I4,M5) = R2')
        else:
            self.w('R0 = DM (I4,M5)')
            self.w('R0 = FEXT R0 BY 0:24')
        self.w(f'JUMP {done}')

    def _lane_private(self, key, size, base, done, write, lane):
        miss = self.fresh('n')
        slot = self.pframe.slot('w:' + key) + lane
        self.w(f'R0 = DM ({slot},I6)')
        self.w('COMPU (R1,R0)')
        self.w(f'IF LT JUMP {miss}')
        self.w('R1 = R1 - R0')
        self.w(f'R0 = 0x{size:X}')
        self.w('COMPU (R1,R0)')
        restore = self.fresh('r')
        self.w(f'IF GE JUMP {restore}')
        self._lane_region(base, done, write, lane)
        self.lines.append(f'{restore}:')
        self.w(f'R0 = DM ({slot},I6)')
        self.w('R1 = R1 + R0')
        self.lines.append(f'{miss}:')

    def _lane_fault(self, lane):
        self.w('R0 = 1')
        self.w(f'DM ({self.layout["fault_byte"] // 4 + lane},{"I5" if lane else "I0"}) = R0')

    def lane_read_chain(self, space, done, lane):
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
            self._lane_fault(lane)
            self.w('R0 = 0')
            self.w(f'JUMP {done}')
            return
        nxt = self.fresh('n')
        self.range_check(0, limit, nxt)
        if self.copied is not None:
            # words outside the private copy: nothing writes them during the
            # render, so the shared scratch holds their value
            priv = self.fresh('cp')
            for lo, hi in self.copied[space]:
                skip = self.fresh('n')
                self.range_check(lo, hi + 1, skip)
                self.w(f'JUMP {priv}')
                self.lines.append(f'{skip}:')
            self.w('R0 = LSHIFT R1 BY 2')
            self.w(f'R2 = md_shared_scratch_{space}.')
            self.w('R0 = R0 + R2')
            self.w('I4 = R0')
            self.w('R0 = DM (I4,M5)')
            self.w('R0 = FEXT R0 BY 0:24')
            self.w(f'JUMP {done}')
            self.lines.append(f'{priv}:')
        half = space == 'x' and 'SX' in self.halves
        self.w(f'R0 = LSHIFT R1 BY {2 if half else 3}')
        self.w(f'R2 = {self._lane_scratch(space, lane)}')
        self.w('R0 = R0 + R2')
        self.w('I4 = R0')
        self.w(f'R0 = DM ({lane},I4)')
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
            self._lane_private('ext', 1536, 'EXT', done, False, lane)
            if space == 'x':
                self._lane_private('xp', 64, 'XP', done, False, lane)
        if space == 'y':
            ny = self.fresh('n')
            if m.private_memory:
                self._lane_private('yw', 64, 'Y', done, False, lane)
            else:
                self.range_check(0x800, 0x800 + m.words, ny)
                self.w('R0 = 0x800')
                self.w('R1 = R1 - R0')
                self._lane_region('Y', done, False, lane)
                self.lines.append(f'{ny}:')
            if not m.buffered_output:
                nb = self.fresh('n')
                self.range_check(0x100, 0x140, nb)
                self.w('R1 = FEXT R1 BY 0:5')
                self._lane_region('BLK', done, False, lane)
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
        self._lane_fault(lane)
        self.w('R0 = 0')
        self.w(f'JUMP {done}')

    def lane_write_chain(self, space, done, lane):
        m = self.m
        limit = m.scratch_x_words if space == 'x' else m.scratch_y_words
        nxt = self.fresh('n')
        self.range_check(0, limit, nxt)
        self.sx24('R2', 'R2')
        half = space == 'x' and 'SX' in self.halves
        self.w(f'R0 = LSHIFT R1 BY {2 if half else 3}')
        self.w(f'R1 = {self._lane_scratch(space, lane)}')
        self.w('R0 = R0 + R1')
        self.w('I4 = R0')
        self.w(f'DM ({lane},I4) = R2')
        if self.merge is not None and lane:
            self.w(f'DM ({self.merge // 4 + lane},I4) = R0')     # its written mark
        self.w(f'JUMP {done}')
        self.lines.append(f'{nxt}:')
        if m.buffered_output and space == 'y':
            n2 = self.fresh('n')
            self.w('R0 = 0x1FF')
            self.w('COMP (R1,R0)')
            self.w(f'IF NE JUMP {n2}')
            self.w('R0 = FEXT R2 BY 0:24')
            self.w('R1 = 0xFDB42B')
            self.w('COMP (R0,R1)')
            self.w(f'IF EQ JUMP {done}')
            self._lane_fault(lane)
            self.w(f'JUMP {done}')
            self.lines.append(f'{n2}:')
        if m.private_memory:
            self._lane_private('ext', 1536, 'EXT', done, True, lane)
            if space == 'x':
                self._lane_private('xp', 64, 'XP', done, True, lane)
        if space == 'y':
            if m.private_memory:
                self._lane_private('yw', 64, 'Y', done, True, lane)
            else:
                ny = self.fresh('n')
                self.range_check(0x800, 0x800 + m.words, ny)
                self.w('R0 = 0x800')
                self.w('R1 = R1 - R0')
                self._lane_region('Y', done, True, lane)
                self.lines.append(f'{ny}:')
            if not m.buffered_output:
                nb = self.fresh('n')
                self.range_check(0x100, 0x140, nb)
                self.w('R1 = FEXT R1 BY 0:5')
                self._lane_region('BLK', done, True, lane)
                self.lines.append(f'{nb}:')
        self._lane_fault(lane)
        self.w(f'JUMP {done}')


# ------------------------------------------------------------------ function
def merge_runs(lines, runs, d, b, a, shared, label):
    """Merge mode exit: for each element of the runs, the shared word gets
    lane B's value where B's written mark (d) is set, else lane A's (a), or
    keeps its own value when a is None (lane A wrote the shared word itself).
    d, b, a: (I register, M register, words per element) at element 0;
    shared: (I register, M register, 1)."""
    def w(t):
        lines.append('    ' + t + ';')
    pos = 0
    regs = [d, b] + ([a] if a else []) + [shared]
    for k, (lo, hi) in enumerate(runs):
        if lo != pos:
            for ireg, _, step in regs:
                w(f'{ireg}=MODIFY ({ireg},{step * (lo - pos)})(NW)')
        n = hi - lo + 1
        body = [f'R0 = DM ({d[0]},{d[1]})', f'R1 = DM ({b[0]},{b[1]})',
                f'R2 = DM ({a[0]},{a[1]})' if a else f'R2 = DM ({shared[0]},M5)',
                'R0 = PASS R0', 'IF NE R2 = PASS R1', f'DM ({shared[0]},{shared[1]}) = R2']
        if n <= 2:
            for _ in range(n):
                for t in body:
                    w(t)
        else:
            lines.append(f'    LCNTR = 0x{n:X} , DO (PC,{label}{k})UNTIL LCE;')
            for t in body[:-1]:
                w(t)
            lines.append(f'{label}{k}:')
            w(body[-1])
        pos = hi + 1


def merge_halves_simd(lines, runs, label):
    """merge_runs for lane B's contiguous x half (I0), its marks (I1) and the
    shared x scratch (I4), two words per instruction in SIMD (each PE its own
    word); an odd last word of a run in SISD. M4 = 2 on entry."""
    def w(t):
        lines.append('    ' + t + ';')
    pos = 0
    body = ['R0 = DM (I1,M4)', 'R1 = DM (I0,M4)', 'R2 = DM (I4,M5)', 'R0 = PASS R0', 'IF NE R2 = PASS R1',
            'DM (I4,M4) = R2']
    for k, (lo, hi) in enumerate(runs):
        if lo != pos:
            for ireg in ('I1', 'I0', 'I4'):
                w(f'{ireg}=MODIFY ({ireg},{lo - pos})(NW)')
        n = hi - lo + 1
        if n // 2:
            w(f'BIT SET MODE1 {PEYEN}')
            w('NOP')
            if n // 2 <= 2:
                for _ in range(n // 2):
                    for t in body:
                        w(t)
            else:
                lines.append(f'    LCNTR = 0x{n // 2:X} , DO (PC,{label}{k})UNTIL LCE;')
                for t in body[:-1]:
                    w(t)
                lines.append(f'{label}{k}:')
                w(body[-1])
            w(f'BIT CLR MODE1 {PEYEN}')
            w('NOP')
        if n % 2:
            for t in body:
                w(t.replace('M4', 'M6'))
        pos = hi + 1


def compile_pair(name, machine, ops, layout, source_base, hints=None, merge=False):
    """Assembly of the pair render (see the module docstring)."""
    env = S._os.environ.get
    ops = S.lower_generic(ops, machine)
    ops = if_convert(ops)               # (after lowering: it adds modulo-wrap branches)
    # the x scratch as two lane halves (lane B through I3) when the varying
    # accesses outweigh the constant ones; I3 is free unless the sine table
    # is read
    halves = {}
    if not env('FAST_NO_SPLIT') and 'SINE' not in repr(ops) + repr(hints):
        w = ir_weights(ops)
        var = const = 0
        for wi, o in zip(w, ops):
            if o[0] in ('load', 'store', 'cstore') and (o[2] if o[0] == 'load' else o[1]) == 'SX':
                if isinstance(o[3] if o[0] == 'load' else o[2], int):
                    const += wi
                else:
                    var += wi
            assert not (o[0] == 'cstore' and o[1] == 'SX'), o
        if var > 2 * const:
            halves['SX'] = 'I3'
    if halves:
        # lane A works on the shared x scratch: a failing entry guard must
        # find it unchanged, so every guard has to come before any store
        last_guard = max((i for i, o in enumerate(ops) if o[0] in ('guard_vv', 'guard_y')), default=0)
        if any(o[0] in ('store', 'cstore', 'dwrite', 'global_store', 'field_store') for o in ops[:last_guard]):
            halves = {}
    sr = None
    if not env('FAST_NO_TRIM'):
        from tools.machines.translation.fast_opt import analyze
        sr = scratch_runs(ops, machine, analyze(ops, machine))
    halves.update({r: 'I5' for r in VOICE})
    if not env('FAST_NO_SHADOW'):
        ops = shadow_indices(ops, machine, tuple(halves))
    ops = S.pair_multiplies(ops)
    if not env('FAST_NO_HWLOOP'):
        ops = S.hw_loops(ops)
    if not env('FAST_NO_MR'):
        ops = S.mr_fuse(ops)
    if not env('FAST_NO_PRESHIFT'):
        ops = S.preshift(ops)
    ops = S.hoist_constants(ops)
    if not env('FAST_NO_LOCALIZE'):
        # as compile_function: a split copy that would not get a register
        # keeps the shared function-wide variable
        split = S.localize_constants(ops)
        color = S.allocate(split)
        spilled = {v for o in split for v in [S.dest(o)] if isinstance(v, str) and '@' in v and v not in color}
        ops = S.localize_constants(ops, frozenset(spilled)) if spilled else split
    e = PairEmitter(name, machine, layout, ops, source_base)
    if not env('FAST_NO_REJOIN'):
        e.rejoin_of = rejoin_points(ops)
        joins = sorted({j for j in e.rejoin_of.values() if j is not None})
        e.rejoin_ids = {j: k for k, j in enumerate(joins, 1)}
    if not env('FAST_NO_BAIL'):
        # only where the lanes would not join again before the end: with a
        # join, the per-lane copies of the region are cheaper than two
        # whole scalar renders
        bail, saves = bail_branches(ops, halves)
        e.bail = {br for br in bail if e.rejoin_of.get(br) is None}
        if e.bail:
            last = max(e.bail)
            e.bail_saves = [sv for sv in saves if sv in bail_branches(ops[:last + 1], halves)[1]]
    for sv in guard_saves(ops):
        if sv not in e.bail_saves:
            e.bail_saves.append(sv)
    e.halves = halves
    nx, ny = machine.scratch_x_words, machine.scratch_y_words
    if merge:
        # lane B's written marks: a second private scratch image right after
        # the first (the caller's scratch buffer is twice as large)
        e.merge = 8 * (nx + ny)
    if sr:
        e.copied = {'x': sr[0]['SX'], 'y': sr[0]['SY']}
    e.hints = hints
    body = e.emit()
    copies = [d for d in e.div if d[3] not in e.bail]
    if copies:
        # per-lane SISD copies from the first branch that can diverge (or
        # the loop around it) to the end
        start = min(entry for _, entry, _, _ in copies)
        changed = True
        while changed:
            changed = False
            for i, o in enumerate(ops):
                tgt = o[2] if o[0] in ('jz', 'jnz') else o[1] if o[0] == 'jmp' else None
                if tgt is None or i < start:
                    continue
                t = next(j for j, q in enumerate(ops) if q == ('label', tgt))
                if t < start:
                    start = t - (t > 0 and ops[t - 1][0] == 'hwloop')
                    changed = True
        guards = [i for i, o in enumerate(ops) if o[0] in ('guard_vv', 'guard_y')]
        assert not guards or start > max(guards), 'a divergent branch before an entry guard'
        for lane in (0, 1):
            c = PairEmitter(f'{name}_l{lane}', machine, layout, ops, source_base)
            c.lane, c.base_name, c.start = lane, name, start
            c.entries = {entry for _, entry, _, _ in copies}
            c.rejoin_ids = {j: rid for j, rid in e.rejoin_ids.items()
                            if any(e.rejoin_of.get(br) == j for _, _, _, br in copies)}
            c.pframe, c.frame = e.pframe, LaneFrame(e.pframe, lane)
            c.halves, c.copied, c.hints, c.merge = e.halves, e.copied, hints, e.merge
            c.ret_label = e.L('__laneb') if lane == 0 else e.L('__exit')
            lines = c.emit()
            if env('FAST_PAIR_PROF'):
                lines.insert(0, f'{name}_prof_lane{lane}.:')
            assert c.color == e.color, 'lane copies need the SIMD register allocation'
            body += lines
            e.externs |= c.externs
    f = e.frame
    nx, ny = machine.scratch_x_words, machine.scratch_y_words
    pre, post, fail = [], [], []

    def w(lines, text):
        lines.append('    ' + text + ';')
    for k in range(8):
        w(pre, f'DM ({f.slot(f"s:R{8 + k}")},I6) = R{8 + k}')
    for k in range(16):
        w(pre, f'R0 = S{k}')
        w(pre, f'DM ({f.slot(f"s:S{k}")},I6) = R0')
    for k in range(4):
        w(pre, f'R0 = I{k}')
        w(pre, f'DM ({f.slot(f"s:I{k}")},I6) = R0')
    for m in ('M0', 'M4'):
        w(pre, f'R0 = {m}')
        w(pre, f'DM ({f.slot("s:" + m)},I6) = R0')
    for part in ('MR0F', 'MR1F', 'MR2F'):
        w(pre, f'R0 = {part}')
        w(pre, f'DM ({f.slot("s:" + part)},I6) = R0')
    w(pre, 'R0 = I5')
    w(pre, f'DM ({f.slot("s:I5")},I6) = R0')
    w(pre, f'DM ({f.slot("s:scr")},I6) = R12')
    prof_tags = []

    def prof(lines, tag):
        """FAST_PAIR_PROF: a global label per section (the profile tools
        attribute instruction counts by address range)."""
        if env('FAST_PAIR_PROF'):
            prof_tags.append(tag)
            lines.append(f'{name}_prof_{tag}.:')
    prof(pre, 'copyin')
    # private scratch: shared scratch into both lanes (the words direct
    # accesses can touch, when that is known)
    full = {'SX': [(0, nx - 1)], 'SY': [(0, ny - 1)]}
    touch, written = sr if sr else (full, full)
    # With the x scratch in halves, lane A works on the shared x scratch
    # itself (under the same contract: its writes are the ones lane B's
    # overwrite at exit) and only lane B's half is a private copy.
    w(pre, 'R0 = md_shared_scratch_x.')
    w(pre, 'I4 = R0')
    w(pre, 'I0 = R12')
    if 'SX' in halves:
        copy_runs(pre, touch['SX'], ('I4', 'M6', 1), [('I0', 'M6', 1)], e.L('cpx'))
    else:
        copy_runs(pre, touch['SX'], ('I4', 'M6', 1), [('I0', 'M6', 1), ('I0', 'M6', 1)], e.L('cpx'))
    # the y scratch: in SIMD with broadcast loads through I1 (both PEs read
    # the same word), one pair store writes both lanes
    w(pre, 'R0 = md_shared_scratch_y.')
    w(pre, 'I1 = R0')
    w(pre, f'R0 = 0x{8 * nx:X}')
    w(pre, 'R0 = R0 + R12')
    w(pre, 'I0 = R0')
    w(pre, 'M4 = 2')
    w(pre, f'BIT SET MODE1 {PEYEN} | {BDCST1}')
    w(pre, 'NOP')
    copy_runs(pre, touch['SY'], ('I1', 'M6', 1), [('I0', 'M4', 2)], e.L('cpy'))
    w(pre, f'BIT CLR MODE1 {PEYEN} | {BDCST1}')
    w(pre, 'NOP')
    w(pre, 'I0 = R4')
    w(pre, f'R0 = 0x{8 * nx:X}')
    w(pre, 'R0 = R0 + R12')
    w(pre, 'I2 = R0')
    if 'SX' in halves:
        w(pre, 'R0 = md_shared_scratch_x.')
        w(pre, 'I1 = R0')
        w(pre, 'R0 = 0x4')                  # lane B's half - 1 word
        w(pre, 'R0 = R12 - R0')
        w(pre, 'I3 = R0')
    else:
        w(pre, 'I1 = R12')
        w(pre, f'R0 = 0x{SINE_ADDR:X}')
        w(pre, 'I3 = R0')
    w(pre, 'R0 = 0x4')                      # lane B's voice struct - 1 word
    w(pre, 'R0 = R8 - R0')
    w(pre, 'I5 = R0')
    prof(pre, 'marks')
    if e.merge is not None:
        # mark bases: each region's lane-B base register + the mark offset
        # (both words of the pair slot: SIMD reads give PEy the second);
        # the marks of every word direct or generic writes can reach cleared
        for region, base in (('SX', 'I3' if 'SX' in halves else 'I1'), ('SY', 'I2')):
            slot = f.slot('p:d' + region)
            w(pre, f'R0 = {base}')
            w(pre, f'R1 = 0x{e.merge:X}')
            w(pre, 'R0 = R0 + R1')
            w(pre, f'DM ({slot},I6) = R0')
            w(pre, f'DM ({slot + 1},I6) = R0')
        # (in SIMD each store clears two adjacent words; clearing lane A's
        # unused marks too is harmless; at most one word past a run)
        w(pre, f'BIT SET MODE1 {PEYEN}')
        w(pre, 'NOP')
        w(pre, 'R1 = 0')
        w(pre, 'M4 = 2')
        for region in ('SX', 'SY'):
            if 'SX' in halves and region == 'SX':
                spans = [(lo + 1, hi + 1) for lo, hi in written[region]]
            else:
                spans = [(2 * lo, 2 * hi + 1) for lo, hi in written[region]]
            merged = []
            for a, b in spans:
                if merged and a <= merged[-1][1] + 1:
                    merged[-1][1] = max(merged[-1][1], b)
                else:
                    merged.append([a, b])
            w(pre, f'R0 = DM ({f.slot("p:d" + region)},I6)')
            w(pre, 'I4 = R0')
            at = 0
            for k, (a, b) in enumerate(merged):
                if a != at:
                    w(pre, f'I4=MODIFY (I4,{a - at})(NW)')
                pairs = (b - a + 2) // 2
                if pairs // 3:
                    pre.append(f'    LCNTR = 0x{pairs // 3:X} , DO (PC,{e.L("clr" + region)}{k})UNTIL LCE;')
                    w(pre, 'DM (I4,M4) = R1')
                    w(pre, 'DM (I4,M4) = R1')
                    pre.append(f'{e.L("clr" + region)}{k}:')
                    w(pre, 'DM (I4,M4) = R1')
                for _ in range(pairs % 3):
                    w(pre, 'DM (I4,M4) = R1')
                at = a + 2 * pairs
        w(pre, f'BIT CLR MODE1 {PEYEN}')
        w(pre, 'NOP')
    # lane flag: BTF clear in PEx, set in PEy (only BIT TST/XOR change it)
    w(pre, f'BIT CLR ASTATx {BTF}')
    w(pre, f'BIT SET ASTATy {BTF}')
    w(pre, f'BIT SET MODE1 {PEYEN}')
    w(pre, 'NOP')
    for k, (base, idx) in enumerate(e.bail_saves):
        # voice words a declining pair puts back (both lanes per pair slot)
        ireg, off = e.ibase(base)
        word = off + idx
        w(pre, f'M0 = {word}')
        w(pre, f'M4 = {word}')
        w(pre, f'IF {LANE_A} R0 = DM (M0,I0)')
        w(pre, f'IF {LANE_B} R0 = DM (M4,I5)')
        w(pre, f'DM ({f.slot(f"bs:{k}")},I6) = R0')
    if machine.private_memory:
        vv = layout['virtual_voice_byte'] // 4
        w(pre, f'M0 = {vv}')
        w(pre, f'M4 = {vv}')
        w(pre, f'IF {LANE_A} R1 = DM (M0,I0)')
        w(pre, f'IF {LANE_B} R1 = DM (M4,I5)')
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
    prof(post, 'exit')
    # exit: lane B's scratch back to the shared scratch, R0 = 0
    w(post, f'BIT CLR MODE1 {PEYEN}')
    w(post, 'NOP')
    # (the words direct stores can write; any of them not written this block
    # still holds its entry value)
    w(post, 'M4 = 2')
    w(post, f'R1 = DM ({f.slot("s:scr")},I6)')
    w(post, 'R0 = md_shared_scratch_x.')
    w(post, 'I4 = R0')
    if e.merge is not None:
        # B's value where B wrote, else A's (or the shared word lane A wrote)
        mk = e.merge
        for region, k0, inter in (('SX', 0, 'SX' not in halves), ('SY', 8 * nx, True)):
            if region == 'SY':
                w(post, 'R0 = md_shared_scratch_y.')
                w(post, 'I4 = R0')
            w(post, f'R1 = DM ({f.slot("s:scr")},I6)')          # (the merge loop uses R1)
            w(post, f'R0 = 0x{k0 + (4 if inter else 0):X}')      # lane B's element 0
            w(post, 'R0 = R0 + R1')
            w(post, 'I0 = R0')                                  # B
            w(post, f'R0 = 0x{mk:X}')
            w(post, 'R0 = R0 + R1')
            w(post, f'R2 = 0x{k0 + (4 if inter else 0):X}')
            w(post, 'R0 = R0 + R2')
            w(post, 'I1 = R0')                                  # its marks
            if inter:
                w(post, f'R0 = 0x{k0:X}')
                w(post, 'R0 = R0 + R1')
                w(post, 'I2 = R0')                              # A
            step = ('M4', 2) if inter else ('M6', 1)
            if inter:
                merge_runs(post, written[region], ('I1',) + step, ('I0',) + step, ('I2',) + step,
                           ('I4', 'M6', 1), e.L('mg' + region))
            else:
                merge_halves_simd(post, written[region], e.L('mg' + region))
    elif 'SX' in halves:
        w(post, 'I0 = R1')
        copy_runs(post, written['SX'], ('I0', 'M6', 1), [('I4', 'M6', 1)], e.L('cpbx'))
    else:
        w(post, 'I0 = R1')
        w(post, 'I0=MODIFY (I0,1)(NW)')
        copy_runs(post, written['SX'], ('I0', 'M4', 2), [('I4', 'M6', 1)], e.L('cpbx'))
    if e.merge is None:
        w(post, 'R0 = md_shared_scratch_y.')
        w(post, 'I4 = R0')
        w(post, f'R0 = 0x{8 * nx + 4:X}')
        w(post, 'R0 = R0 + R1')
        w(post, 'I0 = R0')
        copy_runs(post, written['SY'], ('I0', 'M4', 2), [('I4', 'M6', 1)], e.L('cpby'))
    w(post, 'R0 = 0')
    w(post, f'DM ({f.slot("p:ret")},I6) = R0')
    w(post, f'JUMP {e.L("__restore")}')
    fail.append(f'{e.L("__fail")}:')
    w(fail, f'BIT CLR MODE1 {PEYEN}')
    w(fail, 'NOP')
    for k, (base, idx) in enumerate(e.bail_saves):
        # put back the voice words written before the decline / failed guard
        ireg, off = e.ibase(base)
        slot = f.slot(f'bs:{k}')
        w(fail, f'R0 = DM ({slot},I6)')
        w(fail, f'DM ({off + idx},I0) = R0')
        w(fail, f'R0 = DM ({slot + 1},I6)')
        w(fail, f'DM ({off + idx + 1},I5) = R0')
    w(fail, 'R0 = 1')
    w(fail, f'DM ({f.slot("p:ret")},I6) = R0')
    restore = [f'{e.L("__restore")}:']
    prof(restore, 'restore')
    for k in range(4):
        w(restore, f'R0 = DM ({f.slot(f"s:I{k}")},I6)')
        w(restore, f'I{k} = R0')
    for m in ('M0', 'M4'):
        w(restore, f'R0 = DM ({f.slot("s:" + m)},I6)')
        w(restore, f'{m} = R0')
    for part in ('MR1F', 'MR2F', 'MR0F'):
        w(restore, f'R0 = DM ({f.slot("s:" + part)},I6)')
        w(restore, f'{part} = R0')
    w(restore, f'R0 = DM ({f.slot("s:I5")},I6)')
    w(restore, 'I5 = R0')
    for k in range(16):
        w(restore, f'R0 = DM ({f.slot(f"s:S{k}")},I6)')
        w(restore, f'S{k} = R0')
    for k in range(8):
        w(restore, f'R{8 + k} = DM ({f.slot(f"s:R{8 + k}")},I6)')
    w(restore, f'R0 = DM ({f.slot("p:ret")},I6)')
    (pre, post, fail, restore), body = remap_pair_frame((pre, post, fail, restore), body,
                                                        [p[0] for p in e.slowpairs] + [d[0] for d in e.div])
    if not S._os.environ.get('FAST_NO_FORWARD'):
        body = S.forward_spills(body)
    if not env('FAST_NO_LANEHOIST'):
        body = hoist_lane_index(body)
    if not env('FAST_NO_PACK'):
        body = S.pack(body)
        if not env('FAST_NO_PACKFAR'):
            body = S.pack_far(body)
        if not env('FAST_NO_PACKMOVE'):
            body = S.pack_moves(body)
    text = ['.EXTERN md_shared_scratch_x.;', '.EXTERN md_shared_scratch_y.;']
    for sym in sorted(e.externs):
        text.append(f'.EXTERN {sym};')
    text += ['.SECTION/SW seg_swco;', f'.GLOBAL {name}.;'] + ([f'.GLOBAL {name}_laneb.;'] if e.div else []) \
        + [f'.GLOBAL {name}_div{n}.;' for n in range(1, len(e.div) + 1)] + [f'{name}.:']
    # the frame is not on the C stack (509 words, shared with the stock call
    # chain and interrupts) but in a fixed block: I6 points at its top word,
    # which keeps the caller's I6 for the epilogue. I7 stays on the stack.
    assert f.size() <= PAIR_FRAME_WORDS - 1, (name, f.size())
    text += ['    I12=I6;', f'    I6=0x{PAIR_FRAME_TOP:08X};', '    DM (0,I6)=I12;']
    if env('FAST_PAIR_PROF'):
        cut = next((k for k, l in enumerate(body) if S._split(l)[0].strip() in
                    {d[0] + ':' for d in e.div} | {p[0] + ':' for p in e.slowpairs}), None)
        if cut is not None:
            body.insert(cut, f'{name}_prof_slow.:')
        pre.append(f'{name}_prof_body.:')
        prof_tags += ['body'] + (['slow'] if cut is not None else []) + (['lane0', 'lane1'] if copies else [])
        at = text.index('.SECTION/SW seg_swco;') + 1
        text[at:at] = [f'.GLOBAL {name}_prof_{t}.;' for t in prof_tags]
    text += pre
    text += [line.split('\x00')[0] for line in body]
    text.append(f'{e.L("__exit")}:')
    text += post + fail + restore
    text += ['    I6=DM (0,I6);', '    I12=DM (M7,I6);', '    JUMP (M14,I12) (DB);', '    RFRAME;', '    NOP;', '.ENDSEG;', '']
    return '\n'.join(text), e
