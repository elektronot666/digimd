"""Live accounting over actual linked objects using the original audit tools."""
import hashlib
import json
import json
from tools.machines.registry import ROOT, machines
from tools.resource_audit.audit import components
from tools.resource_audit.elf import read_elf


def audit(report, folder):
    from tools.trxbd_sharc import build_sharc as B
    manifest=dict(schema='digimd.machines.resource-manifest.v1',objects=['stubs','math_leaf','dispatch'],machines=[],components=[])
    def component(obj,owner,start=0,end='section_end'):
        manifest['components'].append(dict(object=obj,section='seg_swco',owner=owner,start=start,end=end))
    component('stubs','shared');component('dispatch','dispatch')
    component('math_leaf','shared',0,'trxbd_amp_clip.')
    component('math_leaf','TRX-BD','trxbd_amp_clip.')
    for m in machines():
        n=m['native']
        if not n['enabled']: continue
        obj='trxbd_sharc' if n['prefix']=='trxbd' else n['prefix']
        manifest['objects'].append(obj)
        spans=n.get('code_components') or [dict(owner=m['name'],start=0,end='section_end')]
        for span in spans: component(obj,span['owner'],span['start'],span['end'])
        obj=n['prefix']+'_resample';manifest['objects'].append(obj)
        component(obj,m['name']+'-runtime-specialization')
        if n['prefix']!='trxbd':
            for i,_ in enumerate(n['assembly_sources']):
                obj=n['prefix']+'_helper_'+str(i);manifest['objects'].append(obj);component(obj,m['name'])
    manifest['pool']=report['pool']
    split=components(manifest,folder)
    if split['unclassified_object_code_bytes']:
        raise ValueError('Unclassified executable bytes in the live registry build')
    code=sum(s['bytes'] for s in report['sections'] if s['kind']=='sw_code')
    data=sum(s['bytes'] for s in report['sections'] if s['kind']!='sw_code')
    padding=code-split['object_code_bytes']
    if not 0<=padding<16:
        raise ValueError('Unreconciled code alignment')
    # Also reconcile data across all objects and linked alignment.
    object_data=sum(s['size'] for o in split['objects'].values() for s in o['sections'] if s['kind']!='code')
    if not 0<=data-object_data<32:
        raise ValueError('Unreconciled linked data')
    inventory_path=ROOT/'data/machine_inventory.json'
    dataset_path=ROOT/'data/datasets.json'
    inventory=json.loads(inventory_path.read_text());originals={m['md_id']:m for m in inventory['machines']}
    datasets=json.loads(dataset_path.read_text())
    rows=[]
    for m in machines():
        n=m['native'];owners=[m['name'],m['name']+'-runtime-specialization']
        compiled=None
        if n['enabled']:
            spans=[c for c in split['components'] if c['owner'] in owners]
            payload=[]
            for c in spans:
                elf=B.E.read_elf(folder/(c['object']+'.doj'))
                section=next(s for s in elf.sections if s.name==c['section'])
                payload.append(section.data[c['offset_start']:c['offset_end']])
            obj='trxbd_sharc' if n['prefix']=='trxbd' else n['prefix']
            tables=[s.data for s in B.E.read_elf(folder/(obj+'.doj')).sections if s.kind=='data']
            compiled=dict(code_bytes=sum(c['bytes'] for c in spans),
                          code_sha256=hashlib.sha256(b''.join(payload)).hexdigest(),
                          data_bytes=sum(len(s) for s in tables),
                          data_sha256=hashlib.sha256(b''.join(tables)).hexdigest(),
                          state_bytes=report['compiler_layouts']['engine_state_bytes'][m['name']],
                          physical_cycles=None, benchmark_instructions=None)
            measured=ROOT/'out/machines/native-matrix.json'
            benchmark_kind={'TRX-BD':'BD','TRX-SD':'SD'}.get(m['name'])
            if measured.exists() and benchmark_kind is not None:
                matrix=json.loads(measured.read_text())
                if report['spliced_section7_sha256'] in matrix['streams'].values():
                    layout=report['layout']
                    compiled['benchmark_instructions']=dict(units='whole-handler emulator instructions, including stock processing',
                        matrix_sha256=hashlib.sha256(measured.read_bytes()).hexdigest(),
                        cases=[dict(voices=c['voices'],patch=c['patch'],metrics=c['whole_handler'][layout])
                               for c in matrix['cases'] if c['kind']==benchmark_kind])
        original=originals[m['md_id']]
        observed_sources=[d for d in datasets['e12']['sources'] if m['name'] in d['observed_machine_readers']]
        requirements=dict(coefficient_control=original['coefficient_control'],dsp=original['dsp'],
                          observed_data=original['data'],original_voice=original['voice'],
                          observed_e12_sources=observed_sources,
                          e12_directory={k:datasets['e12'][k] for k in ('directory_address','directory_words','source_count')} if m['family']=='E12' else None,
                          pi_external_state=next((d for d in datasets['pi_external_state'] if d['name']==m['name']),None),
                          complete_native_dependency_bound_proven=n['enabled'])
        rows.append(dict(original_requirements=requirements,selector=m['selector'],md_id=m['md_id'],name=m['name'],family=m['family'],
                         status=n['status'],original=m['original'],native=compiled or dict(code_bytes=None,data_bytes=None,state_bytes=None,benchmark_instructions=None,physical_cycles=None),
                         dependencies=n['source_dependencies'],remaining_unknowns=m['requirements']['unknowns']))
    result=dict(schema='digimd.machines.live-resources.v1',status='VALID_KNOWN_SUBSET',identities=49,available_engines=sum(m['native']['enabled'] for m in machines()),concurrent_slots=16,
                code_bytes=code,data_bytes=data,object_data_bytes=object_data,link_padding_bytes=padding,
                pool=report['pool'],compiler_layouts=report['compiler_layouts'],attribution=split,
                code_window_margin_bytes=report['placement_limits']['code_window'][1]-report['placement_limits']['code_window'][0]-code,
                data_window_margin_bytes=65536-data,physical_headroom_bytes=None,
                section7_sha256=report['spliced_section7_sha256'],elf_sha256=report['linked_elf_sha256'],
                shared_sine_bytes=131072,shared_decay_bytes=1752,hook_save_bytes=108,
                original_evidence_sha256={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest() for p in (inventory_path,dataset_path)},
                machines=rows,all_49_native_fit_proven=False,hardware_qualified=False)
    destination=folder.parent/'resources.json'
    destination.write_text(json.dumps(result,indent=2)+'\n')
    (folder.parent/'resource-manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return result
