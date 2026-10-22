from tools.machines.resident.common import check_profile
check_profile()
"""Actual simultaneous original DSP vs all generated native family kernels."""
from pathlib import Path
import json,struct,subprocess,random,hashlib,sys
from tools.trxbd_sharc import build_sharc as B
from tools.machines.registry import ROOT,machines
from tools.machines.resident.common import original_oracle
import vm as V
OUT=ROOT/'out/machines/resident49/original';OUT.mkdir(parents=True,exist_ok=True);BASE=ROOT/'out/machines/translation';ELF=ROOT/'out/machines/resident49/profile.dxe';oracle=original_oracle()
from tools.machines.resident.common import install_alias
install_alias()
entries=[m for m in json.loads((BASE/'batch.json').read_text())['machines'] if m['name'] not in ('TRX-BD','TRX-SD')];legacy=[dict(name=m['name'],md_id=m['md_id'],selector=m['selector'],prefix=m['native']['prefix'],state_bytes=m['native']['state_bytes'],original_slot_words=36 if m['name']=='TRX-BD' else 30,stream_byte_offset=m['native']['stream_byte_offset'],private_memory=False,legacy=True) for m in machines() if m['name'] in ('TRX-BD','TRX-SD')];entries=legacy+entries;catalog={m['name']:m for m in machines()};groups={family:[m for m in entries if catalog[m['name']]['family']==family] for family in ('GND','TRX','EFM','E12','P-I')};groups.update({'all-a':entries[:16],'all-b':entries[16:32],'all-c':entries[32:]+entries[:1]});results=[];width=6916;record=16*width+4
for group,engines in groups.items():
 if sys.argv[1:] and group not in sys.argv[1:]:continue
 for count in (1,4,8,16):
  selected=[engines[t%len(engines)] for t in range(count)];schedules=[];blob=bytearray();folder=OUT/(group+'-'+str(count));folder.mkdir(exist_ok=True)
  for b in range(32):
   row=[]
   for t in range(16):
    m=selected[t] if t<count else None;default=[p['default_raw']<<7 for p in catalog[m['name']]['syn']] if m else [0]*8
    p=default if b<8 else [0]*8 if b<16 else [0x3f80]*8 if b<24 else [random.Random(b*10000+t*100+k).randrange(0x3f81) for k in range(8)]
    r=(m['md_id'] if m else 0xffffffff,int(b==0 or (b+t)%(7+t%3)==0),*p);row.append(r);blob+=struct.pack('<10I',*r)
   schedules.append(row)
  path=folder/'schedule.raw';path.write_bytes(blob);ref=folder/'original.raw'
  with (folder/'original.log').open('w') as log:subprocess.run([str(oracle),str(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'),str(path),str(ref)],check=True,stdout=log,stderr=log,timeout=60)
  raw=ref.read_bytes();assert len(raw)==32*record;vm=V.VM();vm.load_dn2();elf=vm.load_elf(ELF);sy=elf.symbols;top=0x26f7f0;vm.c_runtime_state(stack_base=top-4000,stack_top=top,l7=0,mode1=vm.reg('MODE1'));vm.write(B.SINE_ADDR,struct.pack('<32768i',*(v[0] if v[0]<0x800000 else v[0]-0x1000000 for v in struct.iter_unpack('<I',(BASE/'P.u32').read_bytes()[0x148000*4:0x150000*4]))));vm.write(0x81000000,(BASE/'P.u32').read_bytes()[0x80000:0x800000]);addresses=[0x81840010+t*7168 for t in range(count)]
  for t,(m,a) in enumerate(zip(selected,addresses)):
   vm.write(a-16,b'\xa5'*(m['state_bytes']+32));vm.call(sy[m['prefix']+'_voice_init.'],args=(a,))
   if m.get('private_memory'):vm.wr32(a+m['state_bytes']-8,t)
  failures=[];digest=hashlib.sha256();badblocks=0
  for b,row in enumerate(schedules):
   for t,(m,a) in enumerate(zip(selected,addresses)):
    p=m['prefix'];r=row[t];vm.write(0x302000,struct.pack('<8I',*r[2:]));vm.call(sy[p+'_compute_coeffs.'],args=(0x302000,a)) if m.get('legacy') else vm.call(sy[p+'_update_coeffs.'],args=(0x302000,a,r[1]))
    if r[1]:vm.call(sy[p+'_voice_trigger.'],args=(a,))
    vm.call(sy[p+'_render_block_44k1.'],args=(a,B.SINE_ADDR));assert m.get('legacy') or vm.rd32(a+m['memory_fault_byte_offset'])==0,(group,count,b,t,m['name'],'native memory fault')
    actual=vm.read(a+m['stream_byte_offset'],128);n=m['original_slot_words'];state=struct.unpack('<'+str(n)+'I',vm.read(a,4*n));actual+=struct.pack('<64I',*(v&0xffffff for v in state),*([0]*(64-n)))
    if m.get('private_memory'):
     state=struct.unpack('<1600I',vm.read(a+4*n+180,6400));actual+=struct.pack('<1600I',*(v&0xffffff for v in state))
    gold=raw[b*record+t*width+132:b*record+t*width+132+len(actual)]
    if actual!=gold:
     badblocks+=1
     if len(failures)<12:
      indexes=[i for i,(x,y) in enumerate(zip(struct.iter_unpack('<I',actual),struct.iter_unpack('<I',gold))) if x!=y];failures.append(dict(machine=m['name'],block=b,voice=t,words=len(indexes),first=[dict(word=i,native=hex(struct.unpack_from('<I',actual,4*i)[0]),original=hex(struct.unpack_from('<I',gold,4*i)[0])) for i in indexes[:4]]))
    assert vm.read(a-16,16)==b'\xa5'*16 and vm.read(a+m['state_bytes'],16)==b'\xa5'*16;digest.update(actual)
   shared=vm.rd32(sy['md_shared_y20.'])&0xffffff;gold=struct.unpack_from('<I',raw,b*record+16*width)[0]
   if shared!=gold:
    badblocks+=1
    if len(failures)<12:failures.append(dict(block=b,shared_y20_native=hex(shared),shared_y20_original=hex(gold)))
  r=dict(group=group,voices=count,status='FAIL' if badblocks else 'PASS',blocks=32,machines=[m['name'] for m in selected],mismatching_voice_blocks=badblocks,first=failures,native_sha256=digest.hexdigest(),original_sha256=hashlib.sha256(raw).hexdigest());results.append(r);print(group,count,r['status'],failures[:1],flush=True);(OUT/'proof.json').write_text(json.dumps(dict(status='RUNNING',dxe_sha256=hashlib.sha256(ELF.read_bytes()).hexdigest(),oracle_sha256=hashlib.sha256(oracle.read_bytes()).hexdigest(),probe_sha256=hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),cases=results,limits=['Original simultaneous voices and native44.1 kernels, including full private state and shared Y20.','Fixed identities during each schedule; retriggers and SYN modulation exercised.','Stock handler, 48k grouping, live ownership and hardware are separate qualifications.']),indent=2)+'\n')
r=json.loads((OUT/'proof.json').read_text());r['status']='PASS' if all(c['status']=='PASS' for c in results) else 'FAIL';(OUT/'proof.json').write_text(json.dumps(r,indent=2)+'\n')

from tools.machines.resident.common import qualified_inputs
assert json.loads((OUT.parent/'inputs.json' if OUT.name=='original' else OUT/'inputs.json').read_text())==qualified_inputs(),'Resident profile inputs changed'
