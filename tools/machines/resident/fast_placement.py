"""Cache-safe, byte-addressed placement for Full49 fast0 objects only.

The L2 hole in memory_map.json remains forbidden, despite the older T3.2
static-free description. No runtime ownership or fetch-speed claim is made.
"""
import json
import sqlite3

from tools.machines.registry import ROOT
from tools.machines.compact.build import sha
from tools.trxbd_sharc import build_sharc as B
from tools.resource_audit.audit import core, conflicts

# Half-open core byte ranges. Split block 1 around the 108-byte hook save.
REGIONS = {
    'l1b2': (0x300000, 0x318000),
    'l1b1': (0x2dd52c, 0x2e6800),
    'l1b1_after_save': (0x2e686c, 0x2e8000),
    'l2_low': (0x2008823c, 0x200918c0),
    'l2_high': (0x200c2000, 0x200fa000),
}
# L1 block 2 is not used (2026-10-20): on the unit the DSP stopped (stuck
# tone, then silence) the first time P-I-SD's render at 0x300000 ran, in
# build 568c13a3, while sharcvm runs it bit-exact. Renders preferring an
# excluded region stay in DDR; every other placement is unchanged, so this
# build isolates block 2.
EXCLUDED = {'l1b2'}
PREFERRED = {
    'pisd': 'l1b2', 'pirs': 'l1b2', 'pimt': 'l1b2', 'pirc': 'l1b2',
    'pibd': 'l1b1', 'trxcy': 'l1b1', 'piml': 'l1b1',
    'pima': 'l1b1_after_save',
}


def sw_address(byte):
    return 0xb80000 + (byte - 0x20000000) // 2 if 0x20000000 <= byte < 0x20100000 else byte // 2


def audit():
    """Re-derive loader high-water marks and inspect aligned stock literals."""
    db = ROOT / 'out/stock-sharc.sqlite'
    with sqlite3.connect(f'file:{db}?mode=ro', uri=True) as c:
        blocks = c.execute("SELECT target_address, byte_count FROM blocks WHERE image='dn2-1.12' AND byte_count>0").fetchall()
        blocks = [(core(a), core(a) + n) for a, n in blocks]
        assert max(b for a, b in blocks if 0x2c0000 <= a < 0x2f0000) == REGIONS['l1b1'][0]
        assert max(b for a, b in blocks if 0x20000000 <= a < 0x20100000) == REGIONS['l2_low'][0]
        memory = json.loads((ROOT / 'data/memory_map.json').read_text())
        for name, (a, b) in REGIONS.items():
            assert not any(a < end and start < b for start, end in blocks), name
            assert not conflicts(a, b-a, memory['sharc']['forbidden']), name
        literals = []
        for sw, text in c.execute("SELECT sw, fields FROM insn WHERE image='dn2-1.12' AND aligned=1"):
            fields = json.loads(text)
            for prefix in ('addr', 'data'):
                if prefix + '[31:16]' not in fields:
                    continue
                value = core(fields[prefix + '[31:16]'] << 16 | fields.get(prefix + '[15:0]', 0))
                if any(a <= value < b for a, b in REGIONS.values()):
                    # T3.2 section 2.3: peripheral control mask, not a pointer.
                    assert (sw, prefix, value) == (0x1cb5d5, 'data', 0x300000), (hex(sw), prefix, hex(value))
                    literals.append(dict(sw=hex(sw), value=hex(value), meaning='peripheral control bit mask, not memory access'))
    return dict(database_sha256=sha(db), memory_map_sha256=sha(ROOT / 'data/memory_map.json'),
                aligned_literal_exceptions=literals, stock_loader_overlap=False, forbidden_overlap=False,
                runtime_ownership='UNVERIFIED', hardware_accessed=False)


def plan(objects, base_ldf):
    """Claim each fast0 object before the DDR wildcard; fail if it cannot fit."""
    evidence = audit()
    cursors = {k: a for k, (a, _) in REGIONS.items()}
    # bisection probes (2026-10-21), environment only: FULL49_PLACE_DDR=1 keeps
    # every render in DDR; FULL49_L2_SHIFT=n starts the L2 low region n bytes on
    import os
    excluded = set(REGIONS) if os.environ.get('FULL49_PLACE_DDR') else EXCLUDED
    cursors['l2_low'] += (int(os.environ.get('FULL49_L2_SHIFT', '0')) + 3) & ~3
    placements = {}
    ddr = []
    memory, sections = [], []
    for name, (a, b) in REGIONS.items():
        a = cursors[name]                    # (FULL49_L2_SHIFT moves the L2 low start)
        memory.append(f'   mem_fast_{name} {{ TYPE(BW RAM) START(0x{a:08x}) END(0x{b-1:08x}) WIDTH(8) }}')
    for obj in objects:
        if not obj.stem.endswith('_fast0'):
            continue
        p = obj.stem[:-6]
        code = B.E.read_elf(obj).sections
        assert len(code) == 1 and code[0].name == 'seg_swco' and code[0].kind == 'sw_code', obj
        n = len(code[0].data)
        allocated = (n + 3) & ~3
        if PREFERRED.get(p) in excluded or excluded == set(REGIONS):
            ddr.append(p)
            continue
        choices = ([PREFERRED[p]] if p in PREFERRED else []) + ['l2_low', 'l2_high']
        region = next((r for r in choices if r not in excluded and cursors[r] + allocated <= REGIONS[r][1]), None)
        assert region is not None, (obj, n, cursors)
        a = cursors[region]
        cursors[region] += allocated
        section = f'fast0_{p}'
        placements[p] = dict(region=region, section=section, byte_address=a, sw_address=sw_address(a),
                             code_bytes=n, allocated_bytes=allocated, object=str(obj.relative_to(ROOT)))
        sections.append(f'''      {section} SW
      {{
         INPUT_SECTION_ALIGN(4)
         FILL(0x1)
         INPUT_SECTIONS( "{obj}"(seg_swco) )
      }} > mem_fast_{region}
''')
    # 35 engine renders (the 12 other E12 members share theirs) and TRX-SD/BD
    # (35 without the TRX-SD/BD hand-written renders: a bisection build)
    assert len(placements) + len(ddr) in (35, 37), (len(placements), ddr)
    ldf = base_ldf.replace('MEMORY\n{', 'MEMORY\n{\n' + '\n'.join(memory), 1)
    ldf = ldf.replace('      trxbd_code SW', ''.join(sections) + '      trxbd_code SW', 1)
    regions = {k: dict(start=a, end=b, used_bytes=cursors[k]-a, free_bytes=b-cursors[k]) for k, (a, b) in REGIONS.items()
               if k not in EXCLUDED}
    return ldf, dict(status='PASS-static-placement', audit=evidence, regions=regions, renders=placements,
                     excluded_regions=sorted(EXCLUDED), ddr_renders=sorted(ddr))
