"""Rebuild the Full49 renderer with the fast SHARC+ render for the 47
translated engines.

Base: the qualified Full49 routing profile. Every object is reused unchanged
except, for each translated engine, its C object (recompiled with the C render
renamed to `<prefix>_render_block_44k1_c`, kept as the reference fallback) and
the new assembly render `<prefix>_render_block_44k1` (plus, when the engine
has entry guards, the unguarded fast fallback `<prefix>_render_block_44k1_g`).
TRX-BD's scratch object is recompiled so every shared-scratch word it writes
is a canonical sign-extended 24-bit word (an assumption the fast code uses).
The link and every post-link step are those of `tools.machines.resident.native`.

Pairing (SIMD, two voices per instruction stream): every pairable engine also
gets `<prefix>_render_pair` (fast_pair) and the split of its slot process
(tools/machines/slot_split.inc: begin / run / render / rendered, around an
`engine_slot.inc` whose slot process is factored into `_pre` + the 48 kHz
render); the descriptors carry them and the dispatcher (dispatch49.c with
MD_HAS_PAIRING) is recompiled.

    python3 -m tools.machines.resident.fast_relink --out out/audio-fix-full49-fast
"""
import argparse
import os as _os
import re
import json
from pathlib import Path
import shutil
import subprocess

from tools.machines.registry import ROOT
from tools.machines.compact.build import sha
from tools.machines.resident import native as N
from tools.machines.resident import native_support as O
from tools.machines.resident import fast_placement as P
from tools.machines.resident.relink import LINKER
from tools.machines.translation import fast_check as F
from tools.machines.translation.fast_native import layout
from tools.machines.translation.fast_sharc import compile_function
from tools.machines.translation.fast_profile import hints
from tools.trxbd_sharc import build_sharc as B

HERE = Path(__file__).resolve().parent
HAND = HERE.parent / 'handwritten'
SOURCE_BASE = 0x81000000
from tools.machines.translation.fast_pair import PAIR_FRAME, PAIR_FRAME_WORDS
PAIR_SCRATCH, PAIR_SCRATCH_BYTES = 0x8183b000, 0x2800   # dispatch49.c MD_PAIR_SCRATCH
assert PAIR_FRAME == PAIR_SCRATCH + PAIR_SCRATCH_BYTES     # fast_pair: the pair render's frame follows
# the dispatcher's descriptor table (seg_l2), outside the full 64 KB data segment
MD_TABLES = (PAIR_FRAME + 4 * PAIR_FRAME_WORDS, PAIR_FRAME + 4 * PAIR_FRAME_WORDS + 0x1000)
# the hand-written renders' 256-word scratch block (@TMP@ in handwritten/*.s)
HAND_SCRATCH, HAND_SCRATCH_BYTES = MD_TABLES[1], 0x400
MD_TABLES_LDF = (f'   mem_md_tables {{ TYPE(BW RAM) START(0x{MD_TABLES[0]:08x}) END(0x{MD_TABLES[1] - 1:08x}) WIDTH(8) }}\n',
                 '      md_tables BW\n      {\n         INPUT_SECTION_ALIGN(4)\n'
                 '         INPUT_SECTIONS( $OBJECTS(seg_md_tables) )\n      } > mem_md_tables\n')


def tables_section(asm):
    assert asm.count('.SECTION/DOUBLE32 seg_l2;') == 1
    return asm.replace('.SECTION/DOUBLE32 seg_l2;', '.SECTION/DOUBLE32 seg_md_tables;')
LEGACY_ROOT = str(ROOT) + '/'


def localize(text):
    return text.replace(LEGACY_ROOT, str(ROOT) + '/')


def fast_render(m, instructions, img):
    """Assembly texts for the engine's fast render (specialized first)."""
    machine, ops = F.lift(m, instructions, img)
    p = m['prefix']
    texts = []
    generic = None
    if machine.generic_ops is not None:
        generic = p + '_render_block_44k1_g'
        g, _ = compile_function(generic, machine, machine.generic_ops, layout(m), SOURCE_BASE,
                                fallback=p + '_render_block_44k1_c')
        texts.append(g)
    text, e = compile_function(p + '_render_block_44k1', machine, ops, layout(m), SOURCE_BASE,
                               fallback=p + '_render_block_44k1_c', generic=generic, hints=hints(m, machine, ops))
    return [text] + texts, machine


def drop_render(m, text):
    """Remove the C render (and its private helpers) of a non-private engine:
    the assembly render is complete without a reference fallback."""
    import re
    p = m['prefix']
    start = text.index(f'#ifndef {p.upper()}_RENDER_BLOCK_44K1_EXEC_DEFINED')
    end = re.search(r'\n(?:void|uint32_t) ' + p + r'_compute_coeffs\(', text).start() + 1
    return text[:start] + f'extern void {p}_render_block_44k1({m["voice_type"]} *v, const int32_t *sine);\n' + text[end:]


def rename_render(m, text):
    p = m['prefix']
    old = f'void {p}_render_block_44k1({m["voice_type"]} *v, const int32_t *sine) {{'
    assert text.count(old) == 1, m['name']
    return text.replace(old, f'extern void {p}_render_block_44k1({m["voice_type"]} *v, const int32_t *sine);\n'
                        f'void {p}_render_block_44k1_c({m["voice_type"]} *v, const int32_t *sine) {{')


SLOT_PROCESS_HEAD = ('void MD_SLOT_PROCESS(void *memory, uint32_t slot, uint32_t trig,\n'
                     '                     const int32_t *sine, float *out) {\n')


# machine-change work: the dispatcher's word loops (track history save and
# restore, slot and history clears) as calls to md_words.s
# Each call is its function's last action: selcc can keep a value in R3/R5-R7
# across a call that clobbers it (call_clobber.py rejects that). The y copy
# (at most 64 words) and the clear inside md_generic_process stay C.
WORD_LOOPS = [
    # restore: private machines (selectors 45-53) have no selector 5/6
    # fix-ups, so the copy can end the function
    ('        offset=n+45u;\n'
     '        for(i=0u;i<1600u;i++)state[offset+i]=h->private_words[i];\n'
     '        state[offset+1600u]=track;\n'
     '    }',
     '        offset=n+45u;\n'
     '        state[offset+1600u]=track;\n'
     '        md_copy_words(state+offset,h->private_words,1600u);\n'
     '        return 1u;\n'
     '    }'),
    ('        offset=n+45u;\n'
     '        for(i=0u;i<1600u;i++)h->private_words[i]=state[offset+i];\n'
     '    }',
     '        offset=n+45u;\n'
     '        md_copy_words(h->private_words,state+offset,1600u);\n'
     '        return 1u;\n'
     '    }'),
    # y[64] and private_words[1600] are adjacent
    ('    for(k=0u;k<64u;k++)h->y[k]=0u;\n'
     '    for(k=0u;k<1600u;k++)h->private_words[k]=0u;\n'
     '    h->before=HISTORY_GUARD;h->after=HISTORY_GUARD;',
     '    h->before=HISTORY_GUARD;h->after=HISTORY_GUARD;\n'
     '    md_zero_words(h->y,1664u);'),
    ('    for(k=0u;k<MD_SLOT_WORDS;k++)slot->state[k]=0u;\n'
     '    slot->owner=MD_NO_OWNER;',
     '    slot->owner=MD_NO_OWNER;\n'
     '    md_zero_words(slot->state,MD_SLOT_WORDS);'),
]
# governor: a track's first trigger after a machine change is costed at
# that machine's trigger cost measured on another track (when there is one),
# not at period/8: 16 tracks selecting P-I-SD at once otherwise admit three
# per block and cut the voices that just started
FIRST_TRIGGER = (
    '''            if(pool->slots[track].owner!=((track<<8)|selector)||((r->policy&POLICY_CLEAN)!=0u&&(r->fresh&bit)!=0u))
                need=need+(q->change_cost!=0u?q->change_cost:r->period>>4);''',
    '''            if(pool->slots[track].owner!=((track<<8)|selector)||((r->policy&POLICY_CLEAN)!=0u&&(r->fresh&bit)!=0u)) {
                if(trig&&selector>=5u&&selector-5u<MD_SELECTORS&&r->machine_trig[selector-5u]!=0u)
                    need=r->machine_trig[selector-5u];
                need=need+(q->change_cost!=0u?q->change_cost:r->period>>4);
            }''')
# governor: a trigger waiting for budget ranks with the sounding tracks by
# note age (newest first) instead of above all of them: a burst of first
# triggers (16 tracks selecting a machine at once) no longer cuts the voices
# of the same burst that already started
WAITING_RANK = ('                    k=trig?0u:((q->pending&bit)!=0u?1u:2u);',
                '                    k=trig?0u:2u;')
# governor: a machine never rendered since boot is costed at period/16
# (41.7k), not period/8 (83k), for its first trigger. Every render is
# hand-written here (trigger blocks ~10-12k emulator instructions); at 83k a
# first trigger never fits a budget under ~100k, so its cost is never learned
# and every note of it is dropped.
BLIND_TRIGGER = ('        if(need==0u)need=r->cost[track]>(r->period>>3)?r->cost[track]:r->period>>3;',
                 '        if(need==0u)need=r->cost[track]>(r->period>>4)?r->cost[track]:r->period>>4;')
WORD_PROTOTYPES = ('extern void md_copy_words(void *dst,const void *src,uint32_t words);\n'
                   'extern void md_zero_words(void *dst,uint32_t words);\n')


# the P-I engines' voice init clears its private and external words
# (adjacent), as its last action
INIT_ZERO = ('for (i=0u;i<64u;i++) v->x_private[i] = 0; for (i=0u;i<1536u;i++) v->external[i] = 0; '
             'v->virtual_voice = 0u; v->scratch_y20_history = 0;\n\n}',
             'v->virtual_voice = 0u; v->scratch_y20_history = 0; md_zero_words(v->x_private,1600u);\n\n}')


def init_loops(text):
    if INIT_ZERO[0] not in text:
        return text
    assert text.count(INIT_ZERO[0]) == 1
    # declared just before the init function (after the engine's includes)
    at = text.rindex('\nvoid ', 0, text.index(INIT_ZERO[0])) + 1
    text = text[:at] + 'extern void md_zero_words(void *dst,uint32_t words);\n' + text[at:]
    return text.replace(*INIT_ZERO)


def word_loops(text):
    for old, new in WORD_LOOPS + [FIRST_TRIGGER, WAITING_RANK, BLIND_TRIGGER]:
        assert text.count(old) == 1, old
        text = text.replace(old, new)
    marker = '@LAYOUT_CONSTANTS@'
    assert text.count(marker) == 1
    return text.replace(marker, WORD_PROTOTYPES + marker)


def split_engine_slot(text):
    """engine_slot.inc with MD_SLOT_PROCESS = MD_SLOT_PRE + MD_RENDER48 (same
    statements; slot_split.inc builds on MD_SLOT_PRE)."""
    assert text.count(SLOT_PROCESS_HEAD) == 1
    text = text.replace(SLOT_PROCESS_HEAD, '#define MD_SPLIT_CAT2(a, b) a##b\n#define MD_SPLIT_CAT(a, b) MD_SPLIT_CAT2(a, b)\n'
                        '#define MD_SLOT_PRE MD_SPLIT_CAT(MD_SLOT_PROCESS, _pre)\n'
                        'void MD_SLOT_PRE(void *memory, uint32_t slot, uint32_t trig, const int32_t *sine) {\n')
    tail = '    MD_RENDER48(v, sine, out);\n}'
    assert text.count(tail) == 1
    return text.replace(tail, '}\n' + SLOT_PROCESS_HEAD + '    MD_SLOT_PRE(memory, slot, trig, sine);\n'
                        '    MD_RENDER48((MD_VOICE_TYPE *)memory, sine, out);\n}')


def add_split(m, text, output):
    """The engine's slot functions for the dispatcher's pair renders."""
    import re
    p = m['prefix']
    inc = re.findall(r'#include "([^"]*/engine_slot\.inc)"', text)
    assert len(inc) == 1, m['name']
    text = text.replace(f'#include "{inc[0]}"', f'#include "{output / "engine_slot.inc"}"')
    # slot_split.inc mirrors MD_RENDER48 without the quiet path
    assert re.search(r'#define MD_DISABLE_QUIET_PATH\n#include "[^"]*render48\.inc"', text), m['name']
    return text + (f'\n#define MD_RESAMPLE_RUN {p}_resample_run\n#include "{ROOT / "tools/machines/slot_split.inc"}"\n'
                   '#undef MD_RESAMPLE_RUN\n')


DESCRIPTOR = """typedef struct {
    uint32_t md_id;
    uint32_t state_bytes;
    MdInit init;
    MdProcess process;
} MdDescriptor;"""
PAIR_DESCRIPTOR = """typedef uint32_t (*MdBegin)(void *, uint32_t, uint32_t, const int32_t *, float *);
typedef uint32_t (*MdRun)(void *, float *, uint32_t);
typedef void (*MdRender)(void *, const int32_t *);
typedef void (*MdRendered)(void *);
typedef uint32_t (*MdPair)(void *, void *, uint32_t *);
/* begin/run/render/rendered: the slot process split for pairing
 * (slot_split.inc); pair: <prefix>_render_pair, 0 when not pairable */
typedef struct {
    uint32_t md_id;
    uint32_t state_bytes;
    MdInit init;
    MdProcess process;
    MdBegin begin;
    MdRun run;
    MdRender render;
    MdRendered rendered;
    MdPair pair;
} MdDescriptor;
#define MD_HAS_PAIRING 1"""


def pair_descriptors(text, pairable):
    """descriptors.h with the pairing entries (each row of the base table
    extended: the split slot functions and the pair render, or zeros)."""
    import re
    lines = []
    for line in text.splitlines():
        row = re.fullmatch(r'\{(\d+)u,(\d+)u,(\w+)_slot_init,(\w+)_slot_process\},', line)
        if row:
            p = row.group(4)
            if p in pairable:
                line = (f'{{{row.group(1)}u,{row.group(2)}u,{p}_slot_init,{p}_slot_process,{p}_slot_process_begin,'
                        f'{p}_slot_process_run,{p}_slot_process_render,{p}_slot_process_rendered,{p}_render_pair}},')
            else:
                line = f'{{{row.group(1)}u,{row.group(2)}u,{p}_slot_init,{p}_slot_process,0,0,0,0,0}},'
        lines.append(line)
    decl = []
    for p in sorted(pairable):
        decl += [f'extern uint32_t {p}_slot_process_begin(void *,uint32_t,uint32_t,const int32_t *,float *);',
                 f'extern uint32_t {p}_slot_process_run(void *,float *,uint32_t);',
                 f'extern void {p}_slot_process_render(void *,const int32_t *);',
                 f'extern void {p}_slot_process_rendered(void *);',
                 f'extern uint32_t {p}_render_pair(void *,void *,uint32_t *);']
    at = next(k for k, l in enumerate(lines) if l.startswith('const uint32_t md_param_offsets'))
    return '\n'.join(lines[:at] + decl + lines[at:]) + '\n'


def build(base, output, linker=LINKER, hand_renders=True, substitutions=True, only_hand=None, hand_files=None):
    """hand_renders / substitutions off: bisection builds (translated renders without
    SIMD pairs; dispatch49.c as is)"""
    HAND_ON = hand_renders

    def hand_path(stem):
        # bisection: hand-written renders only for only_hand, from hand_files
        if not HAND_ON or (only_hand and stem not in only_hand):
            return HAND / 'none.s'
        return Path((hand_files or {}).get(stem, HAND / f'{stem}.s'))
    base = Path(base).resolve(); output = Path(output).resolve()
    old_proof = json.loads((base / 'build-proof.json').read_text())
    old_objects = json.loads((base / 'objects.json').read_text())
    output.mkdir(parents=True, exist_ok=True)
    for name in ('dispatch.h', 'descriptors.h', 'engine_slot.inc', 'linker.json', 'profile.ldf'):
        shutil.copyfile(base / name, output / name)
    from tools.machines.translation.fast_pair import PAIR_UNPAIRABLE, PAIR_MERGE, compile_pair
    (output / 'engine_slot.inc').write_text(split_engine_slot((output / 'engine_slot.inc').read_text()))
    header = (output / 'dispatch.h').read_text()
    assert header.count(DESCRIPTOR) == 1
    (output / 'dispatch.h').write_text(header.replace(DESCRIPTOR, PAIR_DESCRIPTOR))
    from tools.machines.translation.dsp import decode
    instructions = decode(F.OUT / 'decoded.tsv')
    img = F.image()
    from tools.machines.translation.fast_toolchain import selas
    fast_selas = selas()
    rows = {m['prefix']: m for m in F.machines()}
    assert len(rows) == 47
    actual = dict(old_proof['actual_compilation'])
    log = (output / 'build.log').open('w')

    def run(cmd):
        # selcc can report an unsupported construct and still exit 0 with a
        # truncated assembly file: treat any diagnostic as a failure
        r = subprocess.run([str(B.SELACHE_BIN / cmd[0]), *map(str, cmd[1:])], check=True, capture_output=True, text=True)
        log.write(r.stdout + r.stderr)
        if (r.stdout + r.stderr).strip():
            log.flush()
            raise SystemExit(f'{cmd[0]} reported: {(r.stdout + r.stderr).strip()[:300]}')

    def compile_c(name, source, include, post=None):
        asm = output / (name + '.s'); obj = output / (name + '.doj')
        run(['selcc', '-proc', 'ADSP-21569', '-char-size-8', '-O1', '-I' + str(include), '-S', '-o', asm, source])
        if post:
            asm.write_text(post(asm.read_text()))
        from tools.machines.resident.call_clobber import check
        clobbered = check(asm)
        if clobbered:
            raise SystemExit(f'{asm.name}: value kept in a call-clobbered register: {clobbered[:3]}')
        run(['selas', '-proc', 'ADSP-21569', '-o', obj, asm])
        actual[name] = dict(source_sha256=sha(source), assembly_sha256=sha(asm), object_sha256=sha(obj))
        return obj

    def pair_objects(m):
        """<prefix>_render_pair (fast_pair), unless the machine is unpairable."""
        if m['name'] in PAIR_UNPAIRABLE:
            return []
        machine, ops = F.lift(m, instructions, img)
        text, _ = compile_pair(m['prefix'] + '_render_pair', machine, ops, layout(m), SOURCE_BASE,
                               hints=hints(m, machine, ops), merge=m['name'] in PAIR_MERGE)
        s = output / f'{m["prefix"]}_pair.s'
        s.write_text(text)
        obj = s.with_suffix('.doj')
        r = subprocess.run([str(fast_selas), '-proc', 'ADSP-21569', '-o', str(obj), str(s)], capture_output=True, text=True)
        log.write(r.stdout + r.stderr)
        if r.returncode or (r.stdout + r.stderr).strip():
            raise SystemExit(f'selas {s.name}: {(r.stdout + r.stderr).strip()[:300]}')
        actual[f'{m["prefix"]}_pair'] = dict(assembly_sha256=sha(s), object_sha256=sha(obj))
        pairable.add(m['prefix'])
        return [obj]
    pairable = set()
    objects = []
    report = {}
    for rel, digest in old_objects.items():
        path = ROOT / rel
        if sha(path) != digest:
            raise SystemExit(f'Reused object changed: {rel}')
        stem = path.stem
        if stem in ('trxbd_scratch', 'trxsd_scratch'):
            # the native C ports: recompiled with the relink's fix-up and,
            # with a hand-written render, their C render renamed as its
            # decline target
            from tools.machines.handwritten.native_source import hand_source, scratch_source
            q = stem[:5]
            hand = hand_path(q)
            text = scratch_source(q, localize(path.with_suffix('.c').read_text()))
            if hand.exists():
                text = hand_source(q, text)
            source = output / f'{stem}.c'
            source.write_text(text)
            objects.append(compile_c(stem, source, path.parent, O.export_bd_decay if q == 'trxbd' else None))
            if hand.exists():
                s = output / f'{q}_fast0.s'
                s.write_text(hand.read_text().replace('@SRC@', f'0x{SOURCE_BASE - 0x80000:X}').replace(
                    '@TMP@', f'0x{HAND_SCRATCH:X}'))
                obj = s.with_suffix('.doj')
                subprocess.run([str(fast_selas), '-proc', 'ADSP-21569', '-o', str(obj), str(s)], check=True,
                               stdout=log, stderr=log)
                actual[f'{q}_fast0'] = dict(assembly_sha256=sha(s), object_sha256=sha(obj), handwritten=True)
                objects.append(obj)
                report[{'trxbd': 'TRX-BD', 'trxsd': 'TRX-SD'}[q]] = dict(handwritten=str(hand.relative_to(ROOT)), functions=1)
                print('hand-written render', q, flush=True)
            continue
        if stem in rows:
            m = rows[stem]
            text = localize(path.with_suffix('.c').read_text())
            import re
            wrapper = re.search(r'void ' + stem + r'_render_block_44k1\([^)]*\) \{\n    (\w+)_render_block_44k1\(v, sine\);\n\}',
                                text)
            if wrapper and wrapper.group(1) != stem:
                # E12 member sharing another engine's render (source-equal):
                # its wrapper calls that engine's assembly render; recompiled
                # only for the pairing functions
                source = output / f'{stem}.c'
                source.write_text(add_split(m, text, output))
                objects.append(compile_c(stem, source, path.parent))
                # no translated SIMD pair render: no SIMD on the unit
                # (handwritten/sisd.py); its host's render runs per voice
                report[m['name']] = dict(shared_render=wrapper.group(1), pair=False)
                continue
            marker = f'#ifndef {stem.upper()}_RENDER_BLOCK_44K1_EXEC_DEFINED' in text
            hand = hand_path(stem)
            # a hand-written render declines to the C reference render
            # (<p>_render_block_44k1_c) in states it does not cover: keep it
            keep_c = m.get('private_memory') or hand.exists()
            text = drop_render(m, text) if marker and not keep_c else rename_render(m, text)
            text = init_loops(text) if substitutions else text
            source = output / f'{stem}.c'
            source.write_text(add_split(m, text, output))
            objects.append(compile_c(stem, source, path.parent))
            hand = hand_path(stem)
            if hand.exists():
                # hand-written render (tools/machines/handwritten): no pair
                # render, it is cheaper than the translated pair per voice
                # no SIMD on the unit (handwritten/sisd.py)
                assert 'MODE1 0x200000' not in hand.read_text() and 'PEYEN' not in hand.read_text(), hand
                if not _os.environ.get('FULL49_HAND_NOGUARD'):   # (unset except for bisection probes)
                    from tools.machines.handwritten.loop_check import check as loop_check
                    assert not loop_check(hand), loop_check(hand)
                    from tools.machines.handwritten.legalize import legalize
                    assert legalize(hand.read_text())[1] == 0, f'{hand}: shifter compute with a memory move'
                    code = re.sub(r'/\*.*?\*/', '', hand.read_text(), flags=re.S)
                    # the caller's circular buffers off for the render (handwritten/lregs.py)
                    # (a render that updates no pointer, e.g. handwritten/strict, needs none)
                    if re.search(r'DM \(I[0-5],M\d\)|I[0-5]=MODIFY', code):
                        assert all(f'L{k} = R0;' in code for k in range(6)), f'{hand}: L0-L5 not zeroed'
                    assert not re.search(r'\b(CALL \.L|RTS)\b', code), f'{hand}: internal subroutine'
                    # the accumulator only in the translated renders' forms
                    # (handwritten/mulforms.py): no MRB, no MRF = 0, and every
                    # multiplier operation stepped in sharcvm for this very file
                    assert not re.search(r'\bMR[012]?B\b', code), f'{hand}: background accumulator'
                    assert 'MRF = 0;' not in code, f'{hand}: MRF = 0'
                    forms = json.loads((ROOT / 'out/machines/handwritten/mulforms-check.json').read_text())
                    entry = forms.get(m['name'], {})
                    assert entry.get('sha256') == sha(hand), f'{hand}: no mulforms check of this file'
                    assert entry.get('outside') == 0, f'{hand}: {entry.get("outside")} operations outside the translated forms'
                s = output / f'{stem}_fast0.s'
                s.write_text(hand.read_text().replace('@SRC@', f'0x{SOURCE_BASE - 0x80000:X}').replace(
                    '@TMP@', f'0x{HAND_SCRATCH:X}'))
                obj = s.with_suffix('.doj')
                subprocess.run([str(fast_selas), '-proc', 'ADSP-21569', '-o', str(obj), str(s)], check=True,
                               stdout=log, stderr=log)
                actual[f'{stem}_fast0'] = dict(assembly_sha256=sha(s), object_sha256=sha(obj), handwritten=True)
                objects.append(obj)
                report[m['name']] = dict(handwritten=str(hand.relative_to(ROOT)), functions=1)
                print('hand-written render', m['name'], flush=True)
                continue
            # no translated SIMD pair render: no SIMD on the unit (handwritten/sisd.py)
            asms, machine = fast_render(m, instructions, img)
            for n, asm in enumerate(asms):
                s = output / f'{stem}_fast{n}.s'
                s.write_text(asm)
                obj = s.with_suffix('.doj')
                subprocess.run([str(fast_selas), '-proc', 'ADSP-21569', '-o', str(obj), str(s)], check=True,
                               stdout=log, stderr=log)
                actual[f'{stem}_fast{n}'] = dict(assembly_sha256=sha(s), object_sha256=sha(obj))
                objects.append(obj)
            report[m['name']] = dict(guards=[list(g) for g in machine.guards], functions=len(asms))
            print('fast render', m['name'], flush=True)
            continue
        objects.append(path)
    (output / 'descriptors.h').write_text(pair_descriptors((output / 'descriptors.h').read_text(), pairable))
    from tools.machines.resident.relink import layout_constants
    template = (HERE / 'dispatch49.c').read_text()
    template = word_loops(template) if substitutions else template
    (output / 'dispatch.c').write_text(template.replace('@LAYOUT_CONSTANTS@', layout_constants(base / 'dispatch.c')))
    dispatch = compile_c('dispatch', output / 'dispatch.c', output, tables_section)
    words = output / 'md_words.doj'
    run(['selas', '-proc', 'ADSP-21569', '-o', words, HERE / 'md_words.s'])
    actual['md_words'] = dict(assembly_sha256=sha(HERE / 'md_words.s'), object_sha256=sha(words))
    objects.append(words)
    objects = [dispatch if p.name == 'dispatch.doj' else p for p in objects]
    assert dispatch in objects
    ldf, placement = P.plan(objects, (base / 'profile.ldf').read_text())
    ldf = ldf.replace('MEMORY\n{\n', 'MEMORY\n{\n' + MD_TABLES_LDF[0], 1)
    ldf = ldf.replace('      trxbd_data BW', MD_TABLES_LDF[1] + '      trxbd_data BW', 1)
    assert 'mem_md_tables' in ldf and 'seg_md_tables' in ldf
    (output / 'profile.ldf').write_text(ldf)
    (output / 'fast-placement.json').write_text(json.dumps(placement, indent=2) + '\n')
    subprocess.run([str(linker), '-proc', 'ADSP-21569', '-T', str(output / 'profile.ldf'), '-o', str(output / 'profile.dxe'),
                    *map(str, objects)], check=True, stdout=log, stderr=log)
    log.close()
    (output / 'objects.json').write_text(json.dumps({str(p.relative_to(ROOT)): sha(p) for p in objects}, indent=2) + '\n')
    (output / 'fast-render.json').write_text(json.dumps(report, indent=2) + '\n')
    sources = dict(old_proof['sources'])
    trans = ROOT / 'tools/machines/translation'
    for p in [HERE / 'fast_relink.py', HERE / 'fast_placement.py', HERE / 'native.py', HERE / 'native_support.py',
              HAND / 'native_source.py', HERE / 'md_words.s', HERE / 'call_clobber.py',
              HERE / 'dispatch49.c', ROOT / 'tools/machines/slot_split.inc'] + sorted(trans.glob('fast_*.py')) \
            + sorted(HAND.glob('*.s')):
        sources[str(p.relative_to(ROOT))] = sha(p)
    original_inputs = N.inputs
    N.inputs = lambda: sources
    try:
        N.finish(output, sources, old_proof['optimized_shared_kernels'], actual, linker,
                 code_regions={v['section']: P.REGIONS[v['region']] for v in placement['renders'].values()},
                 extra_regions=[(PAIR_SCRATCH, PAIR_SCRATCH_BYTES, 'pair render scratch'),
                                (PAIR_FRAME, 4 * PAIR_FRAME_WORDS, 'pair render frame'),
                                (HAND_SCRATCH, HAND_SCRATCH_BYTES, 'hand-written render scratch')], data_ranges=[MD_TABLES])
    finally:
        N.inputs = original_inputs
    proof = json.loads((output / 'build-proof.json').read_text())
    proof.update(build_route='fast render relink over reused objects', base_profile=str(base.relative_to(ROOT)),
                 base_stream_sha256=old_proof['stream_sha256'])
    (output / 'build-proof.json').write_text(json.dumps(proof, indent=2) + '\n')
    return proof


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--base', type=Path, default=ROOT / 'out/audio-fix-full49-routing')
    p.add_argument('--out', type=Path, default=ROOT / 'out/audio-fix-full49-fast')
    p.add_argument('--linker', type=Path, default=LINKER)
    p.add_argument('--no-hand', action='store_true', help='bisection: translated renders, no SIMD pair renders')
    p.add_argument('--no-subst', action='store_true', help='bisection: dispatch49.c and the engine C without substitutions')
    p.add_argument('--only-hand', default='', help='bisection: comma-separated prefixes that get their hand-written render')
    p.add_argument('--hand-file', action='append', default=[], help='bisection: prefix=path of a hand-written render to use')
    a = p.parse_args()
    r = build(a.base, a.out, a.linker, hand_renders=not a.no_hand, substitutions=not a.no_subst,
              only_hand=set(a.only_hand.split(',')) - {''}, hand_files=dict(x.split('=', 1) for x in a.hand_file))
    print('Fast native49:', r['stream_sha256'])
