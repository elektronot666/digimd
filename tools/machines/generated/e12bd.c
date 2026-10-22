/* Generated from original OS 1.63 by tools.machines.translation.generate.
 * No runtime opcode decoder. Verification status lives in the progress ledger. */
#include <stdint.h>
#include "../md_runtime_compat.h"
extern int32_t md_shared_scratch_x[256];
extern int32_t md_shared_scratch_y[352];
extern uint32_t md_shared_y20;
extern uint32_t md_boot_x_read(uint32_t);
extern void md_runtime_reset(void);
typedef struct {
    int32_t y[35];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} E12BdVoice;
extern uint32_t md_control_tempo_denominator;
extern uint32_t md_divs32(uint32_t, uint32_t);
extern uint32_t md_divu32(uint32_t, uint32_t);
extern const uint32_t control_24bb90[2332];

#ifdef MD_DSP_HOST_TEST
#include <stdio.h>
#include <stdlib.h>
#define MD_MEMORY_FAULT(v,a) { fprintf(stderr,"unmapped %x\n",(unsigned)(a)); abort(); }
#else
#define MD_MEMORY_FAULT(v,a) { (v)->memory_fault = 1u; }
#endif
#ifdef MD_DSP_HOST_TEST
extern uint32_t md_source_words[0x1e0000];
#else
#ifndef MD_SOURCE_WORDS_BASE
#error This kernel requires a validated startup immutable-source provider
#endif
#define md_source_words ((const uint32_t *)MD_SOURCE_WORDS_BASE)
#endif

static uint32_t md_read_source(E12BdVoice *v, uint32_t address) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) { value = md_source_words[address - 0x20000u]; }
    else if (address >= 0x200000u && address < 0xffff80u) { value = 0u; }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static uint32_t md_agu(uint32_t *r, uint32_t m, int32_t step, uint32_t pre) {
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

static uint32_t md_read_x(E12BdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(E12BdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(E12BdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2083u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(E12BdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2083u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(E12BdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(E12BdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(E12BdVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef E12BD_DSP_INIT_EXEC_DEFINED
#define E12BD_DSP_INIT_EXEC_DEFINED
typedef struct {
    int32_t accum[3];
    uint32_t lows[3];
    uint32_t words[4];
    uint32_t addresses[24];
    uint32_t flags[5];
    uint32_t long_address[3];
    uint32_t parallel_words[6];
    uint32_t shift_count[1];
    uint32_t repeat;
} E12Bd_Dsp_InitExec;
#endif
static void e12bd_dsp_init_group_1036b6(E12Bd_Dsp_InitExec *exec, E12BdVoice *v, const int32_t *sine) {
L_1036b6: /* move    #>$7,exec->words[0] */
    exec->words[0] = (0x7u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1036b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036b8: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036ba: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1036bau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036bb: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036bd: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036bdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036bf: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036bfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036c1: /* move    exec->words[0],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036c1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12bd_dsp_init(E12BdVoice *v, const int32_t *sine) {
    E12Bd_Dsp_InitExec execution;
    E12Bd_Dsp_InitExec *exec;
    exec = &execution;
    exec->accum[0] = 0;
    exec->accum[1] = 0;
    exec->accum[2] = 0;
    exec->lows[0] = 0;
    exec->lows[1] = 0;
    exec->lows[2] = 0;
    exec->words[0] = 0;
    exec->words[1] = 0;
    exec->words[2] = 0;
    exec->words[3] = 0;
    exec->addresses[0] = 0u;
    exec->addresses[1] = 0u;
    exec->addresses[2] = 0u;
    exec->addresses[3] = 0u;
    exec->addresses[4] = 0u;
    exec->addresses[5] = 0u;
    exec->addresses[6] = 0x800u;
    exec->addresses[7] = 0x100u;
    exec->addresses[8] = 0xffffffu;
    exec->addresses[9] = 0xffffffu;
    exec->addresses[10] = 0xffffffu;
    exec->addresses[11] = 0xffffffu;
    exec->addresses[12] = 0xffffffu;
    exec->addresses[13] = 0xffffffu;
    exec->addresses[14] = 0xffffffu;
    exec->addresses[15] = 0xffffffu;
    exec->addresses[16] = 0u;
    exec->addresses[17] = 0u;
    exec->addresses[18] = 0u;
    exec->addresses[19] = 0u;
    exec->addresses[20] = 0u;
    exec->addresses[21] = 0u;
    exec->addresses[22] = 0u;
    exec->addresses[23] = 0u;
    exec->flags[0] = 0;
    exec->flags[1] = 0;
    exec->flags[2] = 0;
    exec->flags[3] = 0;
    exec->flags[4] = 0;
    exec->long_address[0] = 0;
    exec->long_address[1] = 0;
    exec->long_address[2] = 0;
    exec->parallel_words[0] = 0;
    exec->parallel_words[1] = 0;
    exec->parallel_words[2] = 0;
    exec->parallel_words[3] = 0;
    exec->parallel_words[4] = 0;
    exec->parallel_words[5] = 0;
    exec->repeat = 0u;
L_1036b6:
    e12bd_dsp_init_group_1036b6(exec,v,sine);
L_1036c3: /* rts */
    return;}

#ifndef E12BD_DSP_TRIGGER_EXEC_DEFINED
#define E12BD_DSP_TRIGGER_EXEC_DEFINED
typedef struct {
    int32_t accum[3];
    uint32_t lows[3];
    uint32_t words[4];
    uint32_t addresses[24];
    uint32_t flags[5];
    uint32_t long_address[3];
    uint32_t parallel_words[6];
    uint32_t shift_count[1];
    uint32_t repeat;
} E12Bd_Dsp_TriggerExec;
#endif
static void e12bd_dsp_trigger_group_103819(E12Bd_Dsp_TriggerExec *exec, E12BdVoice *v, const int32_t *sine) {
L_103819: /* move    y:(exec->addresses[6]+$f),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103819u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10381a: /* mpyi    #>$3,exec->words[0],a */
    mpy_ss32(sx24((int32_t)(0x3u)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10381au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10381c: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10381cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10381d: /* move    exec->words[1],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10381du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10381e: /* move    exec->words[1],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10381eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10381f: /* move    exec->words[1],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10381fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103820: /* move    a0,a1 */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((exec->lows[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103820u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103821: /* asr     #$1,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103821u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_dsp_trigger_group_103822(E12Bd_Dsp_TriggerExec *exec, E12BdVoice *v, const int32_t *sine) {
L_103822: /* add     #>$103d7b,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x103d7bu)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103822u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103824: /* move    exec->words[1],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103824u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103825: /* move    a1,exec->addresses[0] */
    exec->addresses[0] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103825u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103826: /* move    #>$7fffff,b */
    exec->accum[1] = sx24((int32_t)(0x7fffffu)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103826u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103828: /* asr     #$4,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 4u; exec->lows[1] = ((exec->lows[2] >> 4u) | ((uint32_t)exec->accum[2] << 20u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103828u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103829: /* move    y:(exec->addresses[6]+$4),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 4u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103829u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10382a: /* move    b,y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10382au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10382b: /* move    a,y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x10382bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_dsp_trigger_group_10382c(E12Bd_Dsp_TriggerExec *exec, E12BdVoice *v, const int32_t *sine) {
L_10382c: /* move    y:(exec->addresses[0])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10382cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10382d: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10382du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10382e: /* move    y:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10382eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10382f: /* move    exec->words[2],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x10382fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103830: /* move    y:(exec->addresses[6]+$6),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103830u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103831: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103831u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103832: /* move    y:(exec->addresses[0])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103832u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103833: /* move    exec->words[0],y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103833u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_dsp_trigger_group_103834(E12Bd_Dsp_TriggerExec *exec, E12BdVoice *v, const int32_t *sine) {
L_103834: /* move    a,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x103834u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103835: /* move    y:(exec->addresses[6]+$9),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103835u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103836: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103836u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12bd_dsp_trigger(E12BdVoice *v, const int32_t *sine) {
    E12Bd_Dsp_TriggerExec execution;
    E12Bd_Dsp_TriggerExec *exec;
    exec = &execution;
    exec->accum[0] = 0;
    exec->accum[1] = 0;
    exec->accum[2] = 0;
    exec->lows[0] = 0;
    exec->lows[1] = 0;
    exec->lows[2] = 0;
    exec->words[0] = 0;
    exec->words[1] = 0;
    exec->words[2] = 0;
    exec->words[3] = 0;
    exec->addresses[0] = 0u;
    exec->addresses[1] = 0u;
    exec->addresses[2] = 0u;
    exec->addresses[3] = 0u;
    exec->addresses[4] = 0u;
    exec->addresses[5] = 0u;
    exec->addresses[6] = 0x800u;
    exec->addresses[7] = 0x100u;
    exec->addresses[8] = 0xffffffu;
    exec->addresses[9] = 0xffffffu;
    exec->addresses[10] = 0xffffffu;
    exec->addresses[11] = 0xffffffu;
    exec->addresses[12] = 0xffffffu;
    exec->addresses[13] = 0xffffffu;
    exec->addresses[14] = 0xffffffu;
    exec->addresses[15] = 0xffffffu;
    exec->addresses[16] = 0u;
    exec->addresses[17] = 0u;
    exec->addresses[18] = 0u;
    exec->addresses[19] = 0u;
    exec->addresses[20] = 0u;
    exec->addresses[21] = 0u;
    exec->addresses[22] = 0u;
    exec->addresses[23] = 0u;
    exec->flags[0] = 0;
    exec->flags[1] = 0;
    exec->flags[2] = 0;
    exec->flags[3] = 0;
    exec->flags[4] = 0;
    exec->long_address[0] = 0;
    exec->long_address[1] = 0;
    exec->long_address[2] = 0;
    exec->parallel_words[0] = 0;
    exec->parallel_words[1] = 0;
    exec->parallel_words[2] = 0;
    exec->parallel_words[3] = 0;
    exec->parallel_words[4] = 0;
    exec->parallel_words[5] = 0;
    exec->repeat = 0u;
    uint32_t clear_scratch;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_103819:
    e12bd_dsp_trigger_group_103819(exec,v,sine);
L_103822:
    e12bd_dsp_trigger_group_103822(exec,v,sine);
L_10382c:
    e12bd_dsp_trigger_group_10382c(exec,v,sine);
L_103834:
    e12bd_dsp_trigger_group_103834(exec,v,sine);
L_103837: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

#ifndef E12BD_RENDER_BLOCK_44K1_EXEC_DEFINED
#define E12BD_RENDER_BLOCK_44K1_EXEC_DEFINED
typedef struct {
    int32_t accum[3];
    uint32_t lows[3];
    uint32_t words[4];
    uint32_t addresses[24];
    uint32_t flags[5];
    uint32_t long_address[3];
    uint32_t parallel_words[6];
    uint32_t shift_count[1];
    uint32_t repeat;
} E12Bd_Render_Block_44K1Exec;
#endif
static void e12bd_render_block_44k1_group_103945(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103945: /* move    y:(exec->addresses[6]+$22),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 34u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103945u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103946: /* move    y:(exec->addresses[6]+$5),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103946u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103947: /* move    exec->words[0],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103947u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103948: /* add     #>$1,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x1u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103948u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10394a: /* move    y:(exec->addresses[6]+$13),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10394au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10394b: /* move    y:(exec->addresses[6]+$7),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10394bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10394c: /* mpy     exec->words[2],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10394cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10394d: /* move    a,y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x10394du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_10394e(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_10394e: /* move    y:(exec->addresses[6]+$8),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10394eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10394f: /* move    b,y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10394fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103950: /* move    y:(exec->addresses[6]+$15),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 21u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103950u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103951: /* asl     #$4,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 4u) | (exec->lows[2] >> 20u)); exec->lows[1] = (exec->lows[2] << 4u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103951u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103952: /* nop */

    MD_TRANSLATION_TRACE(0x103952u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103953: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103953u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103954: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103954u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103955: /* clr     b */
    exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103955u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103956(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103956: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103956u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103957: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103957u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103958: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103958u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103959: /* move    exec->words[0],b0 */
    exec->lows[1] = (exec->words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103959u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10395a: /* asr     #$18,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[0] = (uint32_t)(exec->accum[2] >> 0u) & 0xffffffu; exec->accum[0] = exec->accum[2] >> 24u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10395au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10395b: /* add     a,b */
    add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10395bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10395c: /* asl     #$3,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 3u) | (exec->lows[2] >> 21u)); exec->lows[1] = (exec->lows[2] << 3u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10395cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10395d: /* cmp     #>$4,b */
    exec->accum[2] = sx24((int32_t)(0x4u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x10395du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103961(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103961: /* move    #>$4,b */
    exec->accum[1] = sx24((int32_t)(0x4u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103961u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103963(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103963: /* tst     b */
    exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103963u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103964: /* clr     b               ifmi */
    if (exec->flags[3]) { exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); }
    MD_TRANSLATION_TRACE(0x103964u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103965: /* move    #$a0,exec->addresses[2] */
    exec->addresses[2] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103965u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103966: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103966u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103967: /* move    #$1,exec->addresses[12] */
    exec->addresses[12] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103967u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103968: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103968u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103969: /* move    y:(exec->addresses[6]+$11),a0 */
    exec->lows[0] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103969u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10396a: /* move    b0,exec->words[0] */
    exec->words[0] = (exec->lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10396au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_10396b(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_10396b: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10396bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10396c: /* move    #>$147200,exec->words[3] */
    exec->words[3] = (0x147200u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10396cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10396e: /* move    #>$200,exec->words[2] */
    exec->words[2] = (0x200u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10396eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103970: /* move    exec->words[2],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x103970u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103971: /* move    exec->words[3],y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103971u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103974(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103974: /* move    a0,exec->words[3] */
    exec->words[3] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103974u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103975: /* mpyuu   exec->words[2],exec->words[3],b */
    mpy_uu32(exec->words[2], exec->words[3], &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103975u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103976: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103976u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103977: /* and     #>$fffffc,b */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | (u24(exec->accum[1]) & 0xfffffcu));
    MD_TRANSLATION_TRACE(0x103977u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103979: /* add     exec->words[2],b            a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103979u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10397a: /* add     x,a             y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10397au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10397b: /* move    b1,y:(exec->addresses[2])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x10397bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_10397c(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_10397c: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10397cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10397e: /* move    y:(exec->addresses[6]+$1c),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10397eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10397f: /* move    a1,x:>$fe */
    *(md_shared_scratch_x + 254u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10397fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103981: /* move    y:(exec->addresses[6]+$10),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103981u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103982: /* move    exec->words[3],x:>$ff */
    *(md_shared_scratch_x + 255u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x103982u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103984: /* add     exec->words[3],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103984u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103985: /* cmp     exec->words[2],a */
    exec->accum[2] = sx24((int32_t)(exec->words[2])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103985u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103988(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103988: /* move    y:(exec->addresses[6]+$13),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 19u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103988u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103989: /* nop */

    MD_TRANSLATION_TRACE(0x103989u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_10398a(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_10398a: /* move    a0,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x10398au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10398b: /* move    a1,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10398bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10398c: /* move    y:(exec->addresses[6]+$1e),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 30u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10398cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10398d: /* add     #>$20,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x20u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10398du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10398f: /* move    y:(exec->addresses[6]+$a),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10398fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103990: /* cmp     exec->words[0],a */
    exec->accum[2] = sx24((int32_t)(exec->words[0])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103990u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103993(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103993: /* move    y:(exec->addresses[6]+$1f),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 31u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103993u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103994: /* add     #<$1,b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(0x1u)), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103994u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103995: /* move    y:(exec->addresses[6]+$e),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103995u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103996: /* cmp     exec->words[1],b */
    exec->accum[2] = sx24((int32_t)(exec->words[1])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103996u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103999(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103999: /* move    b,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x103999u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10399a: /* sub     exec->words[0],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10399au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10399b: /* and     #<$1f,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x1fu));
    MD_TRANSLATION_TRACE(0x10399bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10399c: /* move    #>$bf,b */
    exec->accum[1] = sx24((int32_t)(0xbfu)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10399cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10399e: /* sub     a,b */
    sub56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10399eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10399f: /* move    y:(exec->addresses[6]+$13),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10399fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a0: /* move    b,exec->addresses[0] */
    exec->addresses[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039a0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a1: /* move    y:(exec->addresses[6]+$6),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039a1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039a2(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039a2: /* mpy     exec->words[3],exec->words[1],b         a,exec->words[0] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039a2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a3: /* move    #>$0,exec->words[1] */
    exec->words[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039a3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a5: /* move    exec->words[1],x:>$c0 */
    *(md_shared_scratch_x + 192u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1039a5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a7: /* move    x:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039a7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a8: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039a8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039a9: /* nop */

    MD_TRANSLATION_TRACE(0x1039a9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039aa: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039aau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039ab: /* move    exec->words[1],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1039abu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039ac(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039ac: /* move    exec->words[1],x:>$ff */
    *(md_shared_scratch_x + 255u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1039acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039b0(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039b0: /* move    x:(exec->addresses[0]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1039b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039b1: /* sub     exec->words[1],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039b1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039b2: /* nop */

    MD_TRANSLATION_TRACE(0x1039b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039b3: /* move    a,x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1039b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039b4(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039b4: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039b4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039b5: /* move    y:(exec->addresses[6]+$a),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 10u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1039b5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039b6: /* cmp     #>$20,b */
    exec->accum[2] = sx24((int32_t)(0x20u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x1039b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039ba(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039ba: /* move    exec->words[0],b */
    exec->accum[1] = sx24((int32_t)(exec->words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1039bau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039bb: /* sub     #>$20,b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(0x20u)), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039bd: /* neg     b */
    exec->accum[1] = (int32_t)(0u - (uint32_t)exec->accum[1] - (uint32_t)(exec->lows[1] != 0u)); exec->lows[1] = (0u - exec->lows[1]) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039bdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039be: /* move    #$a0,exec->addresses[0] */
    exec->addresses[0] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039beu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039bf: /* move    b,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039bfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039c0: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039c0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039c1: /* move    exec->words[0],a */
    exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1039c1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039c4(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039c4: /* move    exec->words[1],x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1039c4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039c5(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039c5: /* nop */

    MD_TRANSLATION_TRACE(0x1039c5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039c6(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039c6: /* move    a,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x1039c6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039c7(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039c7: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039c7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039c8: /* move    #$2,exec->addresses[5] */
    exec->addresses[5] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039c8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039c9: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1039c9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039cb: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039cbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039cc: /* move    y:(exec->addresses[6]+$12),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039ccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039cd: /* add     exec->words[2],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039cdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039ce: /* move    #$3,exec->addresses[20] */
    exec->addresses[20] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039ceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039cf: /* move    #$3,exec->addresses[21] */
    exec->addresses[21] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039cfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039d0(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039d0: /* move    a1,exec->addresses[3] */
    exec->addresses[3] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039d0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039d1: /* move    #$87,exec->addresses[12] */
    exec->addresses[12] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039d1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039d2: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1039d2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039d4: /* btst    #$0,a */
    exec->flags[4] = (((u24(lim24(exec->accum[0]))) >> 0u) & 1u);
    MD_TRANSLATION_TRACE(0x1039d4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039d7(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039d7: /* move    #$87,exec->addresses[4] */
    exec->addresses[4] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039d7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039d8: /* move    #$1,exec->addresses[5] */
    exec->addresses[5] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039d8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039db(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039db: /* move    x:(exec->addresses[3])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1039dbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039dc: /* move    x:(exec->addresses[3])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1039dcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039dd: /* move    a1,x:(exec->addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x1039ddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039de: /* lsl     #$c,a */
    exec->flags[4] = (u24(exec->accum[0]) >> 12u) & 1u; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((u24(exec->accum[0]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[0]) == 0u); exec->flags[3] = ((u24(exec->accum[0]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x1039deu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039df: /* move    b1,x:(exec->addresses[5])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x1039dfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e0: /* lsl     #$c,b */
    exec->flags[4] = (u24(exec->accum[1]) >> 12u) & 1u; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(exec->accum[1]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[1]) == 0u); exec->flags[3] = ((u24(exec->accum[1]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x1039e0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e1: /* move    a1,x:(exec->addresses[4])+exec->addresses[20] */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x1039e1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e2: /* move    b1,x:(exec->addresses[5])+exec->addresses[21] */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], sx24((int32_t)exec->addresses[21]), 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x1039e2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039e3(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039e3: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039e3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e5: /* move    #>$ffffff,exec->addresses[13] */
    exec->addresses[13] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e7: /* move    y:(exec->addresses[6]+$3),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e8: /* move    y:(exec->addresses[6]+$15),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039e8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039e9: /* move    y:(exec->addresses[6]+$21),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 33u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039e9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039ea: /* move    y:(exec->addresses[6]+$20),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 32u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039eau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039eb: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039ebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039ee(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039ee: /* mpy     exec->words[2],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039eeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039ef: /* mpy     exec->words[3],exec->words[1],a */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039efu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f0: /* move    b,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039f0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f1: /* mpy     exec->words[3],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039f1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f2: /* asl     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039f2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f3: /* add     a,b */
    add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1039f3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f4: /* move    a,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f5: /* move    b,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1039f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_1039f6(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_1039f6: /* move    #$a0,exec->addresses[2] */
    exec->addresses[2] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f7: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f8: /* move    #$0,exec->addresses[3] */
    exec->addresses[3] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039f9: /* move    exec->words[3],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1039f9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039fa: /* move    exec->words[2],y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x1039fau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039fb: /* move    y:(exec->addresses[2])+,exec->addresses[4] */
    exec->addresses[4] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039fbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039fc: /* move    y:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039fd: /* move    #$c8,exec->addresses[0] */
    exec->addresses[0] = (0xc8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1039fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103a00(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103a00: /* move    x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a01: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a02: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a02u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a03: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a04: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[2]),exec->addresses[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a05: /* move    y:(exec->addresses[2])+,exec->addresses[4] */
    exec->addresses[4] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a06: /* move    b,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a06u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a07: /* mpy     exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103a08(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103a08: /* asl     #$4,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 4u) | (exec->lows[2] >> 20u)); exec->lows[1] = (exec->lows[2] << 4u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a09: /* move    y:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a0a: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103a0au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12bd_render_block_44k1_group_103a0b(E12Bd_Render_Block_44K1Exec *exec, E12BdVoice *v, const int32_t *sine) {
L_103a0b: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a0d: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12bd_render_block_44k1(E12BdVoice *v, const int32_t *sine) {
    E12Bd_Render_Block_44K1Exec execution;
    E12Bd_Render_Block_44K1Exec *exec;
    exec = &execution;
    exec->accum[0] = 0;
    exec->accum[1] = 0;
    exec->accum[2] = 0;
    exec->lows[0] = 0;
    exec->lows[1] = 0;
    exec->lows[2] = 0;
    exec->words[0] = 0;
    exec->words[1] = 0;
    exec->words[2] = 0;
    exec->words[3] = 0;
    exec->addresses[0] = 0u;
    exec->addresses[1] = 0u;
    exec->addresses[2] = 0u;
    exec->addresses[3] = 0u;
    exec->addresses[4] = 0u;
    exec->addresses[5] = 0u;
    exec->addresses[6] = 0x800u;
    exec->addresses[7] = 0x100u;
    exec->addresses[8] = 0xffffffu;
    exec->addresses[9] = 0xffffffu;
    exec->addresses[10] = 0xffffffu;
    exec->addresses[11] = 0xffffffu;
    exec->addresses[12] = 0xffffffu;
    exec->addresses[13] = 0xffffffu;
    exec->addresses[14] = 0xffffffu;
    exec->addresses[15] = 0xffffffu;
    exec->addresses[16] = 0u;
    exec->addresses[17] = 0u;
    exec->addresses[18] = 0u;
    exec->addresses[19] = 0u;
    exec->addresses[20] = 0u;
    exec->addresses[21] = 0u;
    exec->addresses[22] = 0u;
    exec->addresses[23] = 0u;
    exec->flags[0] = 0;
    exec->flags[1] = 0;
    exec->flags[2] = 0;
    exec->flags[3] = 0;
    exec->flags[4] = 0;
    exec->long_address[0] = 0;
    exec->long_address[1] = 0;
    exec->long_address[2] = 0;
    exec->parallel_words[0] = 0;
    exec->parallel_words[1] = 0;
    exec->parallel_words[2] = 0;
    exec->parallel_words[3] = 0;
    exec->parallel_words[4] = 0;
    exec->parallel_words[5] = 0;
    exec->repeat = 0u;
    uint32_t clear_scratch;
    uint32_t loop_103972 = 0;
    uint32_t loop_1039ae = 0;
    uint32_t loop_1039c2 = 0;
    uint32_t loop_1039d9 = 0;
    uint32_t loop_1039ec = 0;
    uint32_t loop_1039fe = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_103945:
    e12bd_render_block_44k1_group_103945(exec,v,sine);
L_10394e:
    e12bd_render_block_44k1_group_10394e(exec,v,sine);
L_103956:
    e12bd_render_block_44k1_group_103956(exec,v,sine);
L_10395f: /* ble     func_103963 */
    if (!exec->flags[2]) goto L_103963;
    MD_TRANSLATION_TRACE(0x10395fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103961:
    e12bd_render_block_44k1_group_103961(exec,v,sine);
L_103963:
    e12bd_render_block_44k1_group_103963(exec,v,sine);
L_10396b:
    e12bd_render_block_44k1_group_10396b(exec,v,sine);
L_103972: /* do      #<$20,>$10397c */
    loop_103972 = 0x20u; if (loop_103972 == 0u) goto L_10397c;
    MD_TRANSLATION_TRACE(0x103972u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103974:
    e12bd_render_block_44k1_group_103974(exec,v,sine);
    if (--loop_103972 != 0u) goto L_103974;
L_10397c:
    e12bd_render_block_44k1_group_10397c(exec,v,sine);
L_103986: /* blt     func_10398a */
    if (exec->flags[1]) goto L_10398a;
    MD_TRANSLATION_TRACE(0x103986u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103988:
    e12bd_render_block_44k1_group_103988(exec,v,sine);
L_10398a:
    e12bd_render_block_44k1_group_10398a(exec,v,sine);
L_103991: /* blt     func_1039c6 */
    if (exec->flags[1]) goto L_1039c6;
    MD_TRANSLATION_TRACE(0x103991u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103993:
    e12bd_render_block_44k1_group_103993(exec,v,sine);
L_103997: /* bge     func_1039c7 */
    if (!exec->flags[1]) goto L_1039c7;
    MD_TRANSLATION_TRACE(0x103997u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103999:
    e12bd_render_block_44k1_group_103999(exec,v,sine);
L_1039a2:
    e12bd_render_block_44k1_group_1039a2(exec,v,sine);
L_1039ac:
    e12bd_render_block_44k1_group_1039ac(exec,v,sine);
L_1039ae: /* do      exec->words[0],>$1039b4 */
    loop_1039ae = exec->words[0]; if (loop_1039ae == 0u) goto L_1039b4;
    MD_TRANSLATION_TRACE(0x1039aeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039b0:
    e12bd_render_block_44k1_group_1039b0(exec,v,sine);
    if (--loop_1039ae != 0u) goto L_1039b0;
L_1039b4:
    e12bd_render_block_44k1_group_1039b4(exec,v,sine);
L_1039b8: /* bge     func_1039c5 */
    if (!exec->flags[1]) goto L_1039c5;
    MD_TRANSLATION_TRACE(0x1039b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039ba:
    e12bd_render_block_44k1_group_1039ba(exec,v,sine);
L_1039c2: /* do      exec->words[2],>$1039c5 */
    loop_1039c2 = exec->words[2]; if (loop_1039c2 == 0u) goto L_1039c5;
    MD_TRANSLATION_TRACE(0x1039c2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039c4:
    e12bd_render_block_44k1_group_1039c4(exec,v,sine);
    if (--loop_1039c2 != 0u) goto L_1039c4;
L_1039c5:
    e12bd_render_block_44k1_group_1039c5(exec,v,sine);
L_1039c6:
    e12bd_render_block_44k1_group_1039c6(exec,v,sine);
L_1039c7:
    e12bd_render_block_44k1_group_1039c7(exec,v,sine);
L_1039d0:
    e12bd_render_block_44k1_group_1039d0(exec,v,sine);
L_1039d5: /* bcc     func_1039d9 */
    if (!exec->flags[4]) goto L_1039d9;
    MD_TRANSLATION_TRACE(0x1039d5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039d7:
    e12bd_render_block_44k1_group_1039d7(exec,v,sine);
L_1039d9: /* do      #<$22,>$1039e3 */
    loop_1039d9 = 0x22u; if (loop_1039d9 == 0u) goto L_1039e3;
    MD_TRANSLATION_TRACE(0x1039d9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039db:
    e12bd_render_block_44k1_group_1039db(exec,v,sine);
    if (--loop_1039d9 != 0u) goto L_1039db;
L_1039e3:
    e12bd_render_block_44k1_group_1039e3(exec,v,sine);
L_1039ec: /* do      #<$20,>$1039f6 */
    loop_1039ec = 0x20u; if (loop_1039ec == 0u) goto L_1039f6;
    MD_TRANSLATION_TRACE(0x1039ecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1039ee:
    e12bd_render_block_44k1_group_1039ee(exec,v,sine);
    if (--loop_1039ec != 0u) goto L_1039ee;
L_1039f6:
    e12bd_render_block_44k1_group_1039f6(exec,v,sine);
L_1039fe: /* do      #<$20,>$103a0b */
    loop_1039fe = 0x20u; if (loop_1039fe == 0u) goto L_103a0b;
    MD_TRANSLATION_TRACE(0x1039feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a00:
    e12bd_render_block_44k1_group_103a00(exec,v,sine);
L_103a08:
    e12bd_render_block_44k1_group_103a08(exec,v,sine);
    if (--loop_1039fe != 0u) goto L_103a00;
L_103a0b:
    e12bd_render_block_44k1_group_103a0b(exec,v,sine);
L_103a0f: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t e12bd_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_202948: /* lea.l -$10(a7), a7 */
    /* local register save area */
C_20294c: /* movem.l d[2]-d[3]/a2-a3, (a7) */
    /* native locals replace original callee saves */
C_202950: /* movea.l $14(a7), a3 */
    /* resolved argument pointer */
C_202954: /* movea.l $18(a7), a2 */
    /* resolved argument pointer */
C_202958: /* clr.l d[0] */
    d[0] = 0u;
C_20295a: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20295c: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_20295e: /* lea.l $24bd94.l, a1 */
    /* resolved table pointer */
C_202964: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202966: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_202968: /* addq.l #$8, d[1] */
    d[1] = d[1] + 0x8u;
C_20296a: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_20296c: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_20296e: /* move.l (a1, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_202972: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202974: /* move.l d[0], $4(a3) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_202978: /* clr.l $8(a3) */
    *(y + (8u) / 4u) = (int32_t)(0u);
C_20297c: /* clr.l d[0] */
    d[0] = 0u;
C_20297e: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202982: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_202984: /* moveq #$3, d[3] */
    d[3] = 0x00000003u;
C_202986: /* divs.l d[3], d[0] */
    d[0] = md_divs32(d[0], d[3]);
C_20298a: /* asr.l #$6, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x6u) & 63u) < 32u ? ((0x6u) & 63u) : 31u));
C_20298c: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20298e: /* movea.l d[0], a0 */
    a0 = d[0];
C_202990: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202996: /* move.l $190(a0), $c(a3) */
    *(y + (12u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((400u) + a0)) / 4u)));
C_20299c: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2029a0: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_2029a2: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2029a4: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_2029aa: /* move.l d[0], $10(a3) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_2029ae: /* clr.l d[0] */
    d[0] = 0u;
C_2029b0: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_2029b4: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_2029b6: /* lea.l $24bf94.l, a0 */
    /* resolved table pointer */
C_2029bc: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((1028u + (d[0] * 4u + (0))) / 4u));
C_2029c0: /* move.l d[0], $14(a3) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_2029c4: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_2029c8: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_2029ca: /* andi.l #$7fff80, d[0] */
    lhs = d[0]; rhs = 0x7fff80u; d[0] = lhs & rhs;
C_2029d0: /* move.l d[0], $18(a3) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_2029d4: /* move.l #$7fffff, d[2] */
    d[2] = 0x7fffffu;
C_2029da: /* move.l d[2], $1c(a3) */
    *(y + (28u) / 4u) = (int32_t)(d[2]);
C_2029de: /* clr.l d[0] */
    d[0] = 0u;
C_2029e0: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_2029e4: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_2029e6: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2029e8: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_2029ea: /* addi.l #$b, d[1] */
    lhs = d[1]; rhs = 0xbu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_2029f0: /* move.b #$7f, d[3] */
    d[3] = (d[3] & 0xffffff00u) | ((0x7fu) & 0xffu);
C_2029f4: /* and.l d[3], d[0] */
    lhs = d[0]; rhs = d[3]; d[0] = lhs & rhs;
C_2029f6: /* move.l (a1, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_2029fa: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2029fc: /* addi.l #$ff800000, d[0] */
    lhs = d[0]; rhs = 0xff800000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202a02: /* move.l d[0], $20(a3) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_202a06: /* clr.l d[0] */
    d[0] = 0u;
C_202a08: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_202a0c: /* cmpi.l #$3f40, d[0] */
    lhs = d[0]; rhs = 0x3f40u;
C_202a12: /* bls.b $202a1a */
    if (lhs <= rhs) goto C_202a1a;
C_202a14: /* moveq #$7f, d[0] */
    d[0] = 0x0000007fu;
C_202a16: /* swap d[0] */
    d[0] = (d[0] << 16) | (d[0] >> 16);
C_202a18: /* bra.b $202a3a */
    goto C_202a3a;
C_202a1a: /* cmpi.l #$1fff, d[0] */
    lhs = d[0]; rhs = 0x1fffu;
C_202a20: /* bhi.b $202a2c */
    if (lhs > rhs) goto C_202a2c;
C_202a22: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202a28: /* asr.l #$7, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_202a2a: /* bra.b $202a3a */
    goto C_202a3a;
C_202a2c: /* muls.l d[0], d[0] */
    d[0] = d[0] * (d[0]);
C_202a30: /* moveq #$15, d[1] */
    d[1] = 0x00000015u;
C_202a32: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_202a34: /* addi.l #$20, d[0] */
    lhs = d[0]; rhs = 0x20u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202a3a: /* move.l d[0], $24(a3) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_202a3e: /* clr.l d[2] */
    d[2] = 0u;
C_202a40: /* move.w $c(a2), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_202a44: /* cmpi.l #$1fff, d[2] */
    lhs = d[2]; rhs = 0x1fffu;
C_202a4a: /* bhi.b $202a76 */
    if (lhs > rhs) goto C_202a76;
C_202a4c: /* move.l #$10266000, d[0] */
    d[0] = 0x10266000u;
C_202a52: /* move.l $100150c.l, d[3] */
    d[3] = md_control_tempo_denominator;
C_202a58: /* divs.l d[3], d[0] */
    d[0] = md_divs32(d[0], d[3]);
C_202a5c: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202a5e: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_202a60: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_202a62: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_202a64: /* moveq #$c, d[0] */
    d[0] = 0x0000000cu;
C_202a66: /* asr.l d[0], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[0]) & 63u) < 32u ? ((d[0]) & 63u) : 31u));
C_202a68: /* muls.l d[1], d[2] */
    d[2] = d[2] * (d[1]);
C_202a6c: /* moveq #$d, d[1] */
    d[1] = 0x0000000du;
C_202a6e: /* asr.l d[1], d[2] */
    d[2] = (uint32_t)((int32_t)d[2] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_202a70: /* move.l d[2], $28(a3) */
    *(y + (40u) / 4u) = (int32_t)(d[2]);
C_202a74: /* bra.b $202acc */
    goto C_202acc;
C_202a76: /* move.l d[2], d[0] */
    d[0] = d[2];
C_202a78: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_202a7a: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_202a7c: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_202a80: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_202a82: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202a84: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_202a86: /* moveq #$5, d[3] */
    d[3] = 0x00000005u;
C_202a88: /* sub.l d[1], d[3] */
    lhs = d[3]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[3] = lhs - rhs;
C_202a8a: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202a8c: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_202a92: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_202a96: /* asr.l d[3], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[3]) & 63u) < 32u ? ((d[3]) & 63u) : 31u));
C_202a98: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202a9a: /* lsl.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] << ((0x7u) & 63u) : 0u);
C_202a9c: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_202a9e: /* add.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202aa0: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202aa2: /* lsl.l #$3, d[1] */
    d[1] = (((0x3u) & 63u) < 32u ? d[1] << ((0x3u) & 63u) : 0u);
C_202aa4: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_202aa6: /* move.l d[1], d[2] */
    d[2] = d[1];
C_202aa8: /* lsl.l #$4, d[2] */
    d[2] = (((0x4u) & 63u) < 32u ? d[2] << ((0x4u) & 63u) : 0u);
C_202aaa: /* sub.l d[1], d[2] */
    lhs = d[2]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_202aac: /* move.l d[2], d[0] */
    d[0] = d[2];
C_202aae: /* lsl.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] << ((0x6u) & 63u) : 0u);
C_202ab0: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_202ab2: /* move.l $100150c.l, d[2] */
    d[2] = md_control_tempo_denominator;
C_202ab8: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_202abc: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202abe: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_202ac0: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_202ac2: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_202ac4: /* moveq #$c, d[3] */
    d[3] = 0x0000000cu;
C_202ac6: /* asr.l d[3], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[3]) & 63u) < 32u ? ((d[3]) & 63u) : 31u));
C_202ac8: /* move.l d[1], $28(a3) */
    *(y + (40u) / 4u) = (int32_t)(d[1]);
C_202acc: /* lea.l $28(a3), a0 */
    a0 = 40u;
C_202ad0: /* moveq #$10, d[0] */
    d[0] = 0x00000010u;
C_202ad2: /* cmp.l (a0), d[0] */
    lhs = d[0]; rhs = (uint32_t)*(y + ((0u) + a0) / 4u);
C_202ad4: /* blt.b $202adc */
    if ((int32_t)lhs < (int32_t)rhs) goto C_202adc;
C_202ad6: /* move.l #$11, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x11u);
C_202adc: /* moveq #$b, d[0] */
    d[0] = 0x0000000bu;
C_202ade: /* movem.l (a7), d[2]-d[3]/a2-a3 */
    /* native locals replace original callee saves */
C_202ae2: /* lea.l $10(a7), a7 */
    /* local register save area */
C_202ae6: /* rts  */
    return d[0];
}

void e12bd_update_coeffs(const uint32_t *p, E12BdVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = e12bd_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { e12bd_dsp_init(v, 0); }
}
void e12bd_voice_init(E12BdVoice *v) {
    uint32_t i;
    for (i = 0u; i < 35u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void e12bd_voice_trigger(E12BdVoice *v) {
    e12bd_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t e12bd_resample_run(E12BdVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE E12BdVoice
#define MD_RENDER48 e12bd_render_block_48k_float
#define MD_RENDER44 e12bd_render_block_44k1
#define MD_RESAMPLE_RUN e12bd_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE E12BdVoice
#define MD_SLOT_INIT e12bd_slot_init
#define MD_SLOT_PROCESS e12bd_slot_process
#define MD_VOICE_INIT e12bd_voice_init
#define MD_COEFFICIENTS e12bd_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER e12bd_update_coeffs
#define MD_VOICE_TRIGGER e12bd_voice_trigger
#define MD_RENDER44 e12bd_render_block_44k1
#define MD_RENDER48 e12bd_render_block_48k_float
#include "../engine_slot.inc"
#endif
