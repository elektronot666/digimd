"""selcc -O1 register check for compiled C (tools/machines/resident).

After each call returns, R3/R5/R6/R7 hold whatever the callee left
(compiled callees use them without saving). Follow the fall-through path
(through conditional branches, labels and further calls) and flag a read of
one of them before the path writes it. selcc can keep a value in one of
these registers across a call to a function that clobbers it (seen with
md_has_private after the relink's md_copy_words substitution): fast_relink
rejects any compiled object with such a read.

    python3 -m tools.machines.resident.call_clobber <file.s> ...
"""
import re, sys
REGS = {'R3', 'R5', 'R6', 'R7'}
def check(path):
    lines = open(path).read().splitlines()
    bad = []
    for i, l in enumerate(lines):
        if not re.match(r'^\.L_ret_\w+:', l):
            continue
        alive = set(REGS)
        for j in range(i + 1, min(i + 600, len(lines))):
            t = lines[j].split('/*')[0].strip()
            if not t or t.endswith(':'):
                continue
            if re.match(r'^[A-Za-z_]\w*\.:', t) or t.startswith('.'):
                break
            body = t.rstrip(';')
            if body.startswith('IF '):
                body = re.sub(r'^IF \w+ ', '', body)
            if re.match(r'^(JUMP|RTS)', body) and not body.startswith('JUMP (M14'):
                if t.startswith('IF '):
                    continue                    # conditional: follow the fall-through
                break
            if body.startswith('JUMP (M14'):
                break
            if body.startswith('CJUMP'):
                alive = set(REGS)               # a further call: its own return check covers after it
                continue
            writes, reads = set(), set()
            for p in [p.strip() for p in body.split(',')]:
                if p.startswith(('COMP', 'LCNTR', 'DO')) or '=' not in p:
                    reads |= set(re.findall(r'\bR\d+\b', p)); continue
                lhs, rhs = p.split('=', 1)
                reads |= set(re.findall(r'\bR\d+\b', rhs))
                if 'DM' in lhs or 'PM' in lhs: reads |= set(re.findall(r'\bR\d+\b', lhs))
                else: writes |= set(re.findall(r'\bR\d+\b', lhs))
            hit = reads & alive
            if hit:
                bad.append((j + 1, sorted(hit), t)); break
            alive -= writes
            if not alive:
                break
    return bad


if __name__ == '__main__':
    for p in sys.argv[1:]:
        b = check(p)
        print(p, len(b))
        for x in b[:12]:
            print('  ', x)
