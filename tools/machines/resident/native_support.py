"""Exact full49 source/assembly lowering; compact and legacy kernels stay independent."""
import re
from pathlib import Path

HOT=Path(__file__).with_name('hot')


def function(text,name):
    begin=text.index('static uint32_t '+name+'(');brace=text.index('{',begin);depth=1;end=brace+1
    while depth:
        depth+=(text[end]=='{')-(text[end]=='}');end+=1
    return begin,end,text[begin:end]

def inline_raw_access(text):
    changed=[]
    for name in ('md_read_x','md_read_y','md_read_source'):
        if 'static uint32_t '+name+'_raw(' not in text:continue
        _,_,body=function(text,name)
        body=body.replace('static uint32_t '+name+'(','static uint32_t '+name+'_raw(',1)
        marker='address = address & 0xffffffu;'
        assert body.count(marker)==1
        body=body.replace(marker,marker+'\n    if (address >= 0xffff80u) { return 0u; }',1)
        lo,hi,_=function(text,name+'_raw')
        text=text[:lo]+body+text[hi:];changed.append(name)
    assert changed
    return text,changed

def replace_function(text,name,leaf):
    match=re.search(r'^'+re.escape(name)+r'\.:\n',text,re.M);assert match,name
    following=re.search(r'^[A-Za-z_]\w*\.:\n',text[match.end():],re.M)
    assert following,name
    end=match.end()+following.start()
    return text[:match.start()]+leaf+'\n'+text[end:]

def bd_source(text):
    def change(old,new):
        nonlocal text
        assert text.count(old)==1,old;text=text.replace(old,new)
    change('void trxbd_render_block_44k1(', 'extern int32_t md_shared_scratch_x[256];\nextern int32_t md_shared_scratch_y[352];\nvoid trxbd_render_block_44k1(')
    change('    uint32_t noise_silent;', '    uint32_t noise_silent;\n    uint32_t scratch_h,scratch_l,scratch_sum;')
    change('    /* 3. Transient attack countdown', '''    for(i=0u;i<32u;i++) {
        if(noise_silent)md_shared_scratch_y[i]=0;
        else md_shared_scratch_y[i]=noise[i];
    }

    /* 3. Transient attack countdown''')
    change('    trxbd_phase_prepare(y, oscillator_increments);', '''    trxbd_phase_prepare(y, oscillator_increments);
    md_shared_scratch_x[6]=y[26];md_shared_scratch_x[7]=y[27];
    scratch_h=u24(y[31]);scratch_l=u24(y[32]);
    for(i=0u;i<32u;i++) {
        md_shared_scratch_y[32u+i]=lim24(sine[scratch_h&0x7fffu]*2);
        scratch_sum=scratch_l+(uint32_t)oscillator_increments[3];
        scratch_h=((scratch_h&0x7fffu)+(uint32_t)oscillator_increments[2]+(scratch_sum>>24))&0xffffffu;
        scratch_l=scratch_sum&0xffffffu;
    }''')
    change('        if (envelope == 0 && clip_high > 0 && clip_low < 0) {', '        if (0) { /* Full49 must retain the original shared scratch stores. */')
    return '#define TRXBD_DISABLE_QUIET_PATH\n'+text

def bd_helpers(text):
    start=text.index('trxbd_amp_clip.:');end=text.index('.GLOBAL trxbd_osc_shape.;')
    amp=text[start:end]
    assert amp.count('I7=MODIFY (I7,-0x8)(NW);')==amp.count('I7=MODIFY (I7,0x8)(NW);')==1
    amp=amp.replace('I7=MODIFY (I7,-0x8)(NW);','I7=MODIFY (I7,-0x9)(NW);').replace('I7=MODIFY (I7,0x8)(NW);','I7=MODIFY (I7,0x9)(NW);')
    marker='    R10 = PASS R12;';assert amp.count(marker)==1
    amp=amp.replace(marker,'''    R0 = md_shared_scratch_x.;
    R1 = 128;
    R0 = R0 + R1;
    DM(-0xB,I6) = R0;
'''+marker)
    marker='    R6 = MAX(R6,R1);\n    R11 = R5 * R12 (SSF);';assert amp.count(marker)==4
    amp=amp.replace(marker,'''    R6 = MAX(R6,R1);
    R0 = DM(-0xB,I6);
    I4 = R0;
    DM(I4,M5) = R6;
    R1 = 4;
    R0 = R0 + R1;
    DM(-0xB,I6) = R0;
    R11 = R5 * R12 (SSF);''')
    marker='    R0 = MAX(R0,R1);\n    R0 = R0 + R6;';assert amp.count(marker)==4
    amp=amp.replace(marker,'''    R0 = MAX(R0,R1);
    R1 = DM(-0xB,I6);
    R4 = 124;
    R1 = R1 + R4;
    I4 = R1;
    DM(I4,M5) = R0;
    R0 = R0 + R6;''')
    return text[:start]+amp+text[end:]

def sd_source(text):
    def change(old,new):
        nonlocal text
        assert text.count(old)==1,old;text=text.replace(old,new)
    change('void trxsd_render_block_44k1(', 'extern int32_t md_shared_scratch_x[256];\nextern int32_t md_shared_scratch_y[352];\nvoid trxsd_render_block_44k1(')
    change('        if (cursor <= 0xfe0u) {', '        md_shared_scratch_x[64u+i] = x;\n        md_shared_scratch_x[32u+i] = noise[i];\n        if (cursor <= 0xfe0u) {')
    change('        x = trxsd_filter(*(y + 25)', '        if(i==0u) {md_shared_scratch_x[30]=y[27];md_shared_scratch_x[31]=y[28];}\n        x = trxsd_filter(*(y + 25)')
    change('    *(y + 22) = *(v->blk44 + 31);', '''    *(y + 22) = *(v->blk44 + 31);
    /* Original X:3/2 are first filter histories; X:30/31 the second.
     * X:30/31 retain the pre-render second-filter state.
     * X:32..63 retain attenuated noise; X:64..95 pre-transient mix.
     * Y:1/0 retain the two alternating filter coefficients. */
    md_shared_scratch_x[3] = y[25];md_shared_scratch_x[2] = y[26];
    md_shared_scratch_y[1] = fb2;md_shared_scratch_y[0] = feed;''')
    return text

def lower_assembly(text):
    if 'md_agu.:\n' in text:text=replace_function(text,'md_agu',(HOT/'agu.s').read_text())
    arithmetic=(HOT/'accumulator.s').read_text()
    first=arithmetic.index('add56.:');second=arithmetic.index('sub56.:')
    if 'add56.:\n' in text:text=replace_function(text,'add56',arithmetic[first:second])
    if 'sub56.:\n' in text:text=replace_function(text,'sub56',arithmetic[second:])
    return text


def export_bd_decay(text):
    # The existing shared coefficient leaf expects this exact public table.
    # Rename its references and definition together; data words stay unchanged.
    assert text.count('.VAR trxbd_t_decay. =')==1
    return text.replace('trxbd_t_decay.','md_decay_table.').replace(
        '.VAR md_decay_table. =','.GLOBAL md_decay_table.;\n.VAR md_decay_table. =')

