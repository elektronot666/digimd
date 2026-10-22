"""Link selected, original-verified engines beside BD/SD without external assets.

The default registry and frozen reference are never edited. The existing L1,
DDR tables, guarded pool and loader limits remain hard build gates.
"""
import argparse
import hashlib
import json
import re
import struct
import subprocess
from pathlib import Path
from tools.machines.registry import ROOT, machines
from tools.trxbd_sharc import build_sharc as B

DEFAULT = ('GND-IM',)
CANDIDATES = ('GND-SIN', 'GND-NS', 'GND-IM', 'TRX-XT', 'TRX-XC', 'EFM-BD', 'P-I-MA')


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def selected(names, *, allow_empty=False):
    from tools.machines.translation.inputs import inputs
    rows = {m['name']: m for m in json.loads((ROOT/'out/machines/translation/batch.json').read_text())['machines']}
    if (not names and not allow_empty) or len(set(names)) != len(names) or set(names) - set(CANDIDATES):
        raise ValueError('Select distinct asset-free compact candidates')
    result = []
    for name in names:
        m = rows[name]
        if m['requires_source_image'] or m['private_memory'] or m['state_bytes'] > 320:
            raise ValueError('Engine exceeds compact dependency/state contract: '+name)
        proof = json.loads((ROOT/'out/machines/translation'/name/'native-proof.json').read_text())
        if (proof['status'] != 'PASS' or proof['blocks'] < 384 or
                proof['resampling48000']['status'] != 'PASS' or proof['source_inputs'] != inputs(m) or
                proof['oracle_sha256'] != sha(ROOT/'out/machines/translation/oracle/build/mdsequence')):
            raise ValueError('Absent/stale original PCM/state certificate: '+name)
        result.append(m)
    return sorted(result, key=lambda m: m['selector'])


def table_subset(text, names):
    """Retain exact generated top-level definitions and their dependency closure.

    This is selection, not coefficient regeneration. Unknown/unresolved names
    fail at link. Top-level definitions have compiler-generated stable syntax.
    """
    pattern = re.compile(r'^(?:const )?(?:u?int32_t|void) (\w+)(?:\[[^\n]*?\]|\([^\n]*?\))?\s*(?:=\s*)?\{', re.M)
    definitions = {}
    for match in pattern.finditer(text):
        end = match.end(); depth = 1
        while depth:
            if end >= len(text): raise ValueError('Unbalanced generated definition')
            depth += (text[end] == '{') - (text[end] == '}'); end += 1
        if text[end:end+1] == ';': end += 1
        definitions[match[1]] = text[match.start():end]
    # Scalar globals have no brace.
    for match in re.finditer(r'^uint32_t (\w+) = [^;]+;', text, re.M):
        definitions[match[1]] = match[0]
    needed = set(names)
    while True:
        prior = set(needed)
        for name in prior:
            if name not in definitions: raise ValueError('Missing generated dependency '+name)
            needed.update(set(re.findall(r'\b\w+\b', definitions[name])) & definitions.keys())
        if prior == needed: break
    return '#include <stdint.h>\n'+'\n'.join(value for key, value in definitions.items() if key in needed)+'\n'


def build(output, names=DEFAULT):
    from tools.machines.build import build as legacy_build, POOL_BYTES
    from tools.machines.generate import descriptors
    from tools.resource_audit.audit import conflicts, core
    from tools.md_runtime.build import resampler_asm
    output = output.resolve(); output.mkdir(parents=True, exist_ok=True)
    (output/'build-proof.json').unlink(missing_ok=True)
    entries = selected(names, allow_empty=True)
    legacy_report=legacy_build(output/'legacy'); legacy = output/'legacy/build'
    folder = output/'build'; folder.mkdir(exist_ok=True)
    objects = [legacy/n for n in ('stubs.doj','math_leaf.doj','trxbd_sharc.doj',
                                 'trxsd.doj','trxsd_helper_0.doj')]
    desc = descriptors()
    for m in entries:
        desc = desc.replace(f"{{ {m['md_id']}u, 0u, 0, 0 }}", f"{{ {m['md_id']}u, {m['state_bytes']}u, {m['prefix']}_slot_init, {m['prefix']}_slot_process }}")
        desc = (f"extern void {m['prefix']}_slot_init(void *);\nextern void {m['prefix']}_slot_process(void *,uint32_t,uint32_t,const int32_t *,float *);\n"+desc)
    counts={5:36,6:30,**{m['selector']:m['original_slot_words'] for m in entries}}
    desc+='\nconst uint32_t md_history_words[49]={'+','.join(str(counts.get(i,0))+'u' for i in range(5,54))+'};\n'
    (folder/'descriptors.h').write_text(desc)
    (folder/'dispatch.h').write_bytes((ROOT/'tools/machines/dispatch.h').read_bytes())
    dispatch = Path(__file__).with_name('dispatch.c').read_text()
    dispatch += '\nconst uint32_t md_pool_bytes=sizeof(MdPool);\nconst uint32_t md_slot_bytes=sizeof(MdSlot);\n'
    (folder/'dispatch.c').write_text(dispatch)
    # Same unconditional coefficient update used by original-qualified resident49.
    adapter = (ROOT/'tools/machines/engine_slot.inc').read_text()
    start = adapter.index('    changed = 0u;')
    end = adapter.index('#ifdef MD_CONTROL_WITH_TRIGGER', start)
    adapter = adapter[:start]+'''    md_compact_parameters(slot, p14, v->last_p);
    {
'''+adapter[end:]
    adapter = 'extern void md_compact_parameters(uint32_t, uint32_t *, uint32_t *);\n'+adapter
    (folder/'engine_slot.inc').write_text(adapter)
    (folder/'history.c').write_bytes(Path(__file__).with_name('history.c').read_bytes())
    sources = [('dispatch', folder/'dispatch.c'),('history', folder/'history.c')]
    for m in entries:
        from tools.machines.compact.lowering import unused_initializers
        text, _ = unused_initializers((ROOT/m['source']).read_text())
        for old, new in (('../md_runtime_compat.h', str(ROOT/'tools/machines/md_runtime_compat.h')),
                         ('../../md_runtime/render48.inc', str(ROOT/'tools/md_runtime/render48.inc')),
                         ('../engine_slot.inc', str(folder/'engine_slot.inc'))):
            text = text.replace('"'+old+'"', '"'+new+'"')
        text = '#define MD_GENERIC_ENGINE\n'+text+f"\nconst uint32_t {m['prefix']}_slot_bytes=sizeof({m['voice_type']});\n"
        path = folder/(m['prefix']+'.c'); path.write_text(text); sources.append((m['prefix'],path))
    def run(cmd, log):
        subprocess.run([str(B.SELACHE_BIN/cmd[0]), *map(str,cmd[1:])], check=True, stdout=log, stderr=log)
    with (output/'build.log').open('w') as log:
        for name, source in sources:
            run(['selcc','-proc','ADSP-21569','-char-size-8','-O1','-S','-o',folder/(name+'.s'),source],log)
            run(['selas','-proc','ADSP-21569','-o',folder/(name+'.doj'),folder/(name+'.s')],log)
            objects.append(folder/(name+'.doj'))
        # Undefined table symbols from the compiled engine objects are authority.
        unresolved = set()
        for m in entries:
            from tools.resource_audit.elf import read_elf
            unresolved.update(k.rstrip('.') for k,v in read_elf(folder/(m['prefix']+'.doj'))['symbols'].items() if not v['defined'])
        tabletext = (ROOT/'tools/machines/generated/control_tables.c').read_text()
        candidates = set(re.findall(r'^(?:const )?(?:u?int32_t|void) (\w+)', tabletext, re.M))
        tables = table_subset(tabletext, (unresolved & candidates) | {'md_runtime_reset'})
        # Exact reset stores with advancing cursors avoid selcc's repeated
        # index scaling. Keep original constants and reset order unchanged.
        tables = tables.replace('    uint32_t i;\n', '    uint32_t i; int32_t *cursor;\n', 1)
        for space,count in (('x',256),('y',352)):
            old=f'    for(i=0u;i<{count}u;i++) md_shared_scratch_{space}[i]=0;'
            new=f'    cursor=md_shared_scratch_{space};\n    for(i=0u;i<{count}u;i++) *cursor++=0;'
            if tables.count(old)!=1: raise ValueError('Unexpected runtime reset')
            tables=tables.replace(old,new)
        (folder/'tables.c').write_text(tables)
        run(['selcc','-proc','ADSP-21569','-char-size-8','-O1','-S','-o',folder/'tables.s',folder/'tables.c'],log)
        run(['selas','-proc','ADSP-21569','-o',folder/'tables.doj',folder/'tables.s'],log); objects.append(folder/'tables.doj')
        (folder/'parameters.s').write_bytes(Path(__file__).with_name('parameters.s').read_bytes())
        run(['selas','-proc','ADSP-21569','-o',folder/'parameters.doj',folder/'parameters.s'],log); objects.append(folder/'parameters.doj')
        # One exact resampler body. Entry thunks adjust the caller-scratch
        # voice argument to its stream field; all persistent state stays in
        # the same original per-voice fields. No indirect/sample calls added.
        resample = resampler_asm('md_compact_resample',0)
        for prefix,offset in [('trxbd',144),('trxsd',120)]+[(m['prefix'],m['stream_byte_offset']) for m in entries]:
            resample += f'\n.GLOBAL {prefix}_resample_run.;\n{prefix}_resample_run.:\n    R0 = {offset};\n    R4 = R4 + R0;\n    JUMP md_compact_resample.;\n'
        path=folder/'resample.s'
        path.write_text('.SECTION/SW seg_swco;\n'+resample.replace('.SECTION/SW seg_swco;','').replace('.ENDSEG;','')+'\n.ENDSEG;\n')
        run(['selas','-proc','ADSP-21569','-o',path.with_suffix('.doj'),path],log); objects.append(path.with_suffix('.doj'))
        (folder/'compact.ldf').write_text(B.LDF_TEMPLATE.format(c0=B.CODE_RANGE[0],c1=B.CODE_RANGE[1]-1,d0=B.DATA_RANGE[0],d1=B.DATA_RANGE[1]-1))
        run(['seld','-proc','ADSP-21569','-T',folder/'compact.ldf','-Map',folder/'linker.map','-o',folder/'compact.dxe',*objects],log)
    elf = B.E.read_elf(folder/'compact.dxe'); symbols = elf.symbols
    census = B.splice_build.register_census('\n'.join(p.read_text() for p in folder.glob('*.s')))
    if set(census)-B.splice_build.CENSUS_ALLOWED: raise ValueError('Disallowed register use')
    stock = B.load_firmware(ROOT/'firmware/Digitone_II_OS1.12.syx',B.DNII_DEVICE).section_bytes(7)
    stream=B.ldr.Stream(stock)
    stream.patch(B.E.sw_to_byte(0x1c2958),bytes.fromhex('150f250048d73e703f92'),B.jump_nop_bytes(symbols['md_safe_lookup.'])+b'\x01\x00','bounded lookup')
    stream.patch(B.E.sw_to_byte(B.CHORUS_INIT_CALL_SW),B.splice_build.cjump_bytes(B.CHORUS_INIT_SW),B.splice_build.cjump_bytes(symbols['trxbd_init_stub.']),'compact init')
    stream.patch(B.E.sw_to_byte(B.VOICE_HOOK_SW),B.VOICE_HOOK_STOCK,B.jump_nop_bytes(symbols['trxbd_voice_stub.']),'compact dispatch')
    regions=[];code=data=0
    for a,n,s in B.splice_build.ranges_of(elf):
        if not n: continue
        lo,hi=B.CODE_RANGE if s.kind=='sw_code' else B.DATA_RANGE
        if not lo<=a<a+n<=hi: raise ValueError('Linked section outside existing reservation')
        regions.append((a,a+n,s.name))
        if s.kind=='sw_code': code+=n
        else: data+=n
        if s.kind=='bss': stream.add_fill(a,(n+3)//4*4)
        else: stream.add_payload(a,s.data)
    regions += [(B.SAVE_ADDR,B.SAVE_ADDR+108,'saves'),(B.STATE_ADDR,B.STATE_ADDR+POOL_BYTES,'pool'),(0x302000,0x303000,'original track Y history'),(B.SINE_ADDR,B.SINE_ADDR+B.SINE_BYTES,'sine')]
    forbidden=json.loads((ROOT/'data/memory_map.json').read_text())['sharc']['forbidden']
    for i,(a,b,label) in enumerate(regions):
        if conflicts(a,b-a,forbidden) or any(a<y and x<b for x,y,_ in regions[:i]): raise ValueError('Memory overlap '+label)
        for block in B.ldr.Stream(stock).blocks:
            if not block.marker and block.count and a<core(block.target)+block.count and core(block.target)<b: raise ValueError('Stock loader overlap '+label)
    for a,b,label in regions[-4:]: stream.add_fill(a,b-a,label)
    wire=stream.to_bytes(fix_first=True)
    if len(wire)>0xfffff or not B.ldr.check_stream(wire)[0]: raise ValueError('Stock bootstream cap/format')
    scan=B.bc_dsp.scan_sizes_and_blocks(wire,wire)
    if not(scan['parse_ok'] and scan['final'] and not scan['cache_hits'] and not scan['region_miss']): raise ValueError(scan)
    image,blocks,_,_=B.bc_dsp.final_image(wire); original,_,_,_=B.bc_dsp.final_image(stock)
    if B.bc_dsp.scan_bad_shifts(image,original,blocks): raise ValueError('Bad SHARC shift encoding')
    (output/'section7.bin').write_bytes(wire)
    # Verify measured state constants emitted by the actual target compiler.
    def constant(name):
        a=symbols[name+'.']
        for base,n,s in B.splice_build.ranges_of(elf):
            if base<=a<a+4<=base+n and s.kind!='sw_code': return struct.unpack_from('<I',s.data,a-base)[0]
        raise ValueError('Missing layout constant '+name)
    assert constant('md_pool_bytes')==POOL_BYTES and constant('md_slot_bytes')==336
    for m in entries: assert constant(m['prefix']+'_slot_bytes')==m['state_bytes']
    r=dict(schema='digimd.compact.build.v1',status='PASS-build-only',available=['TRX-BD','TRX-SD']+[m['name'] for m in entries],code_bytes=code,data_bytes=data,
           code_margin=B.CODE_RANGE[1]-B.CODE_RANGE[0]-code,data_margin=B.DATA_RANGE[1]-B.DATA_RANGE[0]-data,
           section7_sha256=sha(output/'section7.bin'),stream_bytes=len(wire),regions=regions,objects={str(p.relative_to(ROOT)):sha(p) for p in objects},
           sources={**legacy_report['source_sha256'],**{str(p.relative_to(ROOT)):sha(p) for p in [Path(__file__),Path(__file__).with_name('parameters.s'),Path(__file__).with_name('lowering.py'),Path(__file__).with_name('history.c'),Path(__file__).with_name('dispatch.c'),ROOT/'tools/machines/control.py',ROOT/'tools/machines/dispatch.c',ROOT/'tools/machines/dispatch.h',ROOT/'tools/machines/engine_slot.inc',ROOT/'tools/machines/generated/control_tables.c',*[ROOT/m['source'] for m in entries]]}},
           hardware_tested=False,external_assets=False,physical_cycles=None)
    (output/'build-proof.json').write_text(json.dumps(r,indent=2)+'\n');return r


if __name__=='__main__':
    p=argparse.ArgumentParser(__doc__);p.add_argument('--out',type=Path,default=ROOT/'out/machines/compact');p.add_argument('--machines',nargs='+',choices=CANDIDATES,default=DEFAULT)
    a=p.parse_args();print(json.dumps(build(a.out,a.machines),indent=2))
