"""Keep the hand-written renders inside the instruction vocabulary that has
run on the unit.

On the unit (2026-10-21) triggering P-I-SD, TRX-CP or GND-SIN stopped all
audio until reboot; the translated renders (BISECT1) play, the hand-written
ones on the last good dispatcher (BISECT4) do not, and sharcvm runs both
bit-exact. The translator (translation/fast_sharc.py VOCABULARY) packs a
memory move only with a single ALU or multiplier compute, as selcc does;
its 4,221 compute+memory instructions on the unit contain no shifter
operation. The three reported renders each contain a shifter compute with a
memory move (Type 4 'shift BY R , R = DM(n,Ii)' or Type 6 'shift BY imm ,
DM(Ii,Mm)'); renders without one were not reported.

legalize(text) splits every such instruction into the two it packs, with
the parallel semantics: a store goes first (it reads the old register), a
load second (the compute reads the old register). When the instruction ends
a hardware loop (its label is a DO end), the first part goes before the
label so both stay in the loop body.
"""
import re

SHIFTER = re.compile(r'^(R\d+) = (?:LSHIFT|ASHIFT|ROT|FEXT|FDEP|BSET|BCLR|BTGL|EXP|LEFTZ|LEFTO|FPACK|FUNPACK)\b')
LOAD = re.compile(r'^(R\d+|S\d+) ?= ?DM ?\(')
STORE = re.compile(r'^DM ?\([^)]*\) ?= ?(R\d+|S\d+)$')


def _parts(code):
    """compute, memory of 'compute , memory' (top-level comma), or None"""
    depth, cut = 0, None
    for k, ch in enumerate(code):
        depth += ch == '('
        depth -= ch == ')'
        if ch == ',' and depth == 0:
            cut = k
            break
    if cut is None:
        return None
    return code[:cut].strip(), code[cut + 1:].strip()


def legalize(text):
    lines = text.split('\n')
    do_ends = set(re.findall(r'DO \(PC,(\.L\w+)\)', text))
    targets = set(re.findall(r'\b(?:JUMP|CALL)\s+(\.L\w+)', text))
    out, n = [], 0
    for line in lines:
        code, _, comment = line.partition('/*')
        stripped = code.strip()
        if not stripped.endswith(';') or stripped.startswith(('IF ', 'LCNTR')):
            out.append(line)
            continue
        parts = _parts(stripped[:-1])
        if not parts or not SHIFTER.match(parts[0]):
            out.append(line)
            continue
        compute, memory = parts
        if STORE.match(memory):
            first, second = memory, compute
        elif LOAD.match(memory):
            assert LOAD.match(memory).group(1) != SHIFTER.match(compute).group(1), line
            first, second = compute, memory
        else:
            raise AssertionError(f'shifter compute with an unexpected move: {line}')
        tail = f' /*{comment}' if comment else ''
        label = None
        if out and re.match(r'^(\.L\w+):$', out[-1].strip()):
            label = out[-1].strip()[:-1]
        if label in do_ends:
            assert label not in targets, f'loop end that is also a branch target: {label}'
            out.insert(len(out) - 1, f'    {first};')
            out.append(f'    {second};{tail}')
        else:
            out.append(f'    {first};')
            out.append(f'    {second};{tail}')
        n += 1
    return '\n'.join(out), n


# ---- internal subroutines ------------------------------------------------
# No code that has run on the unit uses CALL/RTS (the translated renders
# return only through the C ABI's JUMP (M14,I12) (DB)). inline_calls()
# replaces each 'CALL .Lsub (DB); d1; d2;' (or 'CALL .Lsub;') by d1; d2; and
# a copy of the subroutine whose labels are renamed and whose returns jump to
# the end of the copy, keeping their delay slots: 'RTS (DB)' -> 'JUMP .Lr
# (DB)', 'RTS' -> 'JUMP .Lr', 'IF cc RTS' -> 'IF cc JUMP .Lr'. The original
# body (reached only by CALL; the code before it jumps over it) is dropped.

_LABEL = re.compile(r'^(\.L\w+):\s*$')
_CALL = re.compile(r'^\s+CALL (\.L\w+)( \(DB\))?;')
_RTS = re.compile(r'^(\s+)(IF \w+ )?RTS( \(DB\))?;(.*)$')
_BRANCH_REF = re.compile(r'\b(?:JUMP|CALL)\s+(\.L\w+)')


def _instr(line):
    code = line.split('/*')[0].strip()
    return code if code.endswith(';') else None


def _subroutine(lines, start):
    """[start, end) of the subroutine whose label is lines[start]: through
    the last unconditional return (and its delay slots) after which the
    next label is not branched to from inside the body"""
    k = start + 1
    end = None
    while k < len(lines):
        m = _RTS.match(lines[k])
        jump = re.match(r'^\s+JUMP \.L\w+( \(DB\))?;', lines[k])
        if (m and not m.group(2)) or jump:
            delayed = (m and m.group(3)) or (jump and jump.group(1))
            j, slots = k + 1, 2 if delayed else 0
            while slots and j < len(lines):
                if _instr(lines[j]):
                    slots -= 1
                j += 1
            end = j
            # next label (skipping blank/comment lines)
            q = j
            while q < len(lines) and not _instr(lines[q]) and not _LABEL.match(lines[q].strip()):
                q += 1
            nxt = _LABEL.match(lines[q].strip()).group(1) if q < len(lines) and _LABEL.match(lines[q].strip()) else None
            body_refs = set(_BRANCH_REF.findall('\n'.join(lines[start:j])))
            if m and (nxt is None or nxt not in body_refs):
                return start, end
            k = j
            continue
        k += 1
    raise AssertionError(f'subroutine {lines[start]} has no final return')


def inline_calls(text):
    total = 0
    while True:
        lines = text.split('\n')
        targets = {m.group(1) for l in lines for m in [_CALL.match(l)] if m}
        if not targets:
            return text, total
        found = {}
        for i, l in enumerate(lines):
            lab = _LABEL.match(l.strip())
            if lab and lab.group(1) in targets:
                a, b = _subroutine(lines, i)
                found[lab.group(1)] = (a, b, lines[a + 1:b])
        assert set(found) == targets, (targets, set(found))
        # leaf subroutines first (nested calls: picc/pihh/pirc)
        subs = {k: v for k, v in found.items() if not any(_CALL.match(x) for x in v[2])}
        assert subs, 'recursive subroutines'
        out, i, copy = [], 0, 0
        skip = {k for a, b, _ in subs.values() for k in range(a, b)}
        while i < len(lines):
            if i in skip:
                i += 1
                continue
            m = _CALL.match(lines[i])
            if not m or m.group(1) not in subs:
                out.append(lines[i])
                i += 1
                continue
            name, delayed = m.group(1), bool(m.group(2))
            i += 1
            if delayed:
                slots = 2
                while slots:
                    if _instr(lines[i]):
                        slots -= 1
                    out.append(lines[i])
                    i += 1
            copy += 1
            tag = f'_i{copy}'
            ret = f'{name}_ret{tag}'
            labels = {x.group(1) for x in (_LABEL.match(l.strip()) for l in subs[name][2]) if x}
            out.append(f'    /* (inlined {name}) */')
            for l in subs[name][2]:
                for lab in labels:
                    l = re.sub(re.escape(lab) + r'\b', lab + tag, l)
                r = _RTS.match(l)
                if r:
                    l = f'{r.group(1)}{r.group(2) or ""}JUMP {ret}{r.group(3) or ""};{r.group(4)}'
                out.append(l)
            out.append(f'{ret}:')
            total += 1
        text = '\n'.join(out)
