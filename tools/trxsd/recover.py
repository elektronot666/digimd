"""Recover OS 1.63 TRX-SD instructions, tables and control metadata, fail closed."""
from pathlib import Path
import hashlib
import json
import struct
from digimd import MD_DEVICE,MD_SHA256,load_firmware
from digimd_dsp56300 import parse_dsp_image
from digimd_trxbd_dsp import TABLE_P,pack24
from tools.extract_md_machines import extract_descriptors

ROOT=Path(__file__).resolve().parents[2]
ENTRIES={'init':0x10074c,'trigger':0x10075b,'render':0x100776}
DEFAULTS=(64,32,0,64,64,64,64,0)
LABELS=('PTCH','DEC','BUMP','BENV','SNAP','TONE','TUNE','CLIP')

def recover(path=ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'):
    fw=load_firmware(path,MD_DEVICE)
    if fw.digest!=MD_SHA256:raise ValueError('Require original OS 1.63')
    cpu=fw.section_bytes(0);dsp=parse_dsp_image(fw.section_bytes(1))
    descriptor=next(d for d in extract_descriptors(cpu) if d['md_id']==17)
    assert descriptor['coefficient_function_address']=='0x2013a6'
    assert tuple(p['label'] for p in descriptor['syn'])==LABELS
    assert tuple(p['default_raw'] for p in descriptor['syn'])==DEFAULTS
    for kind,address in ENTRIES.items():assert dsp.word('P',TABLE_P[kind]+18)==address
    assert dsp.word('P',TABLE_P['init']+19)==0x101882
    assert dsp.word('P',0x100880)==0xc
    sections={
        'code':('P',0x10074c,0x100881),
        'transient':('P',0x100881,0x101882),
        # Precisely reachable addresses for all raw14 PTCH values (0..0x3f80).
        'filter_cos':('X',0x140900+1021,0x140900+1530),
        'filter_radius':('X',0x141900+447,0x141900+956),
    }
    words={name:dsp.words(space,start,end-start) for name,(space,start,end) in sections.items()}
    binaries={name:pack24(data) for name,data in words.items()}
    binaries['control']=cpu[0x13a6:0x14c2]
    binaries['control_decay']=cpu[0x4bf94:0x4bf94+438*4]
    binaries['control_clip']=cpu[0x4c794+0x1e0:0x4c794+0x1e0+196*4]
    manifest=dict(schema='digimd.trxsd.recovery.v1',firmware_sha256=fw.digest,
        dsp2_sha256=hashlib.sha256(fw.section_bytes(1)).hexdigest(),descriptor=descriptor,
        entries={k:hex(v) for k,v in ENTRIES.items()},
        segments={name:dict(space=s,start=hex(a),end_exclusive=hex(b),words=b-a) for name,(s,a,b) in sections.items()},
        artifacts={name:dict(bytes=len(data),sha256=hashlib.sha256(data).hexdigest()) for name,data in binaries.items()},
        original_rate_hz=44100,block_samples=32,voice_slot_words=64,used_voice_words=30,
        state_words=list(range(13,30)),shared_sine=dict(address='0x148000',words=32768),
        limits=['Extraction establishes original bytes and bounds, not native implementation parity.'])
    return manifest,binaries

if __name__=='__main__':
    out=ROOT/'out/trxsd-recovery';out.mkdir(exist_ok=True)
    report,binaries=recover()
    for name,data in binaries.items():(out/(name+'.bin')).write_bytes(data)
    (out/'recovery.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'recovery.json')
