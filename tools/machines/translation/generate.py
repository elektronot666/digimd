"""Batch translate original machine routines; report exact unsupported patterns."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
from digimd import load_firmware, MD_DEVICE, MD_SHA256
from tools.machines.registry import ROOT, machines
from tools.machines.translation.recover import OUT, sha
from tools.machines.translation.dsp import decode, reachable, Lowering, Unsupported
from tools.machines.translation.control import Control

GENERATED=ROOT/'tools/machines/generated'
# Original ROM boundaries, independently recoverable from the adjacent tables.
# The maximum ground-table index is floor(0x3f80/32) == 508.
CONTROL_TABLES={0x2462e8:512,0x24bb90:(0x24e000-0x24bb90)//4}


def control_data(cpu):
    import struct
    lines=['/* Original CPU ROM words, emitted once for every generated engine. */','#include <stdint.h>']
    banks={space:struct.unpack_from('<'+str(n)+'I',(OUT/(space.upper()+'.u32')).read_bytes()) for space,n in [('x',256),('y',352)]}
    for space,values in banks.items():
        lines.append(f'int32_t md_shared_scratch_{space}[{len(values)}] = {{'+','.join(str(v if v<0x800000 else v-0x1000000) for v in values)+'};')
    # Preserve every nonzero boot word in the read-only internal X regions.
    # Synth writes to these regions are deliberately still unsupported.
    values=struct.unpack('<131072I',(OUT/'X.u32').read_bytes())
    lines += ['uint32_t md_boot_x_read(uint32_t address) {']
    for i,v in enumerate(values):
        if (256<=i<0x800 or 0xc00<=i<0x20000) and v:
            lines.append(f'    if(address=={i}u)return {v}u;')
    lines += ['    return 0u;', '}']
    lines += ['uint32_t md_shared_y20 = 0u;', 'void md_runtime_reset(void) {', '    uint32_t i;']
    for space,values in banks.items():
        lines.append(f'    for(i=0u;i<{len(values)}u;i++) md_shared_scratch_{space}[i]=0;')
        for i,v in enumerate(values):
            if v:lines.append(f'    md_shared_scratch_{space}[{i}]={v if v<0x800000 else v-0x1000000};')
    lines += ['    md_shared_y20=0u;', '}']

    for address,n in CONTROL_TABLES.items():
        values=struct.unpack(f'>{n}I',cpu[address-0x200000:address-0x200000+4*n])
        lines += [f'const uint32_t control_{address:x}[{n}] = {{']
        lines += ['    '+', '.join(f'0x{x:08x}u' for x in values[i:i+8])+',' for i in range(0,n,8)]
        lines += ['};']
    lines += ['''uint32_t md_control_tempo_denominator = 3000u;
uint32_t md_divu32(uint32_t numerator, uint32_t denominator) {
    uint32_t quotient, remainder, carry, bit;
    quotient = 0u; remainder = 0u; bit = 32u;
    while (bit != 0u) {
        bit--; carry = remainder >> 31;
        remainder = (remainder << 1) | ((numerator >> bit) & 1u);
        if (carry || remainder >= denominator) {
            remainder -= denominator;
            quotient |= 1u << bit;
        }
    }
    return quotient;
}
uint32_t md_divs32(uint32_t numerator, uint32_t denominator) {
    uint32_t n, d, result;
    n = numerator; d = denominator;
    if ((n & 0x80000000u) != 0u) { n = 0u - n; }
    if ((d & 0x80000000u) != 0u) { d = 0u - d; }
    result = md_divu32(n,d);
    if (((numerator ^ denominator) & 0x80000000u) != 0u) { result = 0u - result; }
    return result;
}''']
    (GENERATED/'control_tables.c').write_text('\n'.join(lines)+'\n')


def memory_helpers(lower,voice,words):
    lines=[]
    if lower.requires_source_image:
        lines += ['''#ifdef MD_DSP_HOST_TEST
extern uint32_t md_source_words[0x1e0000];
#else
#ifndef MD_SOURCE_WORDS_BASE
#error This kernel requires a validated startup immutable-source provider
#endif
#define md_source_words ((const uint32_t *)MD_SOURCE_WORDS_BASE)
#endif
''',f'''static uint32_t md_read_source({voice} *v, uint32_t address) {{
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) {{ value = md_source_words[address - 0x20000u]; }}
    else if (address >= 0x200000u && address < 0xffff80u) {{ value = 0u; }}
    else {{ v->memory_fault = 1u; }}
    return value;
}}''']
    for address,values in lower.tables.items():
        lines += [f'extern const uint32_t source_{address:x}[{len(values)}];']
    lines += ['''static uint32_t md_agu(uint32_t *r, uint32_t m, int32_t step, uint32_t pre) {
    uint32_t old, next, mask, base, result;
    int32_t offset;
    old = *r; next = (old + (uint32_t)step) & 0xffffffu;
    if (m != 0xffffffu) {
        /* Fill below the highest set bit, exactly the original loop's mask.
         * Keep its one-bit minimum for m==0/1 and its single-wrap arithmetic
         * for every stride, including non-power-of-two delay lengths. */
        mask = 1u;
        if (m > 1u) {
            mask = m;
            mask |= mask >> 1;
            mask |= mask >> 2;
            mask |= mask >> 4;
            mask |= mask >> 8;
            mask |= mask >> 16;
        }
        base = old & ~mask;
        offset = (int32_t)(old & mask) + step;
        if (offset < 0) { offset = offset + (int32_t)m + 1; }
        else if ((uint32_t)offset > m) { offset = offset - (int32_t)m - 1; }
        next = base + (uint32_t)offset;
    }
    *r = next;
    result = pre ? next : old;
    return result;
}
static uint32_t md_index(uint32_t address, uint32_t modifier, int32_t step) {
    uint32_t cursor[1];
    uint32_t result;
    cursor[0] = address;
    result = md_agu(cursor,modifier,step,1u);
    return result;
}
''']
    for space in ('x','y'):
        lines += [f'static uint32_t md_read_{space}({voice} *v, int32_t *scratch, uint32_t address, const int32_t *sine) {{',
                  '    uint32_t value;', '    value = 0u; address = address & 0xffffffu;',
                  f'    if (address < {lower.scratch_x_words if space=="x" else lower.scratch_y_words}u) {{ value = u24(*(scratch + address)); }}']
        if space=='x':
            lines += ['    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }']
        if lower.buffered_output and space=='y':
            lines += ['    else if (address == 0x1ffu) { value = 0xfdb42bu; }']
        if lower.private_memory:
            lines += [f'    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) {{ value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }}']
            if space=='x':lines += ['    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { value = u24(v->x_private[address - 0x800u - v->virtual_voice * 64u]); }']
        if space=='y':
            lines += [f'    else if (address >= 0x800u && address < {0x800+words}u) {{ value = u24(*(v->y + (address - 0x800u))); }}',
                      '    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }']
        lines += ['    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }']
        for address,values in lower.tables.items():
            lines += [f'    else if (address >= {address}u && address < {address+len(values)}u) {{ value = *(source_{address:x} + (address - {address}u)); }}']
        if lower.requires_source_image:
            lines += ['    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }']
            lines += ['    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }']
        lines += ['    else { v->memory_fault = 1u; }','    return value;', '}',
                  f'static void md_write_{space}({voice} *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {{',
                  '    address = address & 0xffffffu;',
                  f'    if (address < {lower.scratch_x_words if space=="x" else lower.scratch_y_words}u) {{ *(scratch + address) = sx24((int32_t)value); }}']
        if lower.buffered_output and space=='y':
            lines += ['    else if (address == 0x1ffu) { if (u24((int32_t)value) != 0xfdb42bu) { v->memory_fault = 1u; } }']
        if lower.private_memory:
            lines += ['    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }']
            if space=='x':lines += ['    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { v->x_private[address - 0x800u - v->virtual_voice * 64u] = sx24((int32_t)value); }']
        if space=='y':
            lines += [f'    else if (address >= 0x800u && address < {0x800+words}u) {{ *(v->y + (address - 0x800u)) = sx24((int32_t)value); }}',
                      '    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }']
        lines += ['    else { v->memory_fault = 1u; }', '}']
    if lower.buffered_output:
        lines=[line for line in lines if 'else if (address >= 0x100u' not in line]
    text=('\n'.join(lines)+'\n')
    for space in ('x','y'):
        text+=f"\nstatic uint32_t md_read_{space}_raw({voice} *v,int32_t *scratch,uint32_t address,const int32_t *sine) {{\n    uint32_t value;\n    address = address & 0xffffffu; value=0u;\n    if (address < 0xffff80u) value=md_read_{space}(v,scratch,address,sine);\n    return value;\n}}\n"
    if lower.requires_source_image:
        text+=f"\nstatic uint32_t md_read_source_raw({voice} *v,uint32_t address) {{\n    uint32_t value;\n    address = address & 0xffffffu; value=0u;\n    if (address < 0xffff80u) value=md_read_source(v,address);\n    return value;\n}}\n"
    if lower.private_memory:
        text=text.replace('address >= 0x800u && address < 2112u','address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u').replace('address - 0x800u)', 'address - 0x800u - v->virtual_voice*64u)')
    text=text.replace('v->memory_fault = 1u;', 'MD_MEMORY_FAULT(v,address);')
    return '''#ifdef MD_DSP_HOST_TEST
#include <stdio.h>
#include <stdlib.h>
#define MD_MEMORY_FAULT(v,a) { fprintf(stderr,"unmapped %x\\n",(unsigned)(a)); abort(); }
#else
#define MD_MEMORY_FAULT(v,a) { (v)->memory_fault = 1u; }
#endif
'''+text


def wrapper(prefix, voice, words, functions, data, lower):
    header=f'''/* Generated from original OS 1.63 by tools.machines.translation.generate.
 * No runtime opcode decoder. Verification status lives in the progress ledger. */
#include <stdint.h>
#include "../md_runtime_compat.h"
extern int32_t md_shared_scratch_x[256];
extern int32_t md_shared_scratch_y[352];
extern uint32_t md_shared_y20;
extern uint32_t md_boot_x_read(uint32_t);
extern void md_runtime_reset(void);
typedef struct {{
    int32_t y[{words}];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;
    {'int32_t x_private[64]; int32_t external[1536]; uint32_t virtual_voice; int32_t scratch_y20_history;' if lower.private_memory else ''}
}} {voice};
'''
    tail=f'''
void {prefix}_update_coeffs(const uint32_t *p, {voice} *v, uint32_t trigger) {{
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) {{ coefficients[i] = 0; }}
    coefficients[0] = (int32_t)trigger;
    count = {prefix}_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) {{ v->y[i] = sx24(coefficients[i]); }}
    if (trigger && v->active == 0u) {{ {prefix}_dsp_init(v, 0); }}
}}
void {prefix}_voice_init({voice} *v) {{
    uint32_t i;
    for (i = 0u; i < {words}u; i++) {{ *(v->y + i) = 0; }}
    for (i = 0u; i < 32u; i++) {{ *(v->blk44 + i) = 0; }}
    for (i = 0u; i < 8u; i++) {{ *(v->last_p + i) = 0xffffffffu; }}
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;
    {'for (i=0u;i<64u;i++) v->x_private[i] = 0; for (i=0u;i<1536u;i++) v->external[i] = 0; v->virtual_voice = 0u; v->scratch_y20_history = 0;' if lower.private_memory else ''}

}}
void {prefix}_voice_trigger({voice} *v) {{
    {prefix}_dsp_trigger(v, 0);
    v->active = 1u;
}}
#ifndef MD_DSP_HOST_TEST
extern uint32_t {prefix}_resample_run({voice} *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE {voice}
#define MD_RENDER48 {prefix}_render_block_48k_float
#define MD_RENDER44 {prefix}_render_block_44k1
#define MD_RESAMPLE_RUN {prefix}_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE {voice}
#define MD_SLOT_INIT {prefix}_slot_init
#define MD_SLOT_PROCESS {prefix}_slot_process
#define MD_VOICE_INIT {prefix}_voice_init
#define MD_COEFFICIENTS {prefix}_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER {prefix}_update_coeffs
#define MD_VOICE_TRIGGER {prefix}_voice_trigger
#define MD_RENDER44 {prefix}_render_block_44k1
#define MD_RENDER48 {prefix}_render_block_48k_float
#include "../engine_slot.inc"
#endif
'''
    text=header+data+'\n'+memory_helpers(lower,voice,words)+'\n'.join(functions)+tail
    return "\n".join(line.rstrip() for line in text.splitlines())+"\n"


def generate(out=OUT):
    recovery=json.loads((out/'recovery.json').read_text())
    for name,digest in recovery['artifacts'].items():
        if sha(out/name)!=digest: raise ValueError('Changed decoder artifact: '+name)
    if (out/'raw-guard.u32').read_bytes()!=bytes(1048576*4):
        raise ValueError('Raw namespace specialization requires the complete zero boot guard')
    namespace=json.loads((out/'ram-namespace.json').read_text())
    if namespace != dict(status='PASS',opcode='0xf09bd8',ram_x0=0x123456,ram_y0=0x234567,peripheral=0x654321,zero_guard_words=1048576):
        raise ValueError('Missing original parallel RAM namespace proof')
    fw=load_firmware(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx',MD_DEVICE)
    if fw.digest!=MD_SHA256: raise ValueError('Require OS 1.63')
    from digimd_dsp56300 import parse_dsp_image
    cpu=fw.section_bytes(0); instructions=decode(out/'decoded.tsv');image=parse_dsp_image(fw.section_bytes(1))
    report=dict(schema='digimd.translation.batch.v1',recovery_sha256=sha(out/'recovery.json'),machines=[])
    GENERATED.mkdir(parents=True,exist_ok=True)
    control_data(cpu)
    shared_dsp_tables={}
    for m in machines():
        prefix=m['name'].lower().replace('-','');voice=prefix.title()+'Voice'
        result=dict(name=m['name'],md_id=m['md_id'],selector=m['selector'],entries=m['original']['dsp_entries'],
                    coefficient_entry=m['original']['coefficient_entry'],status='unsupported',blockers=[],
                    original_reference_parity='pending',native_build='pending',integration='pending',packaging='pending')
        if m['name'] in ('TRX-BD','TRX-SD'):
            result['status']='existing-authentic-native'
            report['machines'].append(result)
            continue
        lower=Lowering(instructions,image);lower.buffered_output=m['name'] in ('P-I-RC','P-I-CC','P-I-HH');lower.scratch_x_words=256 if m['family'] in ('E12','P-I') else 128;lower.scratch_y_words=352 if lower.buffered_output else 256 if m['family'] in ('P-I','E12') else 128;functions=[];coverage={};ops=Counter()
        if m['family']=='GND':lower.scratch_x_words=32;lower.scratch_y_words=1
        if m['name']=='TRX-CP':lower.scratch_x_words=256;lower.scratch_y_words=256
        lower.outline_native_groups=m['family']=='E12' or (m['family']=='P-I' and m['name']!='P-I-MA') or m['name'] in ('TRX-CP','TRX-RS','TRX-CB','TRX-CH','TRX-OH','TRX-CY','TRX-MA','EFM-CY')
        if m['family']=='E12':lower.requires_source_image=True
        if m['family']=='P-I' and m['name']!='P-I-MA':lower.private_memory=True;lower.requires_source_image=True
        for kind,entry in m['original']['dsp_entries'].items():
            try:
                original=reachable(instructions,int(entry,16))
                coverage[kind]=len(original)
                ops.update(i.op for i in original.values())
                fn=prefix+('_render_block_44k1' if kind=='render' else '_dsp_'+kind)
                text,_=lower.function(fn,voice,int(entry,16));functions.append(text)
            except Unsupported as e:
                result['blockers'].append(dict(phase=kind,reason=str(e)))
        control=Control(cpu,int(m['original']['coefficient_entry'],16),CONTROL_TABLES)
        try:
            text,original=control.function(prefix+'_compute_coeffs');functions.append(text)
            coverage['coefficients']=len(original)
        except Unsupported as e:
            result['blockers'].append(dict(phase='coefficients',reason=str(e)))
        for address,values in lower.tables.items():
            if address in shared_dsp_tables and shared_dsp_tables[address]!=values:raise ValueError('Conflicting original table')
            shared_dsp_tables[address]=values
        result.update(reachable_instruction_counts=coverage,dsp_op_histogram=dict(ops),slot_offsets=sorted(lower.slot_words))
        if not result['blockers']:
            words=64 if lower.private_memory else max(lower.slot_words or {0})+1
            text=wrapper(prefix,voice,words,functions,control.data(),lower)
            dest=GENERATED/(prefix+'.c');dest.write_text(text)
            result.update(status='generated-unverified',source=str(dest.relative_to(ROOT)),source_sha256=sha(dest),
                          prefix=prefix,voice_type=voice,original_slot_words=words,state_bytes=words*4+180+(6408 if lower.private_memory else 0),
                          stream_byte_offset=words*4,immutable_control_bytes=sum(control.tables.values())*4,requires_source_image=lower.requires_source_image)
            result.update(buffered_output=lower.buffered_output,private_memory=lower.private_memory,memory_fault_byte_offset=words*4+176)
        report['machines'].append(result)
        print(m['name'],result['status'], '; '.join(b['reason'] for b in result['blockers']),flush=True)
    with (GENERATED/'control_tables.c').open('a') as shared:
        shared.write('\n/* Deduplicated original DSP load-record words. */\n')
        for address,values in sorted(shared_dsp_tables.items()):
            shared.write(f'const uint32_t source_{address:x}[{len(values)}] = {{\n')
            for i in range(0,len(values),8):shared.write('    '+', '.join(f'0x{x:06x}u' for x in values[i:i+8])+',\n')
            shared.write('};\n')
    report['shared_dsp_table_bytes']=sum(map(len,shared_dsp_tables.values()))*4
    report['generated_count']=sum(m['status']=='generated-unverified' for m in report['machines'])
    report['sources_sha256']={str(p.relative_to(ROOT)):sha(p) for p in Path(__file__).parent.glob('*.py')}
    (out/'batch.json').write_text(json.dumps(report,indent=2)+'\n')
    return report


if __name__=='__main__':
    p=argparse.ArgumentParser(__doc__);p.add_argument('--out',type=Path,default=OUT)
    generate(p.parse_args().out)
