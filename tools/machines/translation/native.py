"""Compile generated engines and compare executed SHARC with original DSP2."""
import argparse
import json
import statistics
import struct
import subprocess
import random
import os
from tools.machines.registry import ROOT
from tools.machines.translation.recover import OUT, sha
from tools.machines.translation.inputs import inputs
from tools.machines.registry import machines
from tools.md_runtime.build import assembly, resampler_asm
from tools.trxbd_sharc.build_sharc import SELACHE_BIN, LDF_TEMPLATE, DATA_RANGE, splice_build
import vm as V


def verify48(m,folder,vm,symbols,voice,params,sine):
    p=m['prefix'];words=m['original_slot_words'];offset=m['stream_byte_offset']
    rng=random.Random(0x48000+m['md_id']);rows=[];position=fraction=0
    nblocks=json.loads((folder/'host-proof.json').read_text())['blocks']
    patch=[q['default_raw']<<7 for q in next(q for q in machines() if q['md_id']==m['md_id'])['syn']]
    for block in range(nblocks):
        trig=int(block==0 or block%19==0)
        controls=patch if block<64 else [0]*8 if block<128 else [0x3f80]*8 if block<192 else [rng.randrange(0x3f81) for _ in range(8)]
        renders=trig
        if trig:position=fraction=0
        for _ in range(32):
            if position>=32:position-=32;renders+=1
            fraction+=147
            if fraction>=160:fraction-=160;position+=1
        assert renders<=1
        rows.append(dict(render=renders,trigger=trig,params=controls,position=position,fraction=fraction))
    schedule=folder/'schedule48.raw'
    schedule.write_bytes(b''.join(struct.pack('<10I',r['render'],r['trigger'],*r['params']) for r in rows))
    reference=folder/'original48.raw'
    with (folder/'oracle48.log').open('w') as log:
        subprocess.run([str(OUT/'oracle/build/mdsequence'),str(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'),str(m['md_id']),str(schedule),str(reference)],stdout=log,stderr=log,check=True,env=dict(os.environ,**dict(({'MD_EXTENDED_STATE':'1'} if m.get('private_memory') else {}),**({'MD_SCRATCH_Y20':'1'} if m.get('buffered_output') else {}))))
    raw=reference.read_bytes();f32=lambda x:struct.unpack('<f',struct.pack('<f',x))[0]
    pos=frac=previous=0;buffer=[0]*32;want=[];actual=bytearray();out=0x303000;counts=[]
    vm.call(symbols['md_runtime_reset'])
    vm.call(symbols[p+'_voice_init'],args=(voice,))
    for block,row in enumerate(rows):
        stride=(6916+4*int(m.get('buffered_output',False))) if m.get('private_memory') else 516
        original=raw[block*stride:(block+1)*stride];incoming=struct.unpack_from('<32i',original,132)
        vm.write(params,struct.pack('<8I',*row['params']))
        vm.call(symbols[p+'_update_coeffs'],args=(params,voice,row['trigger']))
        if row['trigger']:
            buffer=list(incoming);previous=buffer[0];pos=frac=0
            vm.call(symbols[p+'_voice_trigger'],args=(voice,))
            vm.call(symbols[p+'_render_block_44k1'],args=(voice,sine))
            vm.write(voice+offset+128,struct.pack('<iII',previous,0,0))
        for _ in range(32):
            if pos>=32:previous=buffer[-1];buffer=list(incoming);pos-=32
            s0=previous if pos==0 else buffer[pos-1];s1=buffer[pos]
            delta=f32(f32(f32(s1-s0)*f32(frac))*f32(1/(8388608*160)))
            want.append(f32(f32(s0/8388608)+delta))
            frac+=147
            if frac>=160:frac-=160;pos+=1
        before=vm.icount
        vm.call(symbols[p+'_render_block_48k_float'],args=(voice,sine,out))
        counts.append(vm.icount-before);actual+=vm.read(out,128)
        state=struct.unpack(f'<{words}I',vm.read(voice,4*words))
        assert struct.pack('<64I',*(x&0xffffff for x in state),*([0]*(64-words)))==original[260:516],(m['name'],block,'48k state')
        if m.get('private_memory'):
            extra=struct.unpack('<1600I',vm.read(voice+words*4+180,6400))
            assert struct.pack('<1600I',*(x&0xffffff for x in extra))==original[516:6916],(m['name'],block,'48k full external/X state')
        if m.get('buffered_output'): assert (vm.rd32(voice+m['state_bytes']-4)&0xffffff)==struct.unpack_from('<I',original,6916)[0],(m['name'],block,'scratch Y20 history')
        assert vm.read(voice+offset+132,8)==struct.pack('<II',pos,frac),(m['name'],block,'resampler phase')
        assert vm.rd32(voice+m.get('memory_fault_byte_offset',m['state_bytes']-4))==0
    expected=struct.pack(f'<{len(want)}f',*want)
    (folder/'reference-48000.f32').write_bytes(expected);(folder/'native-48000.f32').write_bytes(actual)
    assert actual==expected,(m['name'],'48k output')
    return dict(status='PASS',blocks=nblocks,samples=len(want),pcm_sha256=sha(folder/'native-48000.f32'),
                complete_slot_state_exact=True,phase_exact=True,
                emulator_instructions=dict(median=statistics.median(counts),p99=sorted(counts)[int(len(counts)*.99)],maximum=max(counts)))


def build(m, folder):
    p=m['prefix']
    (folder/'engine.c').write_text(f'#define MD_SOURCE_WORDS_BASE 0x80600000u\n#include "{ROOT/m["source"]}"\nconst uint32_t translated_voice_bytes = sizeof({m["voice_type"]});\n')
    (folder/'helpers.s').write_text(assembly(resampler_asm(p+'_resample_run',m['stream_byte_offset'])))
    ldf=LDF_TEMPLATE.format(c0=0x200c2000,c1=0x200f8fff,d0=DATA_RANGE[0],d1=DATA_RANGE[1]-1).replace('ENTRY(trxbd_voice_stub.)',f'ENTRY({p}_voice_init.)')
    (folder/'engine.ldf').write_text(ldf)
    commands=[['selcc','-proc','ADSP-21569','-char-size-8','-O1','-S','-o',folder/'engine.s',folder/'engine.c'],
              ['selcc','-proc','ADSP-21569','-char-size-8','-O1','-S','-o',folder/'control_tables.s',(ROOT/m['source']).parent/'control_tables.c'],
              ['selas','-proc','ADSP-21569','-o',folder/'control_tables.doj',folder/'control_tables.s'],
              ['selas','-proc','ADSP-21569','-o',folder/'engine.doj',folder/'engine.s'],
              ['selas','-proc','ADSP-21569','-o',folder/'helpers.doj',folder/'helpers.s'],
              ['seld','-proc','ADSP-21569','-T',folder/'engine.ldf','-o',folder/'engine.dxe',folder/'engine.doj',folder/'helpers.doj',folder/'control_tables.doj']]
    with (folder/'native-build.log').open('w') as log:
        for cmd in commands:
            subprocess.run([str(SELACHE_BIN/cmd[0]),*map(str,cmd[1:])],stdout=log,stderr=log,check=True)
    census=splice_build.register_census((folder/'engine.s').read_text()+(folder/'helpers.s').read_text())
    if set(census)-splice_build.CENSUS_ALLOWED: raise AssertionError(census)


def run(names=None):
    report=dict(schema='digimd.translation.native-parity.v1',machines=[],status='RUNNING',hardware_qualified=False)
    for m in json.loads((OUT/'batch.json').read_text())['machines']:
        if m['status']!='generated-unverified' or (names and m['name'] not in names):continue
        folder=OUT/m['name'];proof=json.loads((folder/'host-proof.json').read_text())
        if proof['status']!='PASS' or proof['source_sha256']!=sha(ROOT/m['source']) or proof.get('source_inputs')!=inputs(m) or proof.get('oracle_sha256')!=sha(OUT/'oracle/build/mdsequence'):raise ValueError('Host parity is absent/stale')
        build(m,folder)
        vm=V.VM();vm.load_dn2();elf=vm.load_elf(folder/'engine.dxe')
        stack_top=0x26f7f0
        vm.c_runtime_state(stack_base=stack_top-4000,stack_top=stack_top,l7=0,mode1=vm.reg('MODE1'))
        stack_peak=[0]
        real_call=vm.call
        def bounded_call(entry,args=(),**kwargs):
            V.watch(vm,stack_top-16384,stack_top+16)
            result=real_call(entry,args=args,**kwargs)
            writes=V.watch_log(vm);V.watch(vm,0,0)
            if writes:
                depth=stack_top-min(address for _,address,_ in writes)
                stack_peak[0]=max(stack_peak[0],depth)
                assert depth<=4000,(m['name'],'isolated kernel exceeds audio-task stack before stock callers',depth)
            return result
        vm.call=bounded_call
        symbols={k.rstrip('.'):v for k,v in elf.symbols.items()}
        p=m['prefix'];voice=0x300000;params=0x302000;coeff=0x302100;sine=0x80550000
        statebytes=m['state_bytes'];words=m['original_slot_words']
        measured=struct.unpack('<I',vm.read(symbols['translated_voice_bytes'],4))[0]
        assert measured==statebytes,(m['name'],measured,statebytes)
        vm.write(sine,(folder/'sine.raw').read_bytes())
        vm.write(0x80600000,(OUT/'P.u32').read_bytes()[0x80000:0x800000])
        vm.write(voice-16,b'\xa5'*(statebytes+32))
        vm.call(symbols['md_runtime_reset'])
        vm.call(symbols[p+'_voice_init'],args=(voice,))
        counts=[];actual=bytearray()
        schedule=(folder/'schedule.raw').read_bytes()
        original=(folder/'original.raw').read_bytes()
        for block in range(len(schedule)//40):
            row=struct.unpack_from('<10I',schedule,40*block)
            vm.write(params,struct.pack('<8I',*row[2:]));vm.write(coeff,struct.pack('<I',row[1])+bytes(63*4))
            n=vm.call(symbols[p+'_compute_coeffs'],args=(params,coeff))
            vm.write(coeff,struct.pack('<I',row[1]))
            actual+=struct.pack('<I',n)+vm.read(coeff,128)
            vm.call(symbols[p+'_update_coeffs'],args=(params,voice,row[1]))
            if row[1]:vm.call(symbols[p+'_voice_trigger'],args=(voice,))
            before=vm.icount
            if row[0]:vm.call(symbols[p+'_render_block_44k1'],args=(voice,sine))
            counts.append(vm.icount-before)
            assert vm.rd32(voice+m.get('memory_fault_byte_offset',statebytes-4))==0,(m['name'],block,'unmapped data')
            actual+=vm.read(voice+m['stream_byte_offset'],128)
            state=struct.unpack(f'<{words}I',vm.read(voice,4*words))
            actual+=struct.pack('<64I',*(x&0xffffff for x in state),*([0]*(64-words)))
            if m.get('private_memory'):
                extra=struct.unpack('<1600I',vm.read(voice+words*4+180,6400))
                actual+=struct.pack('<1600I',*(x&0xffffff for x in extra))
                if m.get('buffered_output'): actual+=struct.pack('<I',vm.rd32(voice+m['state_bytes']-4)&0xffffff)
            assert vm.read(voice-16,16)==b'\xa5'*16 and vm.read(voice+statebytes,16)==b'\xa5'*16
        dest=folder/'native-sharc.raw';dest.write_bytes(actual)
        if actual!=original:
            width=(1729+int(m.get('buffered_output',False))) if m.get('private_memory') else 129
            i=next(i for i,(a,b) in enumerate(zip(struct.iter_unpack('<I',actual),struct.iter_unpack('<I',original))) if a!=b)
            raise AssertionError((m['name'],'SHARC mismatch',i//width,i%width,actual[4*i:4*i+4].hex(),original[4*i:4*i+4].hex()))
        sections=splice_build.ranges_of(elf)
        code=sum(size for _,size,sec in sections if sec.kind=='sw_code')
        data=sum(size for _,size,sec in sections if sec.kind!='sw_code')
        rate48=verify48(m,folder,vm,symbols,voice,params,sine)
        r=dict(name=m['name'],md_id=m['md_id'],status='PASS',blocks=proof['blocks'],source_sha256=sha(ROOT/m['source']),
               source_inputs=inputs(m),oracle_sha256=sha(OUT/'oracle/build/mdsequence'),
               dxe_sha256=sha(folder/'engine.dxe'),reference_sha256=sha(folder/'original.raw'),actual_sha256=sha(dest),
               state_bytes=measured,code_bytes=code,data_bytes=data,isolated_stack_peak_bytes=stack_peak[0],stack_budget_bytes=4000,
               render44100_emulator_instructions=dict(median=statistics.median(counts),p99=sorted(counts)[int(len(counts)*.99)],maximum=max(counts)),
               resampling48000=rate48,guards_preserved=True,physical_cycles=None,limits=['Isolated native kernel; generic dispatch and stock FX are separate gates.'])
        report['machines'].append(r)
        (folder/'native-proof.json').write_text(json.dumps(r,indent=2)+'\n')
        print(m['name'],'PASS',r['render44100_emulator_instructions'],code,data,flush=True)
    report['status']='PASS' if report['machines'] else 'FAIL'
    (OUT/'native-proof.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser(__doc__);p.add_argument('--machines',nargs='*');a=p.parse_args();run(a.machines)
