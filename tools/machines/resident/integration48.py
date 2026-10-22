"""Original masked DSP clocks and independent resampling through stock AMP/FX."""
from pathlib import Path
import hashlib,json,random,struct,subprocess
from tools.machines.resident.common import ROOT,OUT,BASE,check_profile,install_alias,original_oracle
check_profile();install_alias();original_oracle()
from tools.trxbd_sharc import build_sharc as B
from tools.trxbd_sharc.integration import configure
from tools.machines.verify_native import runner_for
from tools.machines.registry import machines
import fullblock
catalog=machines();byid={m['md_id']:m for m in catalog};generated={m['md_id']:m for m in json.loads((BASE/'batch.json').read_text())['machines']};work=[]
for start in range(0,49,12):work.append(('collection-'+str(start),[catalog[(start+t)%49] for t in range(12)]+[None]*4,16))
for n in (1,4,8):work.append(('voices-'+str(n),[catalog[(t*7+3)%49] for t in range(n)]+[None]*(16-n),n))
folder=OUT/'integration48';folder.mkdir(exist_ok=True);results=[];settings=json.loads((ROOT/'data/control_defaults.json').read_text());offsets=(250,252,244,248,254,256,262,260)
def f32(x):return struct.unpack('<f',struct.pack('<f',x))[0]
for name,selected,active in work:
    case=folder/name;case.mkdir(exist_ok=True);plans=[];wire=bytearray();positions=[0]*16;fractions=[0]*16
    for b in range(64):
        tempo=3000 if b<32 else 720+(b*211)%6481;wire+=struct.pack('<I',tempo);row=[]
        for t,m in enumerate(selected):
            trigger=int(b==0 or (b+t)%(17+t%3)==0) if t<active else 0
            params=[p['default_raw']<<7 for p in m['syn']] if m else [0]*8
            if m and b>=16:params=[0]*8 if b<24 else [0x3f80]*8 if b<32 else [random.Random(71000+b*100+t*10+k).randrange(0x3f81) for k in range(8)]
            render=trigger if m else 0
            if trigger:positions[t]=fractions[t]=0
            for _ in range(32):
                if positions[t]>=32:positions[t]-=32;render+=int(m is not None)
                fractions[t]+=147
                if fractions[t]>=160:fractions[t]-=160;positions[t]+=1
            assert render<=1
            r=dict(machine=m,trigger=trigger,params=params,render=render,position=positions[t],fraction=fractions[t]);row.append(r)
            wire+=struct.pack('<11I',m['md_id'] if m else 0xffffffff,trigger,render,*params)
        plans.append((tempo,row))
    schedule=case/'schedule.raw';schedule.write_bytes(wire);gold=case/'original.raw'
    with (case/'original.log').open('w') as log:subprocess.run([str(OUT/'oracle/build/mdmasksequence'),str(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'),str(schedule),str(gold)],stdout=log,stderr=log,check=True)
    raw=gold.read_bytes();stride=16*6916+4;assert len(raw)==64*stride
    native_vm,native=runner_for('resident49',OUT/'section7.bin');reference_vm,reference=runner_for('legacy-carrier',OUT/'legacy/section7_trxbd.bin')
    types=[m['selector'] if m else (t-12)%4 if t<active else 4 for t,m in enumerate(selected)]
    for runner in (native,reference):
        configure(runner.d.frame,settings,types)
        runner.set(CHR_DPTH=64,CHR_SPD=48,CHR_HPF=0,CHR_WDTH=127,CHR_VOL=64,DEL_TIME=1,DEL_X=1,DEL_WID=127,DEL_FDBK=32,DEL_HPF=0,DEL_LPF=127,DEL_REV=32,DEL_VOL=64,REV_PRE=0,REV_DEC=40,REV_FREQ=64,REV_GAIN=64,REV_HPF=0,REV_VOL=64)
        for t in range(16):
            struct.pack_into('<H',runner.d.frame,0xa5a+2*t,t)
            for off,val in ((340,40),(342,48),(344,56)):struct.pack_into('<H',runner.d.frame,off+146*t,val<<8)
    previous=[0]*16;position=[0]*16;fraction=[0]*16;buffers=[[0]*32 for _ in range(16)];samples=hashlib.sha256();counts=[];peak=0.0
    for b,(tempo,row) in enumerate(plans):
        dry={}
        for t,r in enumerate(row):
            if not r['machine']:continue
            incoming=list(struct.unpack_from('<32i',raw,b*stride+t*6916+132))
            if r['trigger']:buffers[t]=incoming;previous[t]=incoming[0];position[t]=fraction[t]=0
            out=[]
            for _ in range(32):
                if position[t]>=32:previous[t]=buffers[t][-1];buffers[t]=incoming;position[t]-=32
                s0=previous[t] if position[t]==0 else buffers[t][position[t]-1];s1=buffers[t][position[t]]
                out.append(f32(f32(s0/8388608)+f32(f32(f32(s1-s0)*f32(fraction[t]))*f32(1/(8388608*160)))))
                fraction[t]+=147
                if fraction[t]>=160:fraction[t]-=160;position[t]+=1
            dry[t]=struct.pack('<32f',*out);assert (position[t],fraction[t])==(r['position'],r['fraction'])
        mask=sum(1<<t for t,r in enumerate(row) if r['trigger'])
        for runner in (native,reference):
            struct.pack_into('<H',runner.d.frame,0xd8,tempo*5)
            for t,r in enumerate(row):
                if r['machine']:
                    for off,v in zip(offsets,r['params']):struct.pack_into('<H',runner.d.frame,off+146*t,v<<1)
            for off in (34,38,40):struct.pack_into('<H',runner.d.frame,off,mask)
        def inject():
            for t,data in dry.items():reference_vm.write(reference_vm.rd32(0x254a60+4*t),data)
        def capture():
            for t,r in enumerate(row):
                if not r['machine']:continue
                m=r['machine'];meta=generated[m['md_id']];address=0x81800008+7184*t+12
                n=36 if m['name']=='TRX-BD' else 30 if m['name']=='TRX-SD' else meta['original_slot_words']
                assert native_vm.read(native_vm.rd32(0x254a60+4*t),128)==dry[t],(name,b,t,m['name'],'original48 dry PCM')
                state=struct.unpack('<'+str(n)+'I',native_vm.read(address,4*n));wanted=raw[b*stride+t*6916+260:b*stride+t*6916+516]
                assert struct.pack('<64I',*(v&0xffffff for v in state),*([0]*(64-n)))==wanted,(name,b,t,m['name'],'original slot state')
                if meta.get('private_memory'):
                    state=struct.unpack('<1600I',native_vm.read(address+4*n+180,6400));assert struct.pack('<1600I',*(v&0xffffff for v in state))==raw[b*stride+t*6916+516:b*stride+t*6916+6916],(name,b,t,m['name'],'private history')
        for vm,runner,callback in ((reference_vm,reference,inject),(native_vm,native,capture)):
            vm.write(fullblock.RX_BASE+0x1000*vm.rd32(fullblock.SPI_HALF),bytes(runner.d.frame));vm.write(fullblock.CODEC_IN+0x100*vm.rd32(fullblock.DMA_HALF),bytes(256));before=vm.icount;runner._call_probed([(B.VOICE_RESUME_SW,callback)])
            if vm is native_vm:counts.append(vm.icount-before)
        actual=native_vm.read(fullblock.OUT_FLOATS,256);assert actual==reference_vm.read(fullblock.OUT_FLOATS,256),(name,b,'stock AMP/filter/FX stereo')
        for fx in (0x25b668,0x25b068,0x25b368):assert native_vm.read(fx,256)==reference_vm.read(fx,256),(name,b,'stock effects')
        samples.update(actual);peak=max(peak,*map(abs,struct.unpack('<64f',actual)))
    result=dict(name=name,voices=active,blocks=64,status='PASS',md_machines=[m['name'] for m in selected if m],pcm_sha256=samples.hexdigest(),peak=peak,maximum_emulator_instructions=max(counts));results.append(result);print(name,'PASS',flush=True)
    (folder/'proof.json').write_text(json.dumps(dict(status='RUNNING',cases=results),indent=2)+'\n')
check_profile();(folder/'proof.json').write_text(json.dumps(dict(status='PASS',cases=results,source_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),oracle_sha256=hashlib.sha256((OUT/'oracle/build/mdmasksequence').read_bytes()).hexdigest(),stream_sha256=hashlib.sha256((OUT/'section7.bin').read_bytes()).hexdigest(),limits=['Original CPU/DSP dry PCM through unchanged stock AMP/filter/FX, deterministic frames, no physical capture.','Emulated instruction counts are not physical cycles or real-time headroom.']),indent=2)+'\n')
