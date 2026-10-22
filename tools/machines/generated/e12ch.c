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
    int32_t y[32];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} E12ChVoice;
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

static uint32_t md_read_source(E12ChVoice *v, uint32_t address) {
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

static uint32_t md_read_x(E12ChVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
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
static void md_write_x(E12ChVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(E12ChVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2080u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(E12ChVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2080u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(E12ChVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(E12ChVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(E12ChVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef E12CH_DSP_INIT_EXEC_DEFINED
#define E12CH_DSP_INIT_EXEC_DEFINED
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
} E12Ch_Dsp_InitExec;
#endif
static void e12ch_dsp_init_group_1036e0(E12Ch_Dsp_InitExec *exec, E12ChVoice *v, const int32_t *sine) {
L_1036e0: /* move    #>$4,exec->words[0] */
    exec->words[0] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1036e0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036e2: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036e2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036e4: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1036e4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036e5: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036e7: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036e9: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036e9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1036eb: /* move    exec->words[0],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1036ebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12ch_dsp_init(E12ChVoice *v, const int32_t *sine) {
    E12Ch_Dsp_InitExec execution;
    E12Ch_Dsp_InitExec *exec;
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
L_1036e0:
    e12ch_dsp_init_group_1036e0(exec,v,sine);
L_1036ed: /* rts */
    return;}

#ifndef E12CH_DSP_TRIGGER_EXEC_DEFINED
#define E12CH_DSP_TRIGGER_EXEC_DEFINED
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
} E12Ch_Dsp_TriggerExec;
#endif
static void e12ch_dsp_trigger_group_103768(E12Ch_Dsp_TriggerExec *exec, E12ChVoice *v, const int32_t *sine) {
L_103768: /* move    y:(exec->addresses[6]+$f),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103768u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103769: /* mpyi    #>$3,exec->words[0],a */
    mpy_ss32(sx24((int32_t)(0x3u)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103769u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10376b: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10376bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10376c: /* move    exec->words[1],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10376cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10376d: /* move    exec->words[1],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10376du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10376e: /* move    a0,a1 */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((exec->lows[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10376eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10376f: /* asr     #$1,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10376fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103770: /* add     #>$103d7b,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x103d7bu)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103770u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_dsp_trigger_group_103772(E12Ch_Dsp_TriggerExec *exec, E12ChVoice *v, const int32_t *sine) {
L_103772: /* move    exec->words[1],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103772u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103773: /* move    a1,exec->addresses[0] */
    exec->addresses[0] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103773u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103774: /* move    #>$7fffff,exec->words[1] */
    exec->words[1] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103774u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103776: /* move    exec->words[1],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103776u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103777: /* move    y:(exec->addresses[0])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103777u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103778: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103778u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103779: /* move    y:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103779u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10377a: /* move    exec->words[2],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x10377au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_dsp_trigger_group_10377b(E12Ch_Dsp_TriggerExec *exec, E12ChVoice *v, const int32_t *sine) {
L_10377b: /* move    y:(exec->addresses[6]+$6),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10377bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10377c: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10377cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10377d: /* move    y:(exec->addresses[0])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10377du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10377e: /* move    exec->words[0],y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10377eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10377f: /* move    a,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x10377fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103780: /* move    y:(exec->addresses[6]+$9),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103780u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103781: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103781u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12ch_dsp_trigger(E12ChVoice *v, const int32_t *sine) {
    E12Ch_Dsp_TriggerExec execution;
    E12Ch_Dsp_TriggerExec *exec;
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
L_103768:
    e12ch_dsp_trigger_group_103768(exec,v,sine);
L_103772:
    e12ch_dsp_trigger_group_103772(exec,v,sine);
L_10377b:
    e12ch_dsp_trigger_group_10377b(exec,v,sine);
L_103782: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

#ifndef E12CH_RENDER_BLOCK_44K1_EXEC_DEFINED
#define E12CH_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} E12Ch_Render_Block_44K1Exec;
#endif
static void e12ch_render_block_44k1_group_103838(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103838: /* move    y:(exec->addresses[6]+$13),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103838u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103839: /* move    y:(exec->addresses[6]+$7),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103839u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10383a: /* mpy     exec->words[2],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10383au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10383b: /* move    y:(exec->addresses[6]+$8),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10383bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10383c: /* move    b,y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10383cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10383d: /* move    y:(exec->addresses[6]+$15),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10383du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10383e: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10383eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10383f: /* clr     b */
    exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10383fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103840(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103840: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103840u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103841: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103841u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103842: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103842u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103843: /* move    exec->words[0],b0 */
    exec->lows[1] = (exec->words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103843u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103844: /* asr     #$18,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[0] = (uint32_t)(exec->accum[2] >> 0u) & 0xffffffu; exec->accum[0] = exec->accum[2] >> 24u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103844u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103845: /* add     a,b */
    add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103845u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103846: /* asl     #$3,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 3u) | (exec->lows[2] >> 21u)); exec->lows[1] = (exec->lows[2] << 3u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103846u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103847: /* cmp     #>$4,b */
    exec->accum[2] = sx24((int32_t)(0x4u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103847u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10384b(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10384b: /* move    #>$4,b */
    exec->accum[1] = sx24((int32_t)(0x4u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10384bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10384d(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10384d: /* tst     b */
    exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10384du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10384e: /* clr     b               ifmi */
    if (exec->flags[3]) { exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); }
    MD_TRANSLATION_TRACE(0x10384eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10384f: /* move    #$a0,exec->addresses[2] */
    exec->addresses[2] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10384fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103850: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103850u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103851: /* move    #$2,exec->addresses[12] */
    exec->addresses[12] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103851u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103852: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103852u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103853: /* move    y:(exec->addresses[6]+$11),a0 */
    exec->lows[0] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103853u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103854: /* move    b0,exec->words[0] */
    exec->words[0] = (exec->lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103854u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103855(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103855: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103855u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103856: /* move    #>$147400,exec->words[3] */
    exec->words[3] = (0x147400u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103856u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103858: /* move    exec->words[3],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103858u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103859: /* move    #>$200,exec->words[2] */
    exec->words[2] = (0x200u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103859u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10385b: /* move    exec->words[2],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x10385bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10385c: /* move    #>$fffffc,exec->words[3] */
    exec->words[3] = (0xfffffcu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10385cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10385e: /* move    exec->words[3],y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x10385eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103861(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103861: /* move    a0,exec->words[3] */
    exec->words[3] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103861u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103862: /* mpy     exec->words[3],exec->words[2],b         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103862u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103863: /* and     exec->words[2],b            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | (u24(exec->accum[1]) & exec->words[2])); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103863u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103864: /* add     exec->words[2],b            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103864u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103865: /* add     x,a             a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103865u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103866: /* move    b1,y:(exec->addresses[2])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x103866u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103867(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103867: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103867u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103869: /* move    y:(exec->addresses[6]+$1c),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103869u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10386a: /* move    a1,x:>$fe */
    *(md_shared_scratch_x + 254u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10386au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10386c: /* move    y:(exec->addresses[6]+$10),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10386cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10386d: /* move    exec->words[3],x:>$ff */
    *(md_shared_scratch_x + 255u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10386du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10386f: /* add     exec->words[3],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10386fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103870: /* cmp     exec->words[2],a */
    exec->accum[2] = sx24((int32_t)(exec->words[2])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103870u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103873(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103873: /* move    y:(exec->addresses[6]+$13),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 19u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103873u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103874: /* nop */

    MD_TRANSLATION_TRACE(0x103874u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103875(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103875: /* move    a0,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103875u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103876: /* move    a1,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103876u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103877: /* move    y:(exec->addresses[6]+$1e),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 30u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103877u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103878: /* add     #>$20,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x20u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103878u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10387a: /* move    y:(exec->addresses[6]+$a),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10387au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10387b: /* cmp     exec->words[0],a */
    exec->accum[2] = sx24((int32_t)(exec->words[0])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x10387bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10387e(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10387e: /* move    y:(exec->addresses[6]+$1f),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 31u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10387eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10387f: /* add     #<$1,b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(0x1u)), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10387fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103880: /* move    y:(exec->addresses[6]+$e),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103880u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103881: /* cmp     exec->words[1],b */
    exec->accum[2] = sx24((int32_t)(exec->words[1])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103881u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103884(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103884: /* move    b,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x103884u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103885: /* sub     exec->words[0],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103885u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103886: /* and     #<$1f,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x1fu));
    MD_TRANSLATION_TRACE(0x103886u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103887: /* move    #>$bf,b */
    exec->accum[1] = sx24((int32_t)(0xbfu)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103887u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103889: /* sub     a,b */
    sub56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103889u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10388a: /* move    y:(exec->addresses[6]+$13),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10388au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10388b: /* move    b,exec->addresses[0] */
    exec->addresses[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10388bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10388c: /* move    y:(exec->addresses[6]+$6),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10388cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10388d(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10388d: /* mpy     exec->words[3],exec->words[1],b         a,exec->words[0] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10388du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10388e: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10388eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10388f: /* move    exec->words[1],x:>$c0 */
    *(md_shared_scratch_x + 192u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10388fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103891: /* move    x:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103891u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103892: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103892u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103893: /* nop */

    MD_TRANSLATION_TRACE(0x103893u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103894: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103894u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103895: /* move    exec->words[1],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103895u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103896(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103896: /* move    exec->words[1],x:>$ff */
    *(md_shared_scratch_x + 255u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103896u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10389a(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10389a: /* move    x:(exec->addresses[0]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10389au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10389b: /* sub     exec->words[1],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10389bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10389c: /* nop */

    MD_TRANSLATION_TRACE(0x10389cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10389d: /* move    a,x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10389du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10389e(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10389e: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10389eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038ac(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038ac: /* nop */

    MD_TRANSLATION_TRACE(0x1038acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038ad(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038ad: /* move    a,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x1038adu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038ae(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038ae: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038aeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038af: /* move    #$2,exec->addresses[5] */
    exec->addresses[5] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038afu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b0: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1038b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b2: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1038b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b3: /* move    y:(exec->addresses[6]+$12),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b4: /* add     exec->words[2],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1038b4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b5: /* move    #$3,exec->addresses[20] */
    exec->addresses[20] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038b5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b6: /* move    #$3,exec->addresses[21] */
    exec->addresses[21] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038b7(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038b7: /* move    a1,exec->addresses[3] */
    exec->addresses[3] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038b7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b8: /* move    #$87,exec->addresses[12] */
    exec->addresses[12] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038b9: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1038b9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038bb: /* btst    #$0,a */
    exec->flags[4] = (((u24(lim24(exec->accum[0]))) >> 0u) & 1u);
    MD_TRANSLATION_TRACE(0x1038bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038be(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038be: /* move    #$87,exec->addresses[4] */
    exec->addresses[4] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038beu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038bf: /* move    #$1,exec->addresses[5] */
    exec->addresses[5] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038bfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038c2(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038c2: /* move    x:(exec->addresses[3])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1038c2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c3: /* move    x:(exec->addresses[3])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1038c3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c4: /* move    a1,x:(exec->addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x1038c4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c5: /* lsl     #$c,a */
    exec->flags[4] = (u24(exec->accum[0]) >> 12u) & 1u; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((u24(exec->accum[0]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[0]) == 0u); exec->flags[3] = ((u24(exec->accum[0]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x1038c5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c6: /* move    b1,x:(exec->addresses[5])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x1038c6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c7: /* lsl     #$c,b */
    exec->flags[4] = (u24(exec->accum[1]) >> 12u) & 1u; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(exec->accum[1]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[1]) == 0u); exec->flags[3] = ((u24(exec->accum[1]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x1038c7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c8: /* move    a1,x:(exec->addresses[4])+exec->addresses[20] */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x1038c8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c9: /* move    b1,x:(exec->addresses[5])+exec->addresses[21] */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], sx24((int32_t)exec->addresses[21]), 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x1038c9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038ca(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038ca: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038cau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038cc: /* move    #>$ffffff,exec->addresses[13] */
    exec->addresses[13] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038ccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038ce: /* move    y:(exec->addresses[6]+$3),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038ceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038cf: /* move    y:(exec->addresses[6]+$15),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038cfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d0: /* move    exec->words[3],a */
    exec->accum[0] = sx24((int32_t)(exec->words[3])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1038d0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d1: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038d1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038d4(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038d4: /* mpy     exec->words[3],exec->words[1],a         a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1038d4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d5: /* nop */

    MD_TRANSLATION_TRACE(0x1038d5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d6: /* move    a,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038d6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038d7(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038d7: /* move    #$a0,exec->addresses[2] */
    exec->addresses[2] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038d7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d8: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038d8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d9: /* move    #$0,exec->addresses[3] */
    exec->addresses[3] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038d9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038da: /* move    exec->words[3],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1038dau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038db: /* move    y:(exec->addresses[2])+,exec->addresses[4] */
    exec->addresses[4] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038dbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038dc: /* move    y:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038dcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038dd: /* move    #$c8,exec->addresses[0] */
    exec->addresses[0] = (0xc8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038ddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038e0(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038e0: /* move    x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e1: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e2: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e3: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e4: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[2]),exec->addresses[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e5: /* move    y:(exec->addresses[2])+,exec->addresses[4] */
    exec->addresses[4] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e6: /* move    b,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e7: /* mpy     exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1038e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038e8(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038e8: /* move    y:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038e8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e9: /* move    b,x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1038e9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038ea(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038ea: /* move    y:(exec->addresses[6]+$4),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038eau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038ec: /* move    y:(exec->addresses[6]+$5),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 5u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1038ecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038ed: /* nop */

    MD_TRANSLATION_TRACE(0x1038edu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038ee: /* nop */

    MD_TRANSLATION_TRACE(0x1038eeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038ef: /* move    x:(exec->addresses[2]+$103c7b),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1064059u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038efu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f1: /* move    x:(exec->addresses[2]+$103cfb),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1064187u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038f1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f3: /* add     exec->words[0],a            a1,b */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1038f3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f4: /* nop */

    MD_TRANSLATION_TRACE(0x1038f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038f5(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038f5: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f6: /* nop */

    MD_TRANSLATION_TRACE(0x1038f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f7: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f8: /* nop */

    MD_TRANSLATION_TRACE(0x1038f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038f9: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038f9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038fb: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038fbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038fd: /* move    #$78,exec->addresses[4] */
    exec->addresses[4] = (0x78u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038fe: /* move    #$4,exec->addresses[12] */
    exec->addresses[12] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_1038ff(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_1038ff: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1038ffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103900: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103900u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103901: /* move    #$c4,exec->addresses[2] */
    exec->addresses[2] = (0xc4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103901u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103902: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103902u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103903: /* asl     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103903u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103904: /* asl     a               a,exec->words[1] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103904u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103905: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103905u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103906: /* add     #>$7fffff,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x7fffffu)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103906u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103908(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103908: /* asr     #$3,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 3u; exec->lows[0] = ((exec->lows[2] >> 3u) | ((uint32_t)exec->accum[2] << 21u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103908u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103909: /* add     #>$400000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103909u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10390b: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10390bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10390c: /* nop */

    MD_TRANSLATION_TRACE(0x10390cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10390d: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10390du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10390e: /* move    exec->words[1],a */
    exec->accum[0] = sx24((int32_t)(exec->words[1])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10390eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10390f: /* add     #>$c00000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0xc00000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10390fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103911: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103911u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103912(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103912: /* move    a,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103912u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103913: /* mpyi    #>$800000,exec->words[0],a */
    mpy_ss32(sx24((int32_t)(0x800000u)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103913u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103915: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103915u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103916: /* move    exec->words[0],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[0]);
    MD_TRANSLATION_TRACE(0x103916u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103917: /* move    a,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103917u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103918: /* move    exec->words[0],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[0]);
    MD_TRANSLATION_TRACE(0x103918u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103919: /* move    #$c4,exec->addresses[0] */
    exec->addresses[0] = (0xc4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103919u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10391a: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10391au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10391c(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10391c: /* move    #>$fffffd,exec->addresses[16] */
    exec->addresses[16] = (0xfffffdu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10391cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10391e: /* move    y:(exec->addresses[6]+$17),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10391eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10391f: /* move    exec->words[3],x:(exec->addresses[2])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x10391fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103920: /* move    y:(exec->addresses[6]+$18),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 24u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103920u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103921: /* move    y:(exec->addresses[6]+$1b),a0 */
    exec->lows[0] = (u24(*(v->y + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103921u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103922: /* move    y:(exec->addresses[6]+$1a),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 26u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103922u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103923: /* move    a,x:(exec->addresses[2])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103923u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103924: /* move    exec->words[3],x:(exec->addresses[2])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103924u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103925(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103925: /* move    y:(exec->addresses[6]+$19),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103925u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103926: /* move    exec->words[3],x:(exec->addresses[2]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103926u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103927: /* move    #$c6,exec->addresses[1] */
    exec->addresses[1] = (0xc6u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103927u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103928: /* move    x:>$e6,exec->words[3] */
    exec->words[3] = (u24(*(md_shared_scratch_x + 230u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103928u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10392a: /* move    exec->words[3],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10392au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10392b: /* move    x:>$e7,exec->words[3] */
    exec->words[3] = (u24(*(md_shared_scratch_x + 231u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10392bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10392d: /* move    exec->words[3],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10392du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10392e: /* move    x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10392eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103931(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103931: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103931u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103932: /* mac     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103932u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103933: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103933u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103934: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103934u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103935: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+exec->addresses[16],exec->words[0]    y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16]), 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103935u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103936: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103936u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103937: /* nop */

    MD_TRANSLATION_TRACE(0x103937u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103938: /* move    a,l:(exec->addresses[1])+ */
    exec->long_address[0] = md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103938u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_103939(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_103939: /* move    a,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103939u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12ch_render_block_44k1_group_10393a(E12Ch_Render_Block_44K1Exec *exec, E12ChVoice *v, const int32_t *sine) {
L_10393a: /* move    x:>$e4,exec->words[1] */
    exec->words[1] = (u24(*(md_shared_scratch_x + 228u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10393au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10393c: /* move    exec->words[1],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10393cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10393d: /* move    l:-(exec->addresses[1]),y */
    exec->long_address[0] = md_agu(&exec->addresses[1], exec->addresses[9], -(1), 1u); exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); exec->words[3] = exec->parallel_words[0]; exec->words[2] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x10393du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10393e: /* move    exec->words[3],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10393eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10393f: /* move    exec->words[2],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x10393fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103940: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103940u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103942: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103942u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12ch_render_block_44k1(E12ChVoice *v, const int32_t *sine) {
    E12Ch_Render_Block_44K1Exec execution;
    E12Ch_Render_Block_44K1Exec *exec;
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
    uint32_t loop_10385f = 0;
    uint32_t loop_103898 = 0;
    uint32_t loop_1038c0 = 0;
    uint32_t loop_1038d2 = 0;
    uint32_t loop_1038de = 0;
    uint32_t loop_10392f = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_103838:
    e12ch_render_block_44k1_group_103838(exec,v,sine);
L_103840:
    e12ch_render_block_44k1_group_103840(exec,v,sine);
L_103849: /* ble     func_10384d */
    if (!exec->flags[2]) goto L_10384d;
    MD_TRANSLATION_TRACE(0x103849u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10384b:
    e12ch_render_block_44k1_group_10384b(exec,v,sine);
L_10384d:
    e12ch_render_block_44k1_group_10384d(exec,v,sine);
L_103855:
    e12ch_render_block_44k1_group_103855(exec,v,sine);
L_10385f: /* do      #<$20,>$103867 */
    loop_10385f = 0x20u; if (loop_10385f == 0u) goto L_103867;
    MD_TRANSLATION_TRACE(0x10385fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103861:
    e12ch_render_block_44k1_group_103861(exec,v,sine);
    if (--loop_10385f != 0u) goto L_103861;
L_103867:
    e12ch_render_block_44k1_group_103867(exec,v,sine);
L_103871: /* blt     func_103875 */
    if (exec->flags[1]) goto L_103875;
    MD_TRANSLATION_TRACE(0x103871u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103873:
    e12ch_render_block_44k1_group_103873(exec,v,sine);
L_103875:
    e12ch_render_block_44k1_group_103875(exec,v,sine);
L_10387c: /* blt     func_1038ad */
    if (exec->flags[1]) goto L_1038ad;
    MD_TRANSLATION_TRACE(0x10387cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10387e:
    e12ch_render_block_44k1_group_10387e(exec,v,sine);
L_103882: /* bge     func_1038ae */
    if (!exec->flags[1]) goto L_1038ae;
    MD_TRANSLATION_TRACE(0x103882u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103884:
    e12ch_render_block_44k1_group_103884(exec,v,sine);
L_10388d:
    e12ch_render_block_44k1_group_10388d(exec,v,sine);
L_103896:
    e12ch_render_block_44k1_group_103896(exec,v,sine);
L_103898: /* do      exec->words[0],>$10389e */
    loop_103898 = exec->words[0]; if (loop_103898 == 0u) goto L_10389e;
    MD_TRANSLATION_TRACE(0x103898u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10389a:
    e12ch_render_block_44k1_group_10389a(exec,v,sine);
    if (--loop_103898 != 0u) goto L_10389a;
L_10389e:
    e12ch_render_block_44k1_group_10389e(exec,v,sine);
L_10389f: /* bra     func_1038ac */
    goto L_1038ac;
    MD_TRANSLATION_TRACE(0x10389fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038ac:
    e12ch_render_block_44k1_group_1038ac(exec,v,sine);
L_1038ad:
    e12ch_render_block_44k1_group_1038ad(exec,v,sine);
L_1038ae:
    e12ch_render_block_44k1_group_1038ae(exec,v,sine);
L_1038b7:
    e12ch_render_block_44k1_group_1038b7(exec,v,sine);
L_1038bc: /* bcc     func_1038c0 */
    if (!exec->flags[4]) goto L_1038c0;
    MD_TRANSLATION_TRACE(0x1038bcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038be:
    e12ch_render_block_44k1_group_1038be(exec,v,sine);
L_1038c0: /* do      #<$22,>$1038ca */
    loop_1038c0 = 0x22u; if (loop_1038c0 == 0u) goto L_1038ca;
    MD_TRANSLATION_TRACE(0x1038c0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038c2:
    e12ch_render_block_44k1_group_1038c2(exec,v,sine);
    if (--loop_1038c0 != 0u) goto L_1038c2;
L_1038ca:
    e12ch_render_block_44k1_group_1038ca(exec,v,sine);
L_1038d2: /* do      #<$20,>$1038d7 */
    loop_1038d2 = 0x20u; if (loop_1038d2 == 0u) goto L_1038d7;
    MD_TRANSLATION_TRACE(0x1038d2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038d4:
    e12ch_render_block_44k1_group_1038d4(exec,v,sine);
    if (--loop_1038d2 != 0u) goto L_1038d4;
L_1038d7:
    e12ch_render_block_44k1_group_1038d7(exec,v,sine);
L_1038de: /* do      #<$20,>$1038ea */
    loop_1038de = 0x20u; if (loop_1038de == 0u) goto L_1038ea;
    MD_TRANSLATION_TRACE(0x1038deu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1038e0:
    e12ch_render_block_44k1_group_1038e0(exec,v,sine);
L_1038e8:
    e12ch_render_block_44k1_group_1038e8(exec,v,sine);
    if (--loop_1038de != 0u) goto L_1038e0;
L_1038ea:
    e12ch_render_block_44k1_group_1038ea(exec,v,sine);
L_1038f5:
    e12ch_render_block_44k1_group_1038f5(exec,v,sine);
L_1038ff:
    e12ch_render_block_44k1_group_1038ff(exec,v,sine);
L_103908:
    e12ch_render_block_44k1_group_103908(exec,v,sine);
L_103912:
    e12ch_render_block_44k1_group_103912(exec,v,sine);
L_10391c:
    e12ch_render_block_44k1_group_10391c(exec,v,sine);
L_103925:
    e12ch_render_block_44k1_group_103925(exec,v,sine);
L_10392f: /* do      #<$20,>$10393a */
    loop_10392f = 0x20u; if (loop_10392f == 0u) goto L_10393a;
    MD_TRANSLATION_TRACE(0x10392fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103931:
    e12ch_render_block_44k1_group_103931(exec,v,sine);
L_103939:
    e12ch_render_block_44k1_group_103939(exec,v,sine);
    if (--loop_10392f != 0u) goto L_103931;
L_10393a:
    e12ch_render_block_44k1_group_10393a(exec,v,sine);
L_103944: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t e12ch_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_203434: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_203438: /* movem.l d[2]-d[3]/a2, (a7) */
    /* native locals replace original callee saves */
C_20343c: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_203440: /* movea.l $14(a7), a2 */
    /* resolved argument pointer */
C_203444: /* move.w (a2), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_203446: /* clr.l d[0] */
    d[0] = 0u;
C_203448: /* move.w d[2], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[2] & 0xffffu)) & 0xffffu);
C_20344a: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_20344c: /* move.l d[0], d[1] */
    d[1] = d[0];
C_20344e: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_203450: /* addq.l #$8, d[1] */
    d[1] = d[1] + 0x8u;
C_203452: /* moveq #$7f, d[3] */
    d[3] = 0x0000007fu;
C_203454: /* and.l d[3], d[0] */
    lhs = d[0]; rhs = d[3]; d[0] = lhs & rhs;
C_203456: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_20345c: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_203460: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203462: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_203466: /* clr.l $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(0u);
C_20346a: /* clr.l d[0] */
    d[0] = 0u;
C_20346c: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_203470: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_203472: /* move.b #$3, d[3] */
    d[3] = (d[3] & 0xffffff00u) | ((0x3u) & 0xffu);
C_203476: /* divs.l d[3], d[0] */
    d[0] = md_divs32(d[0], d[3]);
C_20347a: /* asr.l #$6, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x6u) & 63u) < 32u ? ((0x6u) & 63u) : 31u));
C_20347c: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20347e: /* movea.l d[0], a0 */
    a0 = d[0];
C_203480: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_203486: /* move.l $190(a0), $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((400u) + a0)) / 4u)));
C_20348c: /* clr.l d[0] */
    d[0] = 0u;
C_20348e: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_203492: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_203494: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_203498: /* clr.l d[0] */
    d[0] = 0u;
C_20349a: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_20349e: /* lsr.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] >> ((0x3u) & 63u) : 0u);
C_2034a0: /* mulu.w #$cccd, d[2] */
    d[2] = (d[2] & 0xffffu) * (0xcccdu);
C_2034a4: /* moveq #$15, d[1] */
    d[1] = 0x00000015u;
C_2034a6: /* lsr.l d[1], d[2] */
    d[2] = (((d[1]) & 63u) < 32u ? d[2] >> ((d[1]) & 63u) : 0u);
C_2034a8: /* movea.l d[2], a0 */
    a0 = d[2];
C_2034aa: /* lea.l $32(a0, d[0].l), a0 */
    a0 = (d[0] * 1u + (50)) + a0;
C_2034ae: /* move.l a0, d[1] */
    d[1] = a0;
C_2034b0: /* cmpa.l #$7ff, a0 */
    lhs = a0; rhs = 0x7ffu;
C_2034b6: /* ble.b $2034c0 */
    if ((int32_t)lhs <= (int32_t)rhs) goto C_2034c0;
C_2034b8: /* move.l #$7ff, d[1] */
    d[1] = 0x7ffu;
C_2034be: /* bra.b $2034ca */
    goto C_2034ca;
C_2034c0: /* move.l a0, d[0] */
    d[0] = a0;
C_2034c2: /* not.l d[0] */
    d[0] = ~d[0];
C_2034c4: /* add.l d[0], d[0] */
    lhs = d[0]; rhs = d[0]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2034c6: /* subx.l d[0], d[0] */
    lhs = d[0]; rhs = d[0]; cf_temp[0] = lhs - rhs - cf_carry[0]; cf_carry[0] = (lhs < rhs || (cf_carry[0] && lhs == rhs)); d[0] = cf_temp[0];
C_2034c8: /* and.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; d[1] = lhs & rhs;
C_2034ca: /* move.l d[1], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[1]);
C_2034ce: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_2034d2: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_2034d4: /* andi.l #$7fff80, d[0] */
    lhs = d[0]; rhs = 0x7fff80u; d[0] = lhs & rhs;
C_2034da: /* move.l d[0], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_2034de: /* move.l #$7fffff, d[0] */
    d[0] = 0x7fffffu;
C_2034e4: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_2034e8: /* clr.l d[0] */
    d[0] = 0u;
C_2034ea: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_2034ee: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_2034f0: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2034f2: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_2034f4: /* addi.l #$b, d[1] */
    lhs = d[1]; rhs = 0xbu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_2034fa: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_2034fc: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_2034fe: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_203504: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_203508: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20350a: /* addi.l #$ff800000, d[0] */
    lhs = d[0]; rhs = 0xff800000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203510: /* move.l d[0], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_203514: /* clr.l d[0] */
    d[0] = 0u;
C_203516: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_20351a: /* cmpi.l #$3f40, d[0] */
    lhs = d[0]; rhs = 0x3f40u;
C_203520: /* bls.b $20352c */
    if (lhs <= rhs) goto C_20352c;
C_203522: /* moveq #$7f, d[3] */
    d[3] = 0x0000007fu;
C_203524: /* swap d[3] */
    d[3] = (d[3] << 16) | (d[3] >> 16);
C_203526: /* move.l d[3], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[3]);
C_20352a: /* bra.b $203550 */
    goto C_203550;
C_20352c: /* cmpi.l #$1fff, d[0] */
    lhs = d[0]; rhs = 0x1fffu;
C_203532: /* bhi.b $20353e */
    if (lhs > rhs) goto C_20353e;
C_203534: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20353a: /* asr.l #$7, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_20353c: /* bra.b $20354c */
    goto C_20354c;
C_20353e: /* muls.l d[0], d[0] */
    d[0] = d[0] * (d[0]);
C_203542: /* moveq #$15, d[1] */
    d[1] = 0x00000015u;
C_203544: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_203546: /* addi.l #$20, d[0] */
    lhs = d[0]; rhs = 0x20u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20354c: /* move.l d[0], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_203550: /* clr.l d[2] */
    d[2] = 0u;
C_203552: /* move.w $c(a2), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_203556: /* cmpi.l #$1fff, d[2] */
    lhs = d[2]; rhs = 0x1fffu;
C_20355c: /* bhi.b $203588 */
    if (lhs > rhs) goto C_203588;
C_20355e: /* move.l #$10266000, d[0] */
    d[0] = 0x10266000u;
C_203564: /* move.l $100150c.l, d[3] */
    d[3] = md_control_tempo_denominator;
C_20356a: /* divs.l d[3], d[0] */
    d[0] = md_divs32(d[0], d[3]);
C_20356e: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203570: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_203572: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_203574: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_203576: /* moveq #$c, d[0] */
    d[0] = 0x0000000cu;
C_203578: /* asr.l d[0], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[0]) & 63u) < 32u ? ((d[0]) & 63u) : 31u));
C_20357a: /* muls.l d[1], d[2] */
    d[2] = d[2] * (d[1]);
C_20357e: /* moveq #$d, d[1] */
    d[1] = 0x0000000du;
C_203580: /* asr.l d[1], d[2] */
    d[2] = (uint32_t)((int32_t)d[2] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_203582: /* move.l d[2], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[2]);
C_203586: /* bra.b $2035de */
    goto C_2035de;
C_203588: /* move.l d[2], d[0] */
    d[0] = d[2];
C_20358a: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_20358c: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_20358e: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_203592: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_203594: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203596: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_203598: /* moveq #$5, d[3] */
    d[3] = 0x00000005u;
C_20359a: /* sub.l d[1], d[3] */
    lhs = d[3]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[3] = lhs - rhs;
C_20359c: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_20359e: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_2035a4: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_2035a8: /* asr.l d[3], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[3]) & 63u) < 32u ? ((d[3]) & 63u) : 31u));
C_2035aa: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2035ac: /* lsl.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] << ((0x7u) & 63u) : 0u);
C_2035ae: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_2035b0: /* add.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2035b2: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2035b4: /* lsl.l #$3, d[1] */
    d[1] = (((0x3u) & 63u) < 32u ? d[1] << ((0x3u) & 63u) : 0u);
C_2035b6: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_2035b8: /* move.l d[1], d[2] */
    d[2] = d[1];
C_2035ba: /* lsl.l #$4, d[2] */
    d[2] = (((0x4u) & 63u) < 32u ? d[2] << ((0x4u) & 63u) : 0u);
C_2035bc: /* sub.l d[1], d[2] */
    lhs = d[2]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_2035be: /* move.l d[2], d[0] */
    d[0] = d[2];
C_2035c0: /* lsl.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] << ((0x6u) & 63u) : 0u);
C_2035c2: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_2035c4: /* move.l $100150c.l, d[2] */
    d[2] = md_control_tempo_denominator;
C_2035ca: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_2035ce: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2035d0: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_2035d2: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2035d4: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_2035d6: /* moveq #$c, d[3] */
    d[3] = 0x0000000cu;
C_2035d8: /* asr.l d[3], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[3]) & 63u) < 32u ? ((d[3]) & 63u) : 31u));
C_2035da: /* move.l d[1], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[1]);
C_2035de: /* lea.l $28(a1), a0 */
    a0 = 40u;
C_2035e2: /* moveq #$10, d[0] */
    d[0] = 0x00000010u;
C_2035e4: /* cmp.l (a0), d[0] */
    lhs = d[0]; rhs = (uint32_t)*(y + ((0u) + a0) / 4u);
C_2035e6: /* blt.b $2035ee */
    if ((int32_t)lhs < (int32_t)rhs) goto C_2035ee;
C_2035e8: /* move.l #$11, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x11u);
C_2035ee: /* moveq #$b, d[0] */
    d[0] = 0x0000000bu;
C_2035f0: /* movem.l (a7), d[2]-d[3]/a2 */
    /* native locals replace original callee saves */
C_2035f4: /* lea.l $c(a7), a7 */
    /* local register save area */
C_2035f8: /* rts  */
    return d[0];
}

void e12ch_update_coeffs(const uint32_t *p, E12ChVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = e12ch_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { e12ch_dsp_init(v, 0); }
}
void e12ch_voice_init(E12ChVoice *v) {
    uint32_t i;
    for (i = 0u; i < 32u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void e12ch_voice_trigger(E12ChVoice *v) {
    e12ch_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t e12ch_resample_run(E12ChVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE E12ChVoice
#define MD_RENDER48 e12ch_render_block_48k_float
#define MD_RENDER44 e12ch_render_block_44k1
#define MD_RESAMPLE_RUN e12ch_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE E12ChVoice
#define MD_SLOT_INIT e12ch_slot_init
#define MD_SLOT_PROCESS e12ch_slot_process
#define MD_VOICE_INIT e12ch_voice_init
#define MD_COEFFICIENTS e12ch_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER e12ch_update_coeffs
#define MD_VOICE_TRIGGER e12ch_voice_trigger
#define MD_RENDER44 e12ch_render_block_44k1
#define MD_RENDER48 e12ch_render_block_48k_float
#include "../engine_slot.inc"
#endif
