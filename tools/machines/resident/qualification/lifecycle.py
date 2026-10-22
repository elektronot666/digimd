"""Original DSP lifecycle oracle with physical-voice permutations and49 locks.

The old physical-slot profile is an explicit negative control. Complete track
Y plus private X/delay histories are compared, including words temporarily
unused by an intervening machine. No device or golden file is touched.
"""
import argparse
import hashlib
import json
from pathlib import Path
import random
import struct
import subprocess

from tools.machines.registry import ROOT,machines
from tools.machines.compact.build import sha
from tools.machines.resident.common import install_alias
from tools.machines.verify_native import runner_for
from tools.trxbd_sharc.integration import configure
from tools.trxbd_sharc import build_sharc as B
from tools.machines.resident.native import HISTORY,HISTORY_BYTES as BYTES


def f32(x):return struct.unpack('<f',struct.pack('<f',x))[0]


def plan(folder,blocks,scratch_locks=False):
    catalog=machines();rows=[];blob=bytearray();pos=[0]*16;frac=[0]*16;prior=[None]*16
    generated={m['name']:m for m in json.loads((ROOT/'out/machines/translation/batch.json').read_text())['machines']}
    for b in range(blocks):
        tempo=3000 if b<32 else 720+(b*211)%6481;blob+=struct.pack('<I',tempo);row=[]
        for track in range(16):
            m=catalog[(track*3+b//8)%49]
            if scratch_locks and b<16:m=catalog[(0,12,0,12,1,12,1,12,0,1,12,0,12,1,12,12)[b]]
            if b>=64 and b%13 in (5,6) and track<4:m=None
            trigger=int(m is not None and (b==0 or b%3==0 or m!=prior[track]))
            params=[p['default_raw']<<7 for p in m['syn']] if m else [0]*8
            if m and b>=16:params=[0x3f80]*8 if b<24 else [random.Random(b*10000+track*100+k).randrange(0x3f81) for k in range(8)]
            render=trigger
            if trigger:pos[track]=frac[track]=0
            if m:
                for _ in range(32):
                    if pos[track]>=32:pos[track]-=32;render+=1
                    frac[track]+=147
                    if frac[track]>=160:frac[track]-=160;pos[track]+=1
            assert render<=1
            row.append(dict(machine=m,trigger=trigger,params=params,render=render,position=pos[track],fraction=frac[track]))
            blob+=struct.pack('<11I',m['md_id'] if m else 0xffffffff,trigger,render,*params);prior[track]=m
        # Both ordinary and reordered frames, with non-trigger migrations too.
        physical=[(track+(b//2)*3)%16 for track in range(16)]
        rows.append(dict(tempo=tempo,tracks=row,physical=physical))
    (folder/'schedule.raw').write_bytes(blob)
    oracle=ROOT/'out/machines/resident49/oracle/build/mdmasksequence'
    proof=json.loads((ROOT/'out/machines/resident49/integration48/proof.json').read_text())
    assert proof['status']=='PASS' and proof['oracle_sha256']==sha(oracle)
    with (folder/'oracle.log').open('w') as log:
        subprocess.run([str(oracle),str(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'),str(folder/'schedule.raw'),str(folder/'original.raw')],check=True,stdout=log,stderr=subprocess.STDOUT)
    return rows,generated


def execute(stream,folder,rows,metadata,track_owned,strict_shared=False):
    import fullblock
    vm,runner=runner_for('track-lifecycle-'+str(track_owned)+'-'+sha(Path(stream)),stream)
    settings=json.loads((ROOT/'data/control_defaults.json').read_text())
    raw=(folder/'original.raw').read_bytes();stride=16*6916+4
    assert len(raw)==len(rows)*stride
    previous=[0]*16;position=[0]*16;fraction=[0]*16;buffers=[[0]*32 for _ in range(16)]
    ever_private=[False]*16;covered=set();audio=hashlib.sha256();counts=[]
    symbols=B.E.read_elf(Path(stream).parent/'profile.dxe').symbols
    if track_owned:
        assert vm.rd32(symbols['md_history_pool_bytes.'])==BYTES
        assert vm.rd32(symbols['md_history_track_bytes.'])==6664
    scratch_path=folder/'scratch.raw'
    scratch=scratch_path.read_bytes() if scratch_path.exists() else None
    scratch_diffs=[]
    if strict_shared:assert scratch is not None,'Missing original shared-RAM diagnostic receipt'
    if scratch is not None:
        extra=16*6916+4;record=extra+608*4
        assert len(scratch)==len(rows)*record
        assert b''.join(scratch[b*record:b*record+extra] for b in range(len(rows)))==raw,'Diagnostic oracle changed original output'
    for b,row in enumerate(rows):
        types=[4]*16;mask=0;dry={}
        for track,r in enumerate(row['tracks']):
            physical=row['physical'][track];m=r['machine']
            if m is None:continue
            types[physical]=m['selector'];covered.add(m['name'])
            if r['trigger']:mask|=1<<physical
            incoming=list(struct.unpack_from('<32i',raw,b*stride+track*6916+132))
            if r['trigger']:buffers[track]=incoming;previous[track]=incoming[0];position[track]=fraction[track]=0
            values=[]
            for _ in range(32):
                if position[track]>=32:previous[track]=buffers[track][-1];buffers[track]=incoming;position[track]-=32
                a=previous[track] if position[track]==0 else buffers[track][position[track]-1];z=buffers[track][position[track]]
                values.append(f32(f32(a/8388608)+f32(f32(f32(z-a)*f32(fraction[track]))*f32(1/(8388608*160)))))
                fraction[track]+=147
                if fraction[track]>=160:fraction[track]-=160;position[track]+=1
            dry[track]=struct.pack('<32f',*values)
        configure(runner.d.frame,settings,types)
        struct.pack_into('<H',runner.d.frame,0xd8,row['tempo']*5)
        for off in (34,38,40):struct.pack_into('<H',runner.d.frame,off,mask)
        for track,r in enumerate(row['tracks']):
            physical=row['physical'][track]
            struct.pack_into('<H',runner.d.frame,0xa5a+2*physical,track)
            for off,p in zip((250,252,244,248,254,256,262,260),r['params']):
                struct.pack_into('<H',runner.d.frame,off+146*physical,p<<1)
        def capture():
            if track_owned:
                assert vm.rd32(HISTORY)==0x4d444859 and vm.rd32(HISTORY+4)==1
                for owner in range(16):
                    slot=0x81800008+7184*owner;bank=HISTORY+8+6664*owner
                    assert vm.rd32(slot+8)==vm.rd32(slot+7180)==0x4d444752
                    assert vm.rd32(bank)==vm.rd32(bank+6660)==0x4d445947
            if track_owned and scratch is not None:
                extra=16*6916+4;record=extra+608*4
                wanted=struct.unpack_from('<608I',scratch,b*record+extra)
                observed=struct.unpack('<256I',vm.read(symbols['md_shared_scratch_x.'],1024))+struct.unpack('<352I',vm.read(symbols['md_shared_scratch_y.'],1408))
                diffs=[(i,hex(x&0xffffff),hex(y)) for i,(x,y) in enumerate(zip(observed,wanted)) if (x&0xffffff)!=y]
                scratch_diffs.append(dict(block=b,differences=diffs))
                if strict_shared:
                    persistent=[d for d in diffs if d[0]<384]
                    assert not persistent,(b,'shared scratch X256/Y128',persistent[:12])
                (folder/'scratch-differences.json').write_text(json.dumps(scratch_diffs,indent=2)+'\n')
            for track,r in enumerate(row['tracks']):
                m=r['machine'];physical=row['physical'][track]
                if m is None:continue
                actual=vm.read(vm.rd32(0x254a60+4*physical),128)
                assert actual==dry[track],(b,track,physical,m['name'],'original48 PCM')
                slot=0x81800008+7184*(track if track_owned else physical)
                assert vm.rd32(slot)==(track<<8)|m['selector']
                assert vm.rd32(slot+4)==0 and vm.rd32(slot+8)==vm.rd32(slot+7180)==0x4d444752
                n=36 if m['name']=='TRX-BD' else 30 if m['name']=='TRX-SD' else metadata[m['name']]['original_slot_words']
                active=list(struct.unpack('<'+str(n)+'I',vm.read(slot+12,4*n)))
                original=struct.unpack_from('<64I',raw,b*stride+track*6916+260)
                if track_owned:
                    bank=HISTORY+8+6664*track
                    assert vm.rd32(bank)==vm.rd32(bank+6660)==0x4d445947
                    words=list(struct.unpack('<64I',vm.read(bank+4,256)));words[:n]=active
                    assert tuple(x&0xffffff for x in words)==original,(b,track,m['name'],'retained all64 Y',[(i,hex(x&0xffffff),hex(y)) for i,(x,y) in enumerate(zip(words,original)) if (x&0xffffff)!=y])
                else:assert tuple(x&0xffffff for x in active)==original[:n],(b,track,m['name'],'active Y')
                private=metadata[m['name']].get('private_memory',False)
                if private:ever_private[track]=True
                if private or (track_owned and ever_private[track]):
                    address=slot+12+4*n+180 if private else HISTORY+8+6664*track+260
                    values=struct.unpack('<1600I',vm.read(address,6400))
                    expected=struct.unpack_from('<1600I',raw,b*stride+track*6916+516)
                    assert tuple(x&0xffffff for x in values)==expected,(b,track,m['name'],'retained private X/delay')
                if track_owned:
                    offset=144 if m['name']=='TRX-BD' else 120 if m['name']=='TRX-SD' else metadata[m['name']]['stream_byte_offset']
                    assert vm.read(slot+12+offset+132,8)==struct.pack('<II',r['position'],r['fraction'])
                audio.update(actual)
            if track_owned:
                observed=vm.rd32(symbols['md_shared_y20.'])&0xffffff
                expected=struct.unpack_from('<I',raw,b*stride+16*6916)[0]
                assert observed==expected,(b,'shared Y20',hex(observed),hex(expected))
        vm.write(fullblock.RX_BASE+0x1000*vm.rd32(fullblock.SPI_HALF),bytes(runner.d.frame))
        vm.write(fullblock.CODEC_IN+0x100*vm.rd32(fullblock.DMA_HALF),bytes(256))
        before=vm.icount;runner._call_probed([(B.VOICE_RESUME_SW,capture)]);counts.append(vm.icount-before)
        if b%16==0:print('track-owned' if track_owned else 'negative-control','block',b,flush=True)
    assert len(covered)==49,('Machine coverage',len(covered))
    # Duplicate input owners latch a fault and silence the second physical slot.
    if track_owned:
        configure(runner.d.frame,settings,[9,9]+[4]*14)
        for physical in (0,1):struct.pack_into('<H',runner.d.frame,0xa5a+2*physical,3)
        for off in (34,38,40):struct.pack_into('<H',runner.d.frame,off,3)
        vm.write(fullblock.RX_BASE+0x1000*vm.rd32(fullblock.SPI_HALF),bytes(runner.d.frame))
        vm.write(fullblock.CODEC_IN+0x100*vm.rd32(fullblock.DMA_HALF),bytes(256))
        def duplicate():
            assert vm.rd32(0x81800008+7184*3+4)==1
            assert vm.read(vm.rd32(0x254a60+4),128)==bytes(128)
        runner._call_probed([(B.VOICE_RESUME_SW,duplicate)])
    return dict(status='PASS',blocks=len(rows),covered=sorted(covered),dry_pcm_sha256=audio.hexdigest(),
                maximum_handler_emulator_instructions=max(counts),all64Y_and_private_history=True,
                arbitrary_physical_voice_permutations=True,duplicate_owner_refused=track_owned,
                shared_scratch_X256_Y128_exact=strict_shared,physical_cycles=None)

