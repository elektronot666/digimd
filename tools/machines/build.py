"""Resident multi-engine build, source manifests and bounded placement checks."""
import argparse
import hashlib
import json
from pathlib import Path
from tools.machines.registry import ROOT, machines, digest
from tools.machines.generate import descriptors

LAYOUTS = {'l1': (0x2dd600, 0x2e6000), 'l2-experimental': (0x200c2000, 0x200f9000)}
POOL_BYTES = 5384  # 8 + 16 * (320 payload + 16 ownership/guard bytes)


def validate_layout(name):
    if name not in LAYOUTS:
        raise ValueError('Unverified layout; no speculative placement or overlays')
    return LAYOUTS[name]


def prepare(folder):
    if (ROOT / 'tools/machines/descriptors.h').read_text() != descriptors():
        raise ValueError('Stale descriptors; run python3 -m tools.machines.generate')
    for m in machines():
        if m['native']['enabled']:
            if m['native']['external_state_bytes'] != 0:
                raise ValueError('External private state has no qualified allocation')
            if m['native']['state_bytes'] > 320:
                raise ValueError('Engine state exceeds the measured slot bound')
            prefix = m['native']['prefix']
            source = ROOT / m['native']['source']
            (folder / (prefix + '_generic.c')).write_text('#define MD_GENERIC_ENGINE\n#include "' + str(source) + '"\nconst uint32_t '+prefix+'_slot_bytes = sizeof('+m['native']['voice_type']+');\n')
    (folder / 'dispatch_generic.c').write_text('#include "' + str(ROOT / 'tools/machines/dispatch.c') + '"\nconst uint32_t md_pool_bytes = sizeof(MdPool);\nconst uint32_t md_slot_bytes = sizeof(MdSlot);\n')


def lookup_stub():
    # R2 is replaced by the very next stock lookup. Preserve all other inputs,
    # including raw M0, R1, ASTAT and MODE1. The selector index is already in
    # M0; the displaced move is M4=R1 (R1 is not the selector). Scratch save is
    # the existing hook area;
    # unpack runs before voice dispatch and cannot nest in that same handler.
    return '''
.SECTION/SW seg_swco;
.GLOBAL md_safe_lookup.;
md_safe_lookup.:
    DM(0x2E6800) = R0;
    DM(0x2E6804) = R1;
    R2 = ASTAT;
    I5 = md_stock_remap.;
    R0 = M0;
    R1 = 6;
    COMP(R0,R1);
    IF GT JUMP .L_md_custom;
.L_md_lookup_resume:
    R0 = DM(0x2E6800);
    R1 = DM(0x2E6804);
    M4 = R1;
    ASTAT = R2;
    JUMP 0x1C295D;
.L_md_custom:
    M0 = 5;
    JUMP .L_md_lookup_resume;
.ENDSEG;
'''


def finish(report, folder, elf, stock7, code_range):
    from tools.trxbd_sharc import build_sharc as B
    from tools.resource_audit.audit import conflicts, core
    # Load the audited map; do not infer runtime freedom from loader gaps.
    memory = json.loads((ROOT / 'data/memory_map.json').read_text())
    report['audited_memory_map_sha256'] = hashlib.sha256((ROOT/'data/memory_map.json').read_bytes()).hexdigest()
    reservations = [(B.SAVE_ADDR, B.SAVE_ADDR + 108, 'hook saves'),
                    (B.STATE_ADDR, B.STATE_ADDR + POOL_BYTES, '16 guarded slots'),
                    (B.SINE_ADDR, B.SINE_ADDR + B.SINE_BYTES, 'shared sine')]
    regions = []
    for address, size, sec in B.splice_build.ranges_of(elf):
        if not size:
            continue
        window = code_range if sec.kind == 'sw_code' else B.DATA_RANGE
        if not window[0] <= address < address + size <= window[1]:
            raise ValueError('Linked section exceeds verified window: ' + sec.name)
        regions.append((address, address + size, sec.name))
    all_ranges = regions + reservations
    for i, (lo, hi, name) in enumerate(all_ranges):
        if conflicts(lo, hi-lo, memory['sharc']['forbidden']):
            raise ValueError('Forbidden stock runtime region: ' + name)
        if any(lo < end and start < hi for start, end, _ in all_ranges[:i]):
            raise ValueError('Custom regions overlap: ' + name)
    # ELF and reservations may not touch any loader-written stock bytes.
    stream = B.ldr.Stream(stock7)
    for block in stream.blocks:
        if block.marker or not block.count:
            continue
        lo = core(block.target)
        for start, end, name in all_ranges:
            if start < lo + block.count and lo < end:
                raise ValueError('Stock loader overlap: ' + name)
    sources = [ROOT/'tools/machines'/name for name in ('dispatch.c','dispatch.h','engine_slot.inc','descriptors.h','registry.json','registry.py','generate.py','build.py','audit.py')]
    sources += [ROOT / m['native']['source'] for m in machines() if m['native']['enabled']]
    sources += [ROOT / p for m in machines() if m['native']['enabled'] for p in m['native']['source_dependencies']]
    report['source_sha256'].update({str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in sources})
    report['machine_registry_sha256'] = digest()
    report['reservations'] = [dict(address=hex(a), bytes=b-a, purpose=n) for a,b,n in reservations]
    report['pool'] = dict(slots=16, payload_bytes_per_slot=320, slot_stride=336, slot_state_offset=12, header_bytes=8, reservation_bytes=POOL_BYTES)
    report['placement_limits'] = dict(code_window=list(code_range), data_window=list(B.DATA_RANGE), physical_headroom=None,
                                     hardware_qualified=False, l2_experimental=report['layout'] != 'l1')
    report['linked_elf_sha256'] = hashlib.sha256((folder / 'trxbd.dxe').read_bytes()).hexdigest()
    report['linker_script_sha256'] = hashlib.sha256((folder / 'trxbd.ldf').read_bytes()).hexdigest()
    report['toolchain_sha256'] = {name: hashlib.sha256((B.SELACHE_BIN / name).read_bytes()).hexdigest() for name in ('selcc','selas','seld')}
    def native_constant(name):
        address = elf.symbols[name+'.']
        for addr, size, sec in B.splice_build.ranges_of(elf):
            if sec.kind == 'data' and addr <= address < addr+size:
                import struct
                return struct.unpack_from('<I', sec.data, address-addr)[0]
        raise ValueError('Missing compiler layout proof: '+name)
    measured = {m['name']: native_constant(m['native']['prefix']+'_slot_bytes') for m in machines() if m['native']['enabled']}
    if measured != {m['name']: m['native']['state_bytes'] for m in machines() if m['native']['enabled']}:
        raise ValueError('Compiler state sizeof disagrees with engine manifest')
    if native_constant('md_pool_bytes') != POOL_BYTES or native_constant('md_slot_bytes') != 336:
        raise ValueError('Compiler pool layout exceeds the validated reservation')
    report['compiler_layouts'] = dict(engine_state_bytes=measured, pool_bytes=native_constant('md_pool_bytes'), slot_bytes=native_constant('md_slot_bytes'))
    from tools.machines.audit import audit
    result = audit(report, folder)
    report['resource_report_sha256'] = hashlib.sha256((folder.parent/'resources.json').read_bytes()).hexdigest()
    report['resource_totals'] = {k:result[k] for k in ('code_bytes','data_bytes','code_window_margin_bytes','data_window_margin_bytes')}


def build(out, layout='l1'):
    from tools.trxbd_sharc.build_sharc import build_section7
    return build_section7(ROOT / 'firmware/Digitone_II_OS1.12.syx', out, dual_engine=True, generic_machines=True, layout=layout)


if __name__ == '__main__':
    p = argparse.ArgumentParser(__doc__)
    p.add_argument('--out', type=Path, default=ROOT / 'out/machines/l1')
    p.add_argument('--layout', choices=LAYOUTS, default='l1')
    a = p.parse_args()
    r = build(a.out, a.layout)
    print(r['spliced_section7_sha256'])
