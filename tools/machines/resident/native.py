"""Build corrected, optimized, resident49 code and a RAM-only companion bundle.

Compact/default firmware is independent. This is a native build target, not a
hardware activation or live-memory ownership certificate.
"""
from tools.source_identity import source_revision
import argparse
import hashlib
import json
from pathlib import Path
import re
import shutil
import struct
import subprocess
import sys
from tools.machines.registry import ROOT,machines
from tools.machines.compact.build import sha
from tools.machines.build import build as legacy_build
from tools.machines.resident import common
from tools.machines.resident.families import E12Sharing
from tools.machines.resident import native_support as O
from tools.md_runtime.build import resampler_asm
from tools.trxbd_sharc import build_sharc as B

HERE=Path(__file__).resolve().parent
HISTORY=0x81820000
HISTORY_BYTES=106632
POOL=0x81800000
POOL_BYTES=114952
ROUTE=0x8181d000
ROUTE_BYTES=0x800


def inputs():
    result=common.qualified_inputs()
    paths=[HERE/'dispatch49.c',HERE/'audio_pacing.s',HERE/'md_cycles.s',HERE/'hot/agu.s',HERE/'hot/accumulator.s',ROOT/'tools/machines/engine_slot.inc',
           ROOT/'out/machines/provision/md-original-1.63.assets']
    result.update({str(p.relative_to(ROOT)):sha(p) for p in paths})
    return result


def adapter():
    text=(ROOT/'tools/machines/engine_slot.inc').read_text()
    marker='    changed = 0u;';assert text.count(marker)==1
    text=text.replace(marker,'''    changed = 1u;
    /* Original init can rewrite coefficient words; every serialized frame
     * refreshes them, even when the incoming SYN values are unchanged. */
#ifdef MD_PRIVATE_ADAPTER
    {
        uint32_t offset,word,track;
        offset=0xa5au+2u*slot;word=*((const uint32_t *)0x25c48cu+(offset>>2));
        track=(offset&2u)?word>>16:word&0xffffu;
        v->virtual_voice=track;
    }
#endif
#ifdef MD_TEMPO_ADAPTER
    {
        uint32_t tempo;
        tempo=(*((const uint32_t *)0x25c48cu+(0xd8u>>2)))&0xffffu;
        tempo=(tempo+2u)/5u;
        if(tempo<720u)tempo=720u;
        if(tempo>7200u)tempo=7200u;
        md_control_tempo_denominator=tempo;
        if((v->last_p[0]>>16)!=tempo)changed=1u;
    }
#endif''')
    marker='if (*(v->last_p + k) != *(p14 + k))';assert text.count(marker)==1
    text=text.replace(marker,'if ((*(v->last_p + k) & 0xffffu) != *(p14 + k))')
    marker='    if (trig != 0u) {\n        MD_VOICE_TRIGGER';assert text.count(marker)==1
    return text.replace(marker,'''#ifdef MD_TEMPO_ADAPTER
    v->last_p[0]=(v->last_p[0]&0xffffu)|(md_control_tempo_denominator<<16);
#endif
    if (trig != 0u) {
        MD_VOICE_TRIGGER''')


def build(output,optimized=True):
    output=Path(output).resolve();output.mkdir(parents=True,exist_ok=True)
    source_inputs=inputs();all_rows=json.loads((common.BASE/'batch.json').read_text())['machines']
    rows=[m for m in all_rows if m['name'] not in ('TRX-BD','TRX-SD')];assert len(rows)==47
    byname={m['name']:m for m in all_rows};byselector={m['selector']:m for m in rows}
    sources={m['name']:(ROOT/m['source']).read_text() for m in rows}
    if optimized:sources={name:O.inline_raw_access(text)[0] for name,text in sources.items()}
    sharing=E12Sharing(rows,sources);sharing.prepare(output)
    legacy_build(output/'legacy',layout='l2-experimental');legacy=output/'legacy/build'
    linker=common.linker();shutil.copyfile(common.OUT/'linker.json',output/'linker.json')
    header=(ROOT/'tools/machines/dispatch.h').read_text().replace('#define MD_SLOT_WORDS 80u','#define MD_SLOT_WORDS 1792u').replace('0x00300000u','0x81800000u')
    (output/'dispatch.h').write_text(header);(output/'engine_slot.inc').write_text(adapter())
    descriptor=['#define MD_HAS_DISPOSAL 0']
    for m in machines():
        n=byselector.get(m['selector'],m['native']);p=n['prefix'];assert n['state_bytes']<=7168
        descriptor += [f'extern void {p}_slot_init(void *);',f'extern void {p}_slot_process(void *,uint32_t,uint32_t,const int32_t *,float *);']
    descriptor += ['const uint32_t md_param_offsets[8]={250u,252u,244u,248u,254u,256u,262u,260u};','static const MdDescriptor md_descriptors[49]={']
    for m in machines():
        n=byselector.get(m['selector'],m['native']);descriptor.append(f"{{{m['md_id']}u,{n['state_bytes']}u,{n['prefix']}_slot_init,{n['prefix']}_slot_process}},")
    descriptor.append('};');(output/'descriptors.h').write_text('\n'.join(descriptor)+'\n')
    history_words=[];private=0
    for m in machines():
        history_words.append(36 if m['name']=='TRX-BD' else 30 if m['name']=='TRX-SD' else byname[m['name']]['original_slot_words'])
        if byname[m['name']].get('private_memory'):private|=1<<(m['selector']-5)
    assert all(0<n<=64 for n in history_words)
    constants='static const uint8_t md_history_words[49]={'+','.join(str(n)+'u' for n in history_words)+'};\n'
    constants+=f'#define MD_PRIVATE_LOW 0x{private&0xffffffff:08x}u\n#define MD_PRIVATE_HIGH 0x{private>>32:08x}u\n'
    dispatch=(HERE/'dispatch49.c').read_text().replace('@LAYOUT_CONSTANTS@',constants)
    (output/'dispatch.c').write_text(dispatch)
    objects=[legacy/'stubs.doj',output/'math_scratch.doj',output/'dispatch.doj',output/'trxbd_scratch.doj',legacy/'trxbd_resample.doj',
             output/'trxsd_scratch.doj',legacy/'trxsd_helper_0.doj',legacy/'trxsd_resample.doj']
    actual={}
    with (output/'build.log').open('w') as log:
        def run(cmd):subprocess.run([str(B.SELACHE_BIN/cmd[0]),*map(str,cmd[1:])],check=True,stdout=log,stderr=log)
        def compile(name,source,lower=False):
            asm=output/(name+'.s');obj=output/(name+'.doj')
            run(['selcc','-proc','ADSP-21569','-char-size-8','-O1','-I'+str(output),'-S','-o',asm,source])
            if lower:asm.write_text(O.lower_assembly(asm.read_text()))
            if name=='trxbd_scratch':asm.write_text(O.export_bd_decay(asm.read_text()))
            run(['selas','-proc','ADSP-21569','-o',obj,asm]);actual[name]=dict(source_sha256=sha(source),assembly_sha256=sha(asm),object_sha256=sha(obj));return obj
        compile('dispatch',output/'dispatch.c')
        for name,path,transform,slot in [('trxbd_scratch',ROOT/'tools/trxbd_sharc/trxbd_sharc.c',O.bd_source,'trxbd'),
                                          ('trxsd_scratch',ROOT/'tools/trxsd/trxsd.c',O.sd_source,'trxsd')]:
            text=transform(path.read_text());text=re.sub(r'#include "([^"]+)"',lambda m:'#include "'+str((path.parent/m[1]).resolve())+'"',text)
            typ='TrxBdVoice' if slot=='trxbd' else 'TrxSdVoice';source=output/(name+'.c')
            source.write_text('#define MD_GENERIC_ENGINE\n'+text+f'\nconst uint32_t {slot}_slot_bytes=sizeof({typ});\n')
            compile(name,source)
        asm=output/'math_scratch.s';asm.write_text(O.bd_helpers((legacy/'native_helpers.s').read_text()))
        run(['selas','-proc','ADSP-21569','-o',output/'math_scratch.doj',asm])
        run(['selas','-proc','ADSP-21569','-o',output/'audio_pacing.doj',HERE/'audio_pacing.s'])
        objects.append(output/'audio_pacing.doj')
        run(['selas','-proc','ADSP-21569','-o',output/'md_cycles.doj',HERE/'md_cycles.s'])
        objects.append(output/'md_cycles.doj')
        for m in rows:
            p=m['prefix'];text=sources[m['name']]
            for old,new in (('../md_runtime_compat.h','tools/machines/md_runtime_compat.h'),('../../md_runtime/render48.inc','tools/md_runtime/render48.inc')):
                text=text.replace('"'+old+'"','"'+str(ROOT/new)+'"')
            text=text.replace('"../engine_slot.inc"','"'+str(output/'engine_slot.inc')+'"');text=sharing.fold_source(m,text)
            source=output/(p+'.c');source.write_text('#define MD_GENERIC_ENGINE\n#define MD_SOURCE_WORDS_BASE 0x81000000u\n'+
                ('#define MD_PRIVATE_ADAPTER\n' if m.get('private_memory') else '')+('#define MD_TEMPO_ADAPTER\n' if m['name'].startswith('E12-') else '')+
                text+'\nconst uint32_t '+p+'_slot_bytes=sizeof('+m['voice_type']+');\n')
            objects.append(compile(p,source,optimized))
            asm=output/(p+'_resample.s');asm.write_text('.SECTION/SW seg_swco;\n'+resampler_asm(p+'_resample_run',m['stream_byte_offset'])+'\n.ENDSEG;\n')
            obj=output/(p+'_resample.doj');run(['selas','-proc','ADSP-21569','-o',obj,asm]);objects.append(obj)
            print('Compiled native49',m['name'],flush=True)
        objects.append(compile('tables',ROOT/'tools/machines/generated/control_tables.c'))
        (output/'profile.ldf').write_text(B.LDF_TEMPLATE.format(c0=0x80a00000,c1=0x80ffffff,d0=B.DATA_RANGE[0],d1=B.DATA_RANGE[1]-1))
        subprocess.run([str(linker),'-proc','ADSP-21569','-T',str(output/'profile.ldf'),'-o',str(output/'profile.dxe'),*map(str,objects)],check=True,stdout=log,stderr=log)
    (output/'objects.json').write_text(json.dumps({str(p.relative_to(ROOT)):sha(p) for p in objects},indent=2)+'\n')
    return finish(output,source_inputs,optimized,actual,linker)


def finish(output,source_inputs,optimized,actual,linker,code_regions=None,extra_regions=(),data_ranges=()):
    """Turn a linked profile.dxe into the boot stream, RAM bundle and build proof."""
    common.install_alias();elf=B.E.read_elf(output/'profile.dxe');sy=elf.symbols
    stock=B.load_firmware(ROOT/'firmware/Digitone_II_OS1.12.syx').section_bytes(7);stream=B.ldr.Stream(stock)
    stream.patch(B.E.sw_to_byte(0x1c2958),bytes.fromhex('150f250048d73e703f92'),B.jump_nop_bytes(sy['md_safe_lookup.'])+b'\x01\x00','bounded stock lookup')
    stream.patch(B.E.sw_to_byte(B.CHORUS_INIT_CALL_SW),B.splice_build.cjump_bytes(B.CHORUS_INIT_SW),B.splice_build.cjump_bytes(sy['trxbd_init_stub.']),'native49 initialization')
    stream.patch(B.E.sw_to_byte(B.VOICE_HOOK_SW),B.VOICE_HOOK_STOCK,B.jump_nop_bytes(sy['trxbd_voice_stub.']),'native49 dispatch')
    stream.patch(B.E.sw_to_byte(0x1c9ff1),bytes.fromhex('300700000a00'),
                 B.jump_nop_bytes(sy['md_audio_pacing_gate.'])[:6],'advance DMA half on skipped SPORT completions')
    # the handler returns to 0x1C9FFB ('JUMP 0x1C9FE7'): stamp its end for the
    # governor's stock-time measurement, then continue to the wait
    stream.patch(B.E.sw_to_byte(0x1c9ffb),bytes.fromhex('3e07ff00ecff'),
                 B.jump_nop_bytes(sy['md_handler_end.'])[:6],'stamp handler end for the load governor')
    from tools.resource_audit.audit import conflicts,core
    memory=json.loads((ROOT/'data/memory_map.json').read_text());regions=[];code=data=0
    stock_blocks=B.ldr.Stream(stock).blocks
    def reserve(a,n,label):
        assert not conflicts(a,n,memory['sharc']['forbidden']),(label,hex(a))
        assert not any(a<end and begin<a+n for begin,end,_ in regions),(label,'custom overlap')
        assert not any(not b.marker and a<core(b.target)+b.count and core(b.target)<a+n for b in stock_blocks),(label,'stock loader overlap')
        regions.append((a,a+n,label))
    for a,n,section in B.splice_build.ranges_of(elf):
        if not n:continue
        reserve(a,n,section.name)
        if section.kind=='sw_code':
            begin,end=(code_regions or {}).get(section.name,(0x80a00000,0x81000000))
            assert begin<=a<a+n<=end,(section.name,hex(a),n,hex(begin),hex(end))
            code+=n
        else:assert any(lo<=a<a+n<=hi for lo,hi in (B.DATA_RANGE,*data_ranges)),(section.name,hex(a));data+=n
        if section.kind=='bss':stream.add_fill(a,(n+3)//4*4)
        else:stream.add_payload(a,section.data)
    for a,n,label in ((B.SAVE_ADDR,108,'hook saves'),(POOL,POOL_BYTES,'guarded slots'),(B.SINE_ADDR,B.SINE_BYTES,'sine'),(0x81000000,0x780000,'immutable source')):
        reserve(a,n,label)
        if label!='immutable source':stream.add_fill(a,n)
    from tools.machines.provision.assets import decode
    asset=(ROOT/'out/machines/provision/md-original-1.63.assets').read_bytes();native=decode(asset)
    assert hashlib.sha256(native).hexdigest()=='7d8537d51bdceebd84d14e26e5a33afcf7f6c4d080276d06ea66b030f9f0b552'
    cursor=88;position=0
    while cursor<len(asset):
        kind,count=struct.unpack_from('<BI',asset,cursor);cursor+=5;address=0x81000000+4*position
        if kind==0:stream.add_fill(address,count*4)
        else:
            payload=b''.join(struct.pack('<I',int.from_bytes(asset[i:i+3],'little')) for i in range(cursor,cursor+3*count,3));cursor+=3*count
            stream.add_payload(address,payload)
        position+=count
    assert position==0x1e0000
    reserve(HISTORY,HISTORY_BYTES,'track history');stream.add_fill(HISTORY,HISTORY_BYTES,'retained track history')
    reserve(ROUTE,ROUTE_BYTES,'voice routing and load policy');stream.add_fill(ROUTE,ROUTE_BYTES)
    for a,n,label in extra_regions:reserve(a,n,label);stream.add_fill(a,n)
    assert POOL+POOL_BYTES<=ROUTE and ROUTE+ROUTE_BYTES<=HISTORY
    raw=stream.to_bytes(fix_first=True);assert B.ldr.check_stream(raw)[0]
    (output/'section7.bin').write_bytes(raw)
    sys.path.insert(0,str(ROOT/'out/external/elekloader'))
    from elekloader.codec import aplib,elz
    packed=aplib.pack_section(raw);assert elz.depack_section(packed)==raw;(output/'section7.apl').write_bytes(packed)
    assert source_inputs==inputs(),'Native49 source inputs changed during build'
    (output/'inputs.json').write_text(json.dumps(source_inputs,indent=2)+'\n')
    proof=dict(schema='digimd.resident.native49-build.v1',status='PASS-build-only',machines=49,optimized_shared_kernels=optimized,
               lifecycle='logical-track retained64Y/private X-delay and original shared BD/SD scratch',mandatory_track_mono=True,
               source_commit=source_revision(),sources=source_inputs,
               dxe_sha256=sha(output/'profile.dxe'),stream_sha256=sha(output/'section7.bin'),packed_sha256=sha(output/'section7.apl'),
               native_code_bytes=code,supporting_data_bytes=data,stream_bytes=len(raw),packed_bytes=len(packed),
               pool=dict(address=hex(POOL),bytes=POOL_BYTES),history=dict(address=hex(HISTORY),bytes=HISTORY_BYTES,track_bytes=6664),
               actual_compilation=actual,objects_sha256=sha(output/'objects.json'),linker_sha256=sha(linker),
               regions=[[hex(a),hex(b),label] for a,b,label in regions],reference_parity='PENDING-profile-qualification',
               firmware_activation='NOT IMPLEMENTED',live_memory_ownership='UNQUALIFIED',physical_cycles=None,hardware_accessed=False)
    (output/'build-proof.json').write_text(json.dumps(proof,indent=2)+'\n')
    print('Built native49:',code,'code bytes;',data,'data bytes;',proof['stream_sha256'],flush=True);return proof


def main():
    p=argparse.ArgumentParser(__doc__);p.add_argument('--out',type=Path,default=ROOT/'out/machines/native49');p.add_argument('--baseline',action='store_true')
    a=p.parse_args();build(a.out,not a.baseline)


if __name__=='__main__':main()
