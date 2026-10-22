"""Audit actual linked ELF, object spans, firmware boot records and manifests.

Unknown costs stay null. Unloaded memory is never declared physically free.
Run: python3 -m tools.resource_audit.audit --out out/resource-audit/report.json
"""
import argparse
import hashlib
import json
from pathlib import Path
from digimd import load_firmware, DNII_DEVICE, DNII_SHA256, sharc_blocks
from tools.resource_audit.elf import read_elf

ROOT=Path(__file__).resolve().parents[2]
REGIONS=[(0x240000,0x270000,'L1b0'),(0x2c0000,0x2f0000,'L1b1'),(0x300000,0x320000,'L1b2'),(0x380000,0x3a0000,'L1b3'),(0x20000000,0x20100000,'L2'),(0x80000000,0xa0000000,'DDR-address-aperture')]
# Larger cache guard bands from existing splice tool, not just brickcheck's 16K ranges.
FORBIDDEN=[(0x2e8000,0x2f0000,'DM cache guard'),(0x318000,0x320000,'PM cache guard'),(0x39c000,0x3a0000,'I cache'),(0x200918c0,0x200c2000,'unresolved stock L2 references'),(0x200fa000,0x20100000,'L2 reserved/ROM')]
# Existing L1 code in b3 overlaps the larger 32K I-cache guard used for *new* payloads.
NEW_FORBIDDEN=FORBIDDEN+[(0x398000,0x39c000,'larger I cache guard')]

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()
def core(addr):return addr-0x28000000 if 0x28240000<=addr<0x283a0000 else addr

def merge(spans):
    result=[]
    for start,end in sorted(spans):
        if start<0 or end<start:raise ValueError('Invalid interval')
        if not result or start>result[-1][1]:result.append([start,end])
        else:result[-1][1]=max(end,result[-1][1])
    return result

def conflicts(start,size,ranges):return [name for lo,hi,name in ranges if start<hi and lo<start+size]

def loads(stream):
    return [dict(start=core(b.address),end=core(b.address)+b.count,bytes=b.count,fill=b.fill,record=i) for i,b in enumerate(sharc_blocks(stream)) if b.count and not b.flags&0x38]

def memory_map(fw):
    raw=fw.section_bytes(7);records=loads(raw);result=[]
    for lo,hi,name in REGIONS:
        spans=merge([(max(lo,r['start']),min(hi,r['end'])) for r in records if r['start']<hi and lo<r['end']])
        result.append(dict(name=name,start=lo,end_exclusive=hi,physical_capacity_bytes=None if name.startswith('DDR') else hi-lo,loader_written_bytes=sum(b-a for a,b in spans),written_ranges=spans,free_bytes=None))
    return dict(stock_section7_sha256=hashlib.sha256(raw).hexdigest(),stock_stream_bytes=len(raw),boot_records=len(sharc_blocks(raw)),regions=result,records=records,
        caveat='Loader union includes init-program writes later overwritten. Unloaded bytes include runtime heaps/stacks/cache/buffers; free_bytes is deliberately null.',forbidden=FORBIDDEN,
        candidate_only=[dict(start=0x20088240,end_exclusive=0x200918c0,bytes=0x200918c0-0x20088240),dict(start=0x200c2000,end_exclusive=0x200fa000,bytes=0x200fa000-0x200c2000)],
        physical_headroom_bytes=None)

def components(manifest,folder):
    objects={name:read_elf(folder/(name+'.doj')) for name in manifest['objects']}
    totals={};coverage={};detail=[]
    for c in manifest['components']:
        obj=objects[c['object']];sections=[s for s in obj['sections'] if s['name']==c['section']]
        if len(sections)!=1:raise ValueError('Component section ambiguous')
        sec=sections[0];factor=2 if sec['unit']=='SW16' else 1
        def offset(value):
            if value=='section_end':return sec['size']
            if isinstance(value,int):return value
            sym=obj['symbols'][value]
            if not sym['defined'] or sym['section']!=sec['index']:raise ValueError('Anchor in wrong section')
            return (sym['value']-sec['address'])*factor
        start,end=offset(c['start']),offset(c['end'])
        if not 0<=start<=end<=sec['size']:raise ValueError('Component out of bounds')
        key=(c['object'],c['section']);spans=coverage.setdefault(key,[])
        if any(start<b and a<end for a,b in spans):raise ValueError('Overlapping component attribution')
        spans.append((start,end));totals[c['owner']]=totals.get(c['owner'],0)+end-start
        detail.append(dict(c,bytes=end-start,offset_start=start,offset_end=end))
    code=sum(s['size'] for obj in objects.values() for s in obj['sections'] if s['kind']=='code')
    unclassified=code-sum(totals.values())
    if unclassified<0:raise ValueError('Overcounted code')
    return dict(executable_bytes_by_owner=totals,unclassified_object_code_bytes=unclassified,object_code_bytes=code,components=detail,
        objects={n:dict(sha256=sha(folder/(n+'.doj')),sections=o['sections']) for n,o in objects.items()})

def audit(manifest_path,root=ROOT):
    m=json.loads(manifest_path.read_text());fw=load_firmware(root/'firmware/Digitone_II_OS1.12.syx',DNII_DEVICE)
    if fw.digest!=DNII_SHA256:raise ValueError('Require exact DNII 1.12')
    report=json.loads((root/m['build_report']).read_text());elf_path=root/m['elf'];elf=read_elf(elf_path)
    stream=(root/m['stream']).read_bytes();errors=[]
    if hashlib.sha256(stream).hexdigest()!=report['spliced_section7_sha256']:errors.append('Stream hash differs from build report')
    for name,digest in report['source_sha256'].items():
        if sha(root/name)!=digest:errors.append('Stale source: '+name)
    if report['static_checks']!={'ldr_check_ok':True,'cache_hits':[],'region_miss':[],'bad_shifts':[]}:errors.append('Build static checks failed')
    code=data=0;placements=[];all_loads=loads(stream)
    stock_loads=loads(fw.section_bytes(7))
    linked_intervals=[]
    for s in elf['sections']:
        start,size=s['byte_address'],s['size']
        if not size:continue
        if s['kind']=='code':code+=size
        else:data+=size
        linked_intervals.append((start,start+size))
        issues=conflicts(start,size,NEW_FORBIDDEN)
        if not any(lo<=start and start+size<=hi for lo,hi,_ in REGIONS):issues.append('outside known memory aperture')
        if any(start<r['end'] and r['start']<start+size for r in stock_loads):issues.append('overwrites stock loader bytes')
        if start%(2 if s['kind']=='code' else 4):issues.append('misaligned')
        if not any(r['start']<=start and start+size<=r['end'] for r in all_loads):issues.append('ELF section absent from stream')
        # Confirm actual payload matches ELF after converting VISA parcel byte order.
        from digimd import sharc_read
        target=start+0x28000000 if 0x240000<=start<0x3a0000 else start
        try:
            actual=sharc_read(stream,target,size)
            if hashlib.sha256(actual).hexdigest()!=s['payload_sha256']:issues.append('ELF payload differs from bootstream')
        except ValueError:issues.append('ELF payload missing from bootstream')
        matching=[r for r in report['sections'] if int(r['core_addr'],16)==start and r['bytes']==size]
        if len(matching)!=1:issues.append('ELF differs from build section map')
        placements.append(dict(s,issues=issues));errors.extend(issues)
    if sum(b-a for a,b in merge(linked_intervals))!=sum(b-a for a,b in linked_intervals):errors.append('Linked sections overlap')
    split=components(m,elf_path.parent)
    if split['unclassified_object_code_bytes']:errors.append('Unclassified executable bytes')
    padding=code-split['object_code_bytes']
    if padding<0:errors.append('Objects larger than linked code')
    added=report['added_blocks']
    for block in added:
        start=int(block['core'],16);size=block['bytes']
        errors.extend(conflicts(start,size,NEW_FORBIDDEN))
        if any(start<r['end'] and r['start']<start+size for r in stock_loads):errors.append('Added block overlaps stock')
    spans=[(int(b['core'],16),int(b['core'],16)+b['bytes']) for b in added]
    if sum(b-a for a,b in merge(spans))!=sum(b-a for a,b in spans):errors.append('Added blocks overlap')
    layouts=json.loads((root/m['layout_evidence']).read_text())
    state_reserved=sum(x['reservation_bytes'] for x in m['machines'])
    used=sum(x['voice_bytes']*x['voice_slots']+x['header_bytes'] for x in m['machines'])
    for v in m['machines']:
        layout=layouts['machines'].get(v['name'])
        if layout is None or layout['voice_bytes']!=v['voice_bytes'] or layout['state_bytes']!=v['voice_bytes']*16+v['header_bytes']:errors.append('Compiler layout differs from manifest: '+v['name'])
        if layout is not None and sha(root/v['layout_evidence'])!=layout['source_sha256']:errors.append('Stale compiler layout evidence: '+v['name'])
        if not any(owner==v['name'] or owner.startswith(v['name']+'-') for owner in split['executable_bytes_by_owner']):errors.append('Missing machine code attribution: '+v['name'])
        if not 0<=v['voice_slots']<=16:errors.append('Invalid native voice-slot count')
        if v['voice_bytes']*v['voice_slots']+v['header_bytes']>v['reservation_bytes']:errors.append('Voice state overflow: '+v['name'])
        if conflicts(v['reservation_address'],v['reservation_bytes'],NEW_FORBIDDEN):errors.append('Voice state reserved conflict')
        if not any(int(b['core'],16)==v['reservation_address'] and b['bytes']==v['reservation_bytes'] and b['kind']=='fill' for b in added):errors.append('Declared voice reservation missing from bootstream')
    for key,value in [('code_bytes',code),('data_bytes',data),('reserved_voice_bytes',state_reserved)]:
        if value>m['limits'][key]:errors.append(f'Regression: {key} {value} > {m["limits"][key]}')
    inventory=json.loads((root/m['inventory']).read_text())
    if inventory['machine_count']!=49 or len({v['md_id'] for v in inventory['machines']})!=49:errors.append('Incomplete/duplicate machine inventory')
    known_names={v['name'] for v in m['machines']}
    missing_native=[v['name'] for v in inventory['machines'] if v['name'] not in known_names]
    total_known=code+data+state_reserved+m['shared_data']['sine_bytes']+m['shared_data']['hook_save_bytes']
    benchmark_path=root/'out/resource-audit/experiments/experiments.json'
    benchmarks=None
    if benchmark_path.exists():
        evidence=json.loads(benchmark_path.read_text())
        if not any(result['build']['spliced_section7_sha256']==hashlib.sha256(stream).hexdigest() for result in evidence['results'].values()):errors.append('Benchmark image differs from audited image')
        benchmarks=dict(evidence_sha256=sha(benchmark_path),results={name:dict(init_instructions=result['init_instructions'],cases=[{k:v for k,v in case.items() if k not in ('per_block_instructions',)} for case in result['cases']]) for name,result in evidence['results'].items()},units='SHARC emulator executed instructions, not physical DSP cycles')
    stream_limit=0xfffff
    if len(stream)>stream_limit:errors.append('Depacked bootstream exceeds loader limit')
    ldf=(elf_path.parent/'trxbd.ldf').read_text()
    import re
    windows=[dict(name=n,start=int(a,16),end_exclusive=int(b,16)+1) for n,a,b in re.findall(r'(mem_trxbd_\w+)\s*\{[^}]*START\((0x[0-9a-f]+)\) END\((0x[0-9a-f]+)\)',ldf)]
    for w in windows:w['capacity_bytes']=w['end_exclusive']-w['start'];w['occupied_bytes']=sum(s['size'] for s in placements if w['start']<=s['byte_address']<w['end_exclusive']);w['remaining_bytes']=w['capacity_bytes']-w['occupied_bytes']
    return dict(schema='digimd.resources.audit.v1',status='FAIL' if errors else 'VALID_KNOWN_SUBSET',complete_49=False,errors=errors,manifest_sha256=sha(manifest_path),elf_sha256=sha(elf_path),stream_sha256=hashlib.sha256(stream).hexdigest(),code_bytes=code,data_bytes=data,total_known_residency_reserved_bytes=total_known,unknown_native_machines=missing_native,benchmarks=benchmarks,link_padding_bytes=padding,attribution=split,placements=placements,linker_windows=windows,stock=memory_map(fw),compiler_layouts=layouts,voice_state=dict(current_engine_arrays_used_bytes=used,current_engine_arrays_reserved_bytes=state_reserved,concurrent_slots=16,proposed_union_16x_max_known_bytes=16*max(v['voice_bytes'] for v in m['machines'])+8,layout_status='Voice/state/stream sizeof emitted by actual selcc into probe ELF; additionally checked by original/native full-state tests'),loader=dict(depacked_bytes=len(stream),depacked_max_bytes=stream_limit,depacked_remaining_bytes=stream_limit-len(stream),stored_max_bytes=0x100000,stored_bytes=None),shared_data=m['shared_data'],missing_measurements=m['missing'],physical_realtime_headroom=None)

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--manifest',type=Path,default=ROOT/'tools/resource_audit/manifest.json');p.add_argument('--out',type=Path,default=ROOT/'out/resource-audit/report.json');p.add_argument('--require-complete',action='store_true');a=p.parse_args()
    r=audit(a.manifest);a.out.parent.mkdir(parents=True,exist_ok=True);a.out.write_text(json.dumps(r,indent=2)+'\n')
    print(json.dumps({k:r[k] for k in ('status','code_bytes','data_bytes','link_padding_bytes','errors','voice_state','loader')},indent=2))
    return 1 if r['errors'] else 3 if a.require_complete else 0
if __name__=='__main__':raise SystemExit(main())
