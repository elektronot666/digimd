"""Differential gates for generated kernels; expected PCM/state is original code."""
import argparse
import ctypes
import json
import random
import os
import struct
import subprocess
from pathlib import Path
from tools.machines.registry import ROOT, machines
from tools.machines.translation.recover import OUT, sha
from tools.machines.translation.inputs import inputs


def host_source(m):
    p=m['prefix'];v=m['voice_type'];words=m['original_slot_words']
    return f'''#define MD_DSP_HOST_TEST
#include "{ROOT/m['source']}"
#include <stdio.h>
uint32_t md_source_words[0x1e0000];
static void word(FILE *f, uint32_t v) {{
    unsigned char b[4] = {{v, v>>8, v>>16, v>>24}}; fwrite(b,1,4,f);
}}
int main(int argc, char **argv) {{
    {v} v;
    uint32_t row[10];
    int32_t c[64];
    int32_t sine[32768];
    uint32_t n, k;
    FILE *in, *out, *table;
    if (argc != 4) return 2;
    in = fopen(argv[1], "rb"); out = fopen(argv[2], "wb"); table = fopen(argv[3], "rb");
    if (!in || !out || !table) return 3;
    if (fread(sine,4,32768,table) != 32768) return 4;
    table = fopen("{OUT}/P.u32", "rb");
    if (!table || fseek(table,0x80000,SEEK_SET) != 0 || fread(md_source_words,4,0x1e0000,table) != 0x1e0000) return 6;
    md_runtime_reset();
    {p}_voice_init(&v);
    while (fread(row,4,10,in) == 10) {{
        for (k=0;k<64;k++) c[k] = 0;
        c[0] = row[1];
        n = {p}_compute_coeffs(row+2, c);
        word(out, n);
        c[0] = row[1];
        for (k=0;k<32;k++) word(out, (uint32_t)c[k]);
        {p}_update_coeffs(row+2, &v, row[1]);
        if (row[1]) {p}_voice_trigger(&v);
        if (row[0]) {p}_render_block_44k1(&v, sine);
        if (v.memory_fault) return 5;
        for (k=0;k<32;k++) word(out, (uint32_t)v.blk44[k]);
        for (k=0;k<64;k++) word(out, k<{words} ? u24(v.y[k]) : 0u);
        {'for (k=0;k<64;k++) word(out,u24(v.x_private[k])); for (k=0;k<1536;k++) word(out,u24(v.external[k]));' if m.get('private_memory') else ''}
        {'word(out,u24(v.scratch_y20_history));' if m.get('buffered_output') else ''}
    }}
    return fclose(out) != 0;
}}
'''


def verify(names=None, blocks=384):
    batch=json.loads((OUT/'batch.json').read_text());entries={m['md_id']:m for m in machines()}
    results=[]
    for m in batch['machines']:
        if m['status']!='generated-unverified' or (names and m['name'] not in names): continue
        folder=OUT/m['name'];folder.mkdir(exist_ok=True)
        source=folder/'host.c';source.write_text(host_source(m));exe=folder/'host'
        subprocess.run(['clang','-O2','-fsanitize=address,undefined','-fno-sanitize-recover=all','-o',str(exe),str(source),str((ROOT/m['source']).parent/'control_tables.c')],check=True)
        original=entries[m['md_id']];rng=random.Random(m['md_id'])
        default=[p['default_raw']<<7 for p in original['syn']]
        rows=[]
        for b in range(blocks):
            p=default if b<64 else [0]*8 if b<128 else [0x3f80]*8 if b<192 else [rng.randrange(0x3f81) for _ in range(8)]
            rows.append(struct.pack('<10I',1,int(b==0 or b%19==0 or b%64==0),*p))
        schedule=folder/'schedule.raw';schedule.write_bytes(b''.join(rows))
        reference=folder/'original.raw';actual=folder/'native-host.raw'
        with (folder/'oracle.log').open('w') as log:
            subprocess.run([str(OUT/'oracle/build/mdsequence'),str(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'),str(m['md_id']),str(schedule),str(reference)],check=True,stdout=log,stderr=log,env=dict(os.environ,**dict(({'MD_EXTENDED_STATE':'1'} if m.get('private_memory') else {}),**({'MD_SCRATCH_Y20':'1'} if m.get('buffered_output') else {}))))
        sine=folder/'sine.raw';sine.write_bytes((OUT/'P.u32').read_bytes()[0x148000*4:0x150000*4])
        subprocess.run([str(exe),str(schedule),str(actual),str(sine)],check=True,timeout=15)
        a,b=reference.read_bytes(),actual.read_bytes()
        equal=a==b
        result=dict(name=m['name'],md_id=m['md_id'],status='PASS' if equal else 'FAIL',blocks=blocks,
                    source_inputs=inputs(m),oracle_sha256=sha(OUT/'oracle/build/mdsequence'),
                    source_sha256=sha(ROOT/m['source']),schedule_sha256=sha(schedule),reference_sha256=sha(reference),actual_sha256=sha(actual),
                    samples=blocks*32,state_words=blocks*64,arithmetic_sanitizers=['ASan','UBSan'])
        if not equal:
            width=(1729+int(m.get('buffered_output',False))) if m.get('private_memory') else 129
            offsets=[i for i,(x,y) in enumerate(zip(struct.iter_unpack('<I',a),struct.iter_unpack('<I',b))) if x!=y]
            result['mismatching_words']=len(offsets)
            result['first_mismatches']=[dict(block=i//width,word=i%width,original=hex(struct.unpack_from('<I',a,i*4)[0]),actual=hex(struct.unpack_from('<I',b,i*4)[0])) for i in offsets[:12]]
        results.append(result)
        print(m['name'],result['status'],result.get('first_mismatches',''),flush=True)
        (folder/'host-proof.json').write_text(json.dumps(result,indent=2)+'\n')
    report=dict(schema='digimd.translation.host-parity.v1',status='PASS' if results and all(m['status']=='PASS' for m in results) else 'FAIL',machines=results,
                limits=['Host C proof; native SHARC, 48k resampling, integration and packaging are separate gates.'])
    (OUT/'host-proof.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser(__doc__);p.add_argument('--machines',nargs='*');p.add_argument('--blocks',type=int,default=384)
    a=p.parse_args();r=verify(a.machines,a.blocks)
    raise SystemExit(r['status']!='PASS')
