"""Hardware-loop rules sharcvm does not enforce, for the hand-written renders:
no loop body shorter than three instructions, no branch in a body's last
three instructions. fast_relink rejects a render that breaks one.

    python3 -m tools.machines.handwritten.loop_check <file.s> ...
"""
import re, sys
def instrs(lines):
    out = []
    for i, l in enumerate(lines):
        t = l.split('/*')[0].strip()
        if not t or t.startswith('*'): continue
        if re.match(r'^[.\w]+:$', t): out.append(('LABEL', t[:-1], i)); continue
        if t.startswith('.'): continue
        out.append(('I', t, i))
    return out
def check(path):
    L = open(path, errors='replace').read().splitlines()
    ins = instrs(L)
    issues = []
    for k, (kind, t, i) in enumerate(ins):
        m = re.search(r'DO \(PC,(\.L\w+)\)', t) if kind == 'I' else None
        if not m: continue
        end = m.group(1)
        # body: instructions after k up to and including the one after label end
        body = []
        j = k + 1
        while j < len(ins):
            if ins[j][0] == 'LABEL' and ins[j][1] == end:
                j += 1
                while j < len(ins) and ins[j][0] == 'LABEL': j += 1
                body.append(ins[j]); break
            if ins[j][0] == 'I': body.append(ins[j])
            j += 1
        n = len(body)
        if n < 3: issues.append((path, i + 1, f'loop of {n} instructions'))
        for idx, (_, bt, bi) in enumerate(body[-3:]):
            pos = n - min(3, n) + idx
            if re.search(r'\b(JUMP|CALL|RTS|RTI)\b', bt):
                issues.append((path, bi + 1, f'branch at loop position {pos + 1}/{n}: {bt}'))
    return issues
if __name__ == '__main__':
    files = sys.argv[1:]
    found = [x for f in files for x in check(f)]
    print(len(found), 'issues in', len(files), 'files')
    for x in found[:30]:
        print('  ', x)
