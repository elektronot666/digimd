#!/usr/bin/env python3
"""Compiled SHARC PCM/state vs scheduled original MD control + DSP2 execution.

No translated synthesizer or host C renderer supplies expected samples. The
only 48 kHz reference arithmetic is independently rounded rational resampling
of the original DSP2 blocks. Native and reference schedules share explicit
raw14 controls and the same original-clock render boundaries.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import math
from pathlib import Path
import random
import struct
import subprocess
import sys
import wave

ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT))
from tools.trxbd_sharc.build_sharc import VOICE_RESUME_SW
STATE_ADDR=0x304000
from tools.trxsd.recover import DEFAULTS,LABELS
PATCHES={'default':DEFAULTS,'min':(0,)*8,'max':(127,)*8,'all8':(92,100,120,110,115,80,40,96),'noise':(32,127,0,64,127,0,64,0),'harm_clip':(127,127,127,127,127,127,127,127)}
from digimd import MD_DEVICE,MD_SHA256,load_firmware
from digimd_pcm import load_pcm,compare_pcm
import vm as V
import vmrun
import fullblock

OFFSETS=(250,252,244,248,254,256,262,260)
PIN='6ea7a4adfcfacd1dc62fcac1cfea31b488f96d8f'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def cases(include_quiet=False):
    result=dict(PATCHES)
    for index,label in enumerate(LABELS):
        for suffix,value in (('min',0),('max',127)):
            controls=list(PATCHES['default'])
            controls[index]=value
            result[f'{label}_{suffix}']=tuple(controls)
    result['modulated_retrigger']=PATCHES['all8']
    result['rapid_retrigger']=PATCHES['noise']
    result['decay_max']=tuple((64,127,0,0,0,0,0,0))
    # This fixture needs 1,200 blocks to reach silence, modulation and the
    # retrigger. Keep short default qualification runs on their original cases.
    if include_quiet:
        result['quiet_modulated_retrigger']=PATCHES['default']
    return result


def schedule(name,patch,blocks):
    rng=random.Random(0x56303)
    params=[value<<7 for value in patch]
    position=fraction=0
    rows=[]
    for block in range(blocks):
        if name=='modulated_retrigger' and block%3==0 and block:
            params=[rng.randrange(0x3f81) for _ in range(8)]
        if name=='quiet_modulated_retrigger' and 800<=block<1000 and block%3==0:
            params=[rng.randrange(0x3f81) for _ in range(8)]
        trigger=block==0 or (name=='modulated_retrigger' and block%19==0) or (name=='rapid_retrigger' and block%3==0) or (name=='quiet_modulated_retrigger' and block==1000)
        renders=int(trigger)
        if trigger:
            position=fraction=0
        for _ in range(32):
            if position>=32:
                position-=32
                renders+=1
            fraction+=147
            if fraction>=160:
                fraction-=160
                position+=1
        if renders>1:
            raise AssertionError('A 48 kHz block unexpectedly needs multiple original blocks')
        rows.append(dict(render=renders,trigger=int(trigger),params=list(params),position=position,fraction=fraction))
    return rows


def f32(value):
    return struct.unpack('<f',struct.pack('<f',value))[0]


def resample(rows,original_rows):
    previous=0
    position=fraction=0
    buffer=[0]*32
    result=[]
    for row,original in zip(rows,original_rows):
        incoming=struct.unpack('<32i',original[8:136])
        used=0
        if row['trigger']:
            buffer=incoming
            previous=buffer[0]
            position=fraction=0
            used=1
        for _ in range(32):
            if position>=32:
                previous=buffer[-1]
                buffer=incoming
                position-=32
                used+=1
            s0=buffer[position-1] if position else previous
            s1=buffer[position]
            a=f32(s0/8388608)
            b=f32(f32(f32(s1-s0)*f32(fraction))*f32(1/(8388608*160)))
            result.append(f32(a+b))
            fraction+=147
            if fraction>=160:
                fraction-=160
                position+=1
        if used!=row['render'] or (position,fraction)!=(row['position'],row['fraction']):
            raise AssertionError('Independent resampler clock differs from schedule')
    return struct.pack(f'<{len(result)}f',*result)


def build_oracle():
    """Bind the executed oracle to the current overlay and clean pinned source."""
    dependency=ROOT/'out/external/mpc-vst-machinedrum'
    revision=subprocess.check_output(['git','-C',str(dependency),'rev-parse','HEAD'],text=True).strip()
    if revision!=PIN or subprocess.check_output(['git','-C',str(dependency),'status','--porcelain'],text=True).strip():
        raise ValueError('Original reference dependency is not the clean pinned revision')
    build=ROOT/'out/build/md-reference'
    # Reconfigure so a stale CMake source path cannot silently select another
    # oracle. Ninja rebuilds only changed inputs; no network or upstream edits.
    subprocess.run(['cmake','-S',str(ROOT/'tools/reference_host'),'-B',str(build),'-G','Ninja',
                    '-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS=-DDSP56K_NO_JIT_RUNTIME',
                    f'-DDIGIMD_MACHINEMODULE_ROOT={dependency}'],check=True,stdout=subprocess.DEVNULL)
    subprocess.run(['cmake','--build',str(build),'--target','digimd_mdsdsequence'],check=True,stdout=subprocess.DEVNULL)
    oracle=build/'digimd_mdsdsequence'
    if not oracle.is_file():
        raise FileNotFoundError('CMake did not produce the scheduled original-code oracle')
    return oracle,revision


def validate(section7,md,output,blocks=288,names=None,*,state_address=None):
    if blocks<32:
        raise ValueError('Require at least 32 output blocks to exercise the oscillator body')
    choices=cases(include_quiet=bool(names))
    selected=[(name,choices[name]) for name in names] if names else list(choices.items())
    if any(name=='quiet_modulated_retrigger' for name,_ in selected) and blocks<1200:
        raise ValueError('quiet_modulated_retrigger requires at least 1200 blocks')
    if load_firmware(md,MD_DEVICE).digest!=MD_SHA256:
        raise ValueError('MD OS 1.63 hash mismatch')
    oracle,revision=build_oracle()
    output.mkdir(parents=True,exist_ok=True)
    report_path=output/'original-parity.json'
    report_path.unlink(missing_ok=True)
    stream=section7.read_bytes()
    section_digest=hashlib.sha256(stream).hexdigest()
    results=[]
    for first in range(0,len(selected),16):
        batch=selected[first:first+16]
        plans=[schedule(name,patch,blocks) for name,patch in batch]
        vm=V.VM()
        vm.load_dn2()
        vmrun.load_stream(vm,stream)
        vm.call(vmrun.FXINIT)
        runner=vmrun.Runner(vm,{'PATTERN_VOL':127})
        frame=runner.d.frame
        for t in range(16):
            struct.pack_into('<H',frame,148+2*t,6 if t<len(batch) else 4)
        native=[[] for _ in batch]
        def capture():
            for t,plan in enumerate(plans):
                voice=vm.read(state_address(t) if state_address else STATE_ADDR+8+296*t,296)
                words=struct.unpack('<74I',voice)
                expected=plan[block]
                if (words[63],words[64])!=(expected['position'],expected['fraction']):
                    raise AssertionError(f'{batch[t][0]} block {block}: native resampling clock differs')
                floats=vm.read(vm.rd32(0x254a60+4*t),128)
                if not all(math.isfinite(x) for x in struct.unpack('<32f',floats)):
                    raise AssertionError('Native pre-filter output is non-finite')
                native[t].append((voice[:120],voice[120:248],floats))
        for block in range(blocks):
            trigger_mask=0
            for t,plan in enumerate(plans):
                row=plan[block]
                if row['trigger']:
                    trigger_mask|=1<<t
                for off,value in zip(OFFSETS,row['params']):
                    struct.pack_into('<H',frame,off+146*t,value<<1)
            for off in (34,38,40):
                struct.pack_into('<H',frame,off,trigger_mask)
            vm.write(fullblock.RX_BASE+0x1000*vm.rd32(fullblock.SPI_HALF),bytes(frame))
            vm.write(fullblock.CODEC_IN+0x100*vm.rd32(fullblock.DMA_HALF),bytes(256))
            runner._call_probed([(VOICE_RESUME_SW,capture)])
        for (name,patch),plan,actual in zip(batch,plans,native):
            folder=output/name
            folder.mkdir(parents=True,exist_ok=True)
            schedule_path=folder/'schedule.raw'
            schedule_path.write_bytes(b''.join(struct.pack('<10I',row['render'],row['trigger'],*row['params']) for row in plan))
            original_path=folder/'original-rows.raw'
            with (folder/'oracle.log').open('w') as log:
                subprocess.run([str(oracle),str(md),str(schedule_path),str(original_path)],check=True,stdout=log,stderr=subprocess.STDOUT)
            raw=original_path.read_bytes()
            if len(raw)!=blocks*392:
                raise AssertionError(f'{name}: incomplete original execution')
            original_rows=[raw[n:n+392] for n in range(0,len(raw),392)]
            unique_original=[]
            unique_native=[]
            for block,(row,original,got) in enumerate(zip(plan,original_rows,actual)):
                words=struct.unpack('<98I',original)
                if tuple(words[:2])!=(row['render'],row['trigger']):
                    raise AssertionError('Oracle schedule flags differ')
                state=struct.unpack('<30I',got[0])
                differences=[(i,state[i]&0xffffff,words[34+i]) for i in range(30)
                             if state[i]&0xffffff!=words[34+i]]
                if differences or got[1]!=original[8:136]:
                    raise AssertionError(f'{name} block {block}: native PCM/original Y state differs: {differences[:5]}')
                if row['render']:
                    unique_original.append(original[8:136])
                    unique_native.append(got[1])
            ref44=folder/'original-44100.raw'
            got44=folder/'native-44100.raw'
            ref44.write_bytes(b''.join(unique_original))
            got44.write_bytes(b''.join(unique_native))
            ref48=folder/'original-resampled-48000.f32'
            got48=folder/'native-48000.f32'
            ref48.write_bytes(resample(plan,original_rows))
            got48.write_bytes(b''.join(row[2] for row in actual))
            pcm44=compare_pcm(load_pcm(ref44,'s24in32le',44100),load_pcm(got44,'s24in32le',44100))
            pcm48=compare_pcm(load_pcm(ref48,'f32le',48000),load_pcm(got48,'f32le',48000))
            if not pcm44['bit_exact'] or not pcm48['bit_exact']:
                raise AssertionError(f'{name}: original PCM comparison failed: {pcm44} / {pcm48}')
            samples=load_pcm(ref44,'s24in32le',44100).values
            with wave.open(str(folder/'original-44100.wav'),'wb') as handle:
                handle.setnchannels(1)
                handle.setsampwidth(3)
                handle.setframerate(44100)
                handle.writeframes(b''.join(int(x).to_bytes(3,'little',signed=True) for x in samples))
            result=dict(case=name,syn=list(patch),blocks=blocks,state_words_compared=blocks*30,
                        state_words_per_block=30,
                        schedule_sha256=sha(schedule_path),original_rows_sha256=sha(original_path),
                        pcm44100=pcm44,pcm48000=pcm48)
            (folder/'comparison.json').write_text(json.dumps(result,indent=2)+'\n')
            results.append(result)
            print(name,pcm44['reference']['frames'],'original PCM samples: exact PCM/state/48k',flush=True)
    assert sha(section7)==section_digest,'Section changed during original-reference verification'
    report=dict(schema='digimd.trxsd.original-scheduled-parity.v1',section7_sha256=sha(section7),source_md_sha256=MD_SHA256,
                upstream_revision=revision,oracle_binary_sha256=sha(oracle),
                oracle_source_sha256=sha(ROOT/'tools/reference_host/mdsdsequence.cpp'),
                oracle_cmake_sha256=sha(ROOT/'tools/reference_host/CMakeLists.txt'),
                harness_source_sha256=sha(Path(__file__)),blocks=blocks,cases=results,
                physical_hardware_tested=False,
                boundary='Original control routine and DSP2 with explicit raw14/block/trigger schedule; no MD DSP1 or original control smoothing')
    report_path.write_text(json.dumps(report,indent=2)+'\n')
    return report


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--section7',type=Path,default=ROOT/'out/trxsd-native/section7_trxbd.bin')
    parser.add_argument('--md',type=Path,default=ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx')
    parser.add_argument('--out',type=Path,default=ROOT/'out/trxsd-native-parity')
    parser.add_argument('--blocks',type=int,default=288)
    parser.add_argument('--cases',nargs='+',choices=list(cases(include_quiet=True)))
    args=parser.parse_args()
    validate(args.section7,args.md,args.out,args.blocks,args.cases)


if __name__=='__main__':
    main()
