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
    int32_t y[33];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmcyVoice;
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

static uint32_t md_read_x(EfmcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2081u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2081u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmcyVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmcyVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
#ifndef EFMCY_DSP_INIT_EXEC_DEFINED
#define EFMCY_DSP_INIT_EXEC_DEFINED
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
} Efmcy_Dsp_InitExec;
#endif
static void efmcy_dsp_init_group_1033f1(Efmcy_Dsp_InitExec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1033f1: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033f1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033f2: /* move    exec->words[1],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1033f2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void efmcy_dsp_init(EfmcyVoice *v, const int32_t *sine) {
    Efmcy_Dsp_InitExec execution;
    Efmcy_Dsp_InitExec *exec;
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
L_1033f1:
    efmcy_dsp_init_group_1033f1(exec,v,sine);
L_1033f3: /* rts */
    return;}

#ifndef EFMCY_DSP_TRIGGER_EXEC_DEFINED
#define EFMCY_DSP_TRIGGER_EXEC_DEFINED
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
} Efmcy_Dsp_TriggerExec;
#endif
static void efmcy_dsp_trigger_group_1033f4(Efmcy_Dsp_TriggerExec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1033f4: /* move    #>$50c335,b */
    exec->accum[1] = sx24((int32_t)(0x50c335u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1033f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033f6: /* move    y:(exec->addresses[6]+$18),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033f7: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1033f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033f8: /* move    #>$148000,a */
    exec->accum[0] = sx24((int32_t)(0x148000u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1033f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033fa: /* move    #>$7fffff,exec->words[1] */
    exec->words[1] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033fau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033fc: /* move    a1,y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1033fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033fd: /* move    a0,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x1033fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1033fe: /* move    a1,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1033feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_dsp_trigger_group_1033ff(Efmcy_Dsp_TriggerExec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1033ff: /* move    a0,y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x1033ffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103400: /* move    a1,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103400u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103401: /* move    a0,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103401u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103402: /* move    a1,y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103402u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103403: /* move    a0,y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103403u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103404: /* move    a0,y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103404u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103405: /* move    a0,y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103405u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103406: /* move    a0,y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103406u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_dsp_trigger_group_103407(Efmcy_Dsp_TriggerExec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103407: /* move    a0,y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103407u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103408: /* move    a0,y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103408u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103409: /* move    a0,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103409u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10340a: /* move    a0,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x10340au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10340b: /* move    a0,y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x10340bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10340c: /* move    exec->words[1],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10340cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10340d: /* move    b,y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10340du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void efmcy_dsp_trigger(EfmcyVoice *v, const int32_t *sine) {
    Efmcy_Dsp_TriggerExec execution;
    Efmcy_Dsp_TriggerExec *exec;
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
L_1033f4:
    efmcy_dsp_trigger_group_1033f4(exec,v,sine);
L_1033ff:
    efmcy_dsp_trigger_group_1033ff(exec,v,sine);
L_103407:
    efmcy_dsp_trigger_group_103407(exec,v,sine);
L_10340e: /* rts */
    return;}

#ifndef EFMCY_RENDER_BLOCK_44K1_EXEC_DEFINED
#define EFMCY_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Efmcy_Render_Block_44K1Exec;
#endif
static void efmcy_render_block_44k1_group_10340f(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10340f: /* move    y:(exec->addresses[6]+$e),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10340fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103410: /* move    exec->words[0],x:$e */
    *(md_shared_scratch_x + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103410u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103411: /* move    y:(exec->addresses[6]+$1d),a1 */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((u24(*(v->y + 29u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103411u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103412: /* move    y:(exec->addresses[6]+$1f),b1 */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(*(v->y + 31u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103412u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103413: /* move    a1,x:$1d */
    *(md_shared_scratch_x + 29u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103413u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103414: /* move    b1,x:$1f */
    *(md_shared_scratch_x + 31u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103414u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103415: /* move    #>$7fff,exec->addresses[9] */
    exec->addresses[9] = (0x7fffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103415u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103417: /* move    #>$7fff,exec->addresses[10] */
    exec->addresses[10] = (0x7fffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103417u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103419(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103419: /* move    y:(exec->addresses[6]+$a),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103419u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10341a: /* move    #$21,exec->addresses[0] */
    exec->addresses[0] = (0x21u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10341au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10341b: /* move    y:(exec->addresses[6]+$f),exec->addresses[1] */
    exec->addresses[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10341bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10341d: /* move    y:(exec->addresses[6]+$6),exec->addresses[17] */
    exec->addresses[17] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10341du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10341f: /* move    y:(exec->addresses[6]+$11),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10341fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103421: /* move    y:(exec->addresses[6]+$7),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103421u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103425(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103425: /* move    x:(exec->addresses[1])+exec->addresses[17],exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], sx24((int32_t)exec->addresses[17]), 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103425u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103426: /* move    x:(exec->addresses[2])+exec->addresses[18],exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], sx24((int32_t)exec->addresses[18]), 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103426u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103427: /* mpyr    exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103427u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103428: /* mpyr    exec->words[1],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103428u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103429: /* move    a1,x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x103429u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10342a: /* move    b1,y:(exec->addresses[0])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x10342au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10342b(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10342b: /* move    exec->addresses[1],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->addresses[1]));
    MD_TRANSLATION_TRACE(0x10342bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10342d: /* move    exec->addresses[2],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->addresses[2]));
    MD_TRANSLATION_TRACE(0x10342du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10342f: /* move    #$41,exec->addresses[0] */
    exec->addresses[0] = (0x41u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10342fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103430: /* move    y:(exec->addresses[6]+$13),exec->addresses[1] */
    exec->addresses[1] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103430u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103432: /* move    y:(exec->addresses[6]+$8),exec->addresses[17] */
    exec->addresses[17] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103432u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103434: /* move    y:(exec->addresses[6]+$15),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103434u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103436: /* move    y:(exec->addresses[6]+$9),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103436u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10343a(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10343a: /* move    x:(exec->addresses[1])+exec->addresses[17],exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], sx24((int32_t)exec->addresses[17]), 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10343au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10343b: /* move    x:(exec->addresses[2])+exec->addresses[18],exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], sx24((int32_t)exec->addresses[18]), 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10343bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10343c: /* mpyr    exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10343cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10343d: /* mpyr    exec->words[1],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10343du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10343e: /* move    a1,x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x10343eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10343f: /* move    b1,y:(exec->addresses[0])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x10343fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103440(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103440: /* move    exec->addresses[1],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->addresses[1]));
    MD_TRANSLATION_TRACE(0x103440u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103442: /* move    exec->addresses[2],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->addresses[2]));
    MD_TRANSLATION_TRACE(0x103442u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103444: /* move    #>$ffffff,exec->addresses[9] */
    exec->addresses[9] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103444u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103446: /* move    #>$ffffff,exec->addresses[10] */
    exec->addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103446u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103448: /* move    y:(exec->addresses[6]+$17),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103448u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103449: /* move    y:(exec->addresses[6]+$b),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103449u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10344a: /* move    #$21,exec->addresses[0] */
    exec->addresses[0] = (0x21u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10344au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10344b: /* move    exec->addresses[0],exec->addresses[4] */
    exec->addresses[4] = (exec->addresses[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10344bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10344c(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10344c: /* move    #$41,exec->addresses[1] */
    exec->addresses[1] = (0x41u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10344cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10344d: /* move    exec->addresses[1],exec->addresses[5] */
    exec->addresses[5] = (exec->addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10344du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10344e: /* move    x:(exec->addresses[0]),exec->words[0]       y:(exec->addresses[4]),exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[4], sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10344eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103451(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103451: /* mpyr    exec->words[1],exec->words[0],a         x:(exec->addresses[1]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103451u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103452: /* mpyr    exec->words[1],exec->words[2],b         y:(exec->addresses[5]),exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[5], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103452u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103453: /* mpyr    exec->words[1],exec->words[0],a         a,x:(exec->addresses[0])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103453u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103454: /* mpyr    exec->words[1],exec->words[2],b         b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103454u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103455: /* mpy     exec->words[3],exec->words[1],a         a,x:(exec->addresses[1])+       y:(exec->addresses[4]),exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[4], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103455u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103456: /* move    x:(exec->addresses[0]),exec->words[0]       b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103456u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103457: /* move    a1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103457u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103458(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103458: /* move    exec->words[1],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103458u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103459: /* move    y:(exec->addresses[6]+$c),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103459u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10345a: /* move    #$21,exec->addresses[0] */
    exec->addresses[0] = (0x21u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10345au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10345b: /* move    y:(exec->addresses[6]+$10),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 16u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10345bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10345c: /* move    y:(exec->addresses[6]+$19),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 25u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10345cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10345d: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10345du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10345e: /* move    x:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10345eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103461(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103461: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103461u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103462: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103462u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103463: /* and     #>$7fff,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x103463u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103465: /* move    a1,exec->addresses[1] */
    exec->addresses[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103465u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103466: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103466u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103467: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103467u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103468: /* move    x:(exec->addresses[1]+$148000),exec->words[1] */
    exec->words[1] = (u24(*(sine + exec->addresses[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103468u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10346a: /* move    exec->words[1],x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10346au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10346b(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10346b: /* mpyr    exec->words[1],exec->words[2],b         x:(exec->addresses[0]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10346bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10346c(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10346c: /* move    a1,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10346cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10346d: /* move    b1,y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x10346du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10346e: /* move    #$21,exec->addresses[0] */
    exec->addresses[0] = (0x21u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10346eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10346f: /* move    y:(exec->addresses[6]+$12),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 18u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10346fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103470: /* move    y:(exec->addresses[6]+$1a),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 26u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103470u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103471: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103471u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103472: /* move    y:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_y_raw(v, md_shared_scratch_y, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103472u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103475(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103475: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103475u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103476: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103476u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103477: /* and     #>$7fff,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x103477u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103479: /* move    a1,exec->addresses[1] */
    exec->addresses[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103479u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10347a: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10347au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10347b: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10347bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10347c: /* move    x:(exec->addresses[1]+$148000),exec->words[1] */
    exec->words[1] = (u24(*(sine + exec->addresses[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10347cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10347e: /* move    exec->words[1],y:(exec->addresses[0])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10347eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10347f(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10347f: /* mpyr    exec->words[1],exec->words[2],b         y:(exec->addresses[0]),exec->words[1] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10347fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103480(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103480: /* move    a1,y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103480u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103481: /* move    b1,y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103481u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103482: /* move    #$41,exec->addresses[0] */
    exec->addresses[0] = (0x41u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103482u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103483: /* move    y:(exec->addresses[6]+$14),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 20u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103483u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103484: /* move    y:(exec->addresses[6]+$1b),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 27u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103484u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103485: /* move    y:(exec->addresses[6]+$3),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103485u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103486: /* move    x:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103486u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103489(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103489: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103489u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10348a: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10348au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10348b: /* and     #>$7fff,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x10348bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10348d: /* move    a1,exec->addresses[1] */
    exec->addresses[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10348du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10348e: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10348eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10348f: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10348fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103490: /* move    x:(exec->addresses[1]+$148000),exec->words[1] */
    exec->words[1] = (u24(*(sine + exec->addresses[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103490u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103492: /* move    exec->words[1],x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x103492u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103493(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103493: /* mpyr    exec->words[1],exec->words[2],b         x:(exec->addresses[0]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103493u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103494(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103494: /* move    a1,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103494u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103495: /* move    b1,y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103495u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103496: /* move    #$41,exec->addresses[0] */
    exec->addresses[0] = (0x41u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103496u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103497: /* move    y:(exec->addresses[6]+$16),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 22u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103497u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103498: /* move    y:(exec->addresses[6]+$1c),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 28u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103498u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103499: /* move    y:(exec->addresses[6]+$4),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103499u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10349a: /* move    y:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_y_raw(v, md_shared_scratch_y, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10349au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10349d(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10349d: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10349du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10349e: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10349eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10349f: /* and     #>$7fff,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x10349fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034a1: /* move    a1,exec->addresses[1] */
    exec->addresses[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034a1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034a2: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034a2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034a3: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034a3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034a4: /* move    x:(exec->addresses[1]+$148000),exec->words[1] */
    exec->words[1] = (u24(*(sine + exec->addresses[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034a4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034a6: /* move    exec->words[1],y:(exec->addresses[0])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1034a6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034a7(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034a7: /* mpyr    exec->words[1],exec->words[2],b         y:(exec->addresses[0]),exec->words[1] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034a7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034a8(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034a8: /* move    a1,y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1034a8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034a9: /* move    b1,y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1034a9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034aa: /* move    exec->addresses[7],exec->addresses[3] */
    exec->addresses[3] = (exec->addresses[7]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034aau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ab: /* move    #$21,exec->addresses[0] */
    exec->addresses[0] = (0x21u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034abu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ac: /* move    exec->addresses[0],exec->addresses[4] */
    exec->addresses[4] = (exec->addresses[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ad: /* move    #$41,exec->addresses[1] */
    exec->addresses[1] = (0x41u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034adu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ae: /* move    exec->addresses[1],exec->addresses[5] */
    exec->addresses[5] = (exec->addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034aeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034af: /* move    x:(exec->addresses[0])+,a       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[0] = 0u; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034afu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034b0(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034b0: /* add     exec->words[2],a            x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b1: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034b1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b2: /* add     exec->words[2],a            x:(exec->addresses[0])+,b       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b3: /* asr     #$2,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 2u; exec->lows[0] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b4: /* add     exec->words[2],b            x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034b4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b5: /* add     exec->words[0],b            a,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034b5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b6: /* add     exec->words[2],b            x:(exec->addresses[0])+,a       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034b7: /* asr     #$2,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 2u; exec->lows[1] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034b7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034ba(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034ba: /* add     exec->words[2],a            x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034bau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034bb: /* add     exec->words[0],a            b,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034bc: /* add     exec->words[2],a            x:(exec->addresses[0])+,b       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034bcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034bd: /* asr     #$2,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 2u; exec->lows[0] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034bdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034be: /* add     exec->words[2],b            x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034beu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034bf: /* add     exec->words[0],b            a,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034bfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c0: /* add     exec->words[2],b            x:(exec->addresses[0])+,a       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034c0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c1: /* asr     #$2,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 2u; exec->lows[1] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034c1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034c2(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034c2: /* nop */

    MD_TRANSLATION_TRACE(0x1034c2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c3: /* move    b,y:(exec->addresses[7]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7], sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1034c3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c4: /* move    exec->addresses[3],exec->addresses[7] */
    exec->addresses[7] = (exec->addresses[3]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034c4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c5: /* move    y:(exec->addresses[6]+$18),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034c5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c6: /* move    y:(exec->addresses[6]+$5),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034c6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c7: /* move    y:(exec->addresses[6]+$e),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034c7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c8: /* mpysu   exec->words[0],exec->words[3],a */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[3], &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034c8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034c9: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034c9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034ca(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034ca: /* mpy     exec->words[1],exec->words[0],b         y:(exec->addresses[7]),exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[7], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034cau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034cb: /* move    a,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034cbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034cc: /* move    b1,exec->words[0] */
    exec->words[0] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034ccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034cd: /* mpyr    exec->words[3],exec->words[2],b         x:$e,exec->words[3] */
    exec->parallel_words[0] = u24(*(md_shared_scratch_x + 14u)); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034cdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034d0(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034d0: /* mpysu   exec->words[0],exec->words[3],a */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[3], &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034d0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d1: /* asl     a               b1,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = u24(exec->accum[1]); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034d1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d2: /* mpy     exec->words[1],exec->words[0],b         y:(exec->addresses[7]),exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[7], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034d2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d3: /* move    a,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034d3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d4: /* move    b1,exec->words[0] */
    exec->words[0] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034d4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d5: /* mpyr    exec->words[3],exec->words[2],b         x:$e,exec->words[3] */
    exec->parallel_words[0] = u24(*(md_shared_scratch_x + 14u)); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034d5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034d6(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034d6: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1034d6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d7: /* move    b1,y:(exec->addresses[7]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7], sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x1034d7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d8: /* move    exec->addresses[3],exec->addresses[7] */
    exec->addresses[7] = (exec->addresses[3]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034d8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d9: /* move    exec->addresses[3],exec->addresses[4] */
    exec->addresses[4] = (exec->addresses[3]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034d9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034da: /* move    #$1d,exec->addresses[2] */
    exec->addresses[2] = (0x1du) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034dau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034db: /* move    y:(exec->addresses[6]+$d),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 13u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1034dbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034dc: /* asr     #$1,a,b */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[1] = exec->accum[2] >> 1u; exec->lows[1] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034dcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034dd: /* move    #$40,exec->words[0] */
    exec->words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034ddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034de(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034de: /* add     exec->words[0],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034deu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034df: /* move    a1,exec->words[2] */
    exec->words[2] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034dfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e0: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034e0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e1: /* move    y:(exec->addresses[7])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034e1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e2: /* mpy     exec->words[3],exec->words[1],a */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034e2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e3: /* move    x:(exec->addresses[2]),exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034e3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e4: /* move    y:(exec->addresses[6]+$1e),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 30u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034e4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e5: /* sub     exec->words[0],a            a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034e6(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034e6: /* macr    exec->words[3],exec->words[2],a         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034e6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e7: /* mpy     exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e8: /* move    x:(exec->addresses[2]),exec->words[0]       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034e8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034e9: /* sub     exec->words[0],b            b1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[1]); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034e9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ea: /* macr    exec->words[3],exec->words[2],b         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034eau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034ed(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034ed: /* mpy     exec->words[3],exec->words[1],a         a,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034edu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ee: /* move    x:(exec->addresses[2]),exec->words[0]       b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034eeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ef: /* sub     exec->words[0],a            a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034efu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f0: /* macr    exec->words[3],exec->words[2],a         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034f0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f1: /* mpy     exec->words[3],exec->words[1],b         b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034f1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f2: /* move    x:(exec->addresses[2]),exec->words[0]       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034f2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f3: /* sub     exec->words[0],b            b1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[1]); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1034f3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f4: /* macr    exec->words[3],exec->words[2],b         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034f5(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034f5: /* move    a,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1034f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f6: /* move    b,y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1034f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f7: /* move    b,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x1034f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f8: /* move    exec->addresses[3],exec->addresses[7] */
    exec->addresses[7] = (exec->addresses[3]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034f9: /* move    exec->addresses[3],exec->addresses[4] */
    exec->addresses[4] = (exec->addresses[3]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034f9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034fa: /* move    #$1f,exec->addresses[2] */
    exec->addresses[2] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034fau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034fb: /* move    y:(exec->addresses[6]+$d),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 13u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1034fbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034fc: /* asr     #$1,a,b */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[1] = exec->accum[2] >> 1u; exec->lows[1] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_1034fd(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_1034fd: /* move    #$40,exec->words[0] */
    exec->words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034fe: /* add     exec->words[0],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1034feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ff: /* move    a1,exec->words[2] */
    exec->words[2] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1034ffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103500: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103500u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103501: /* move    y:(exec->addresses[7])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103501u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103502: /* mpy     exec->words[3],exec->words[1],a */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103502u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103503: /* move    x:(exec->addresses[2]),exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103503u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103504: /* move    y:(exec->addresses[6]+$20),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 32u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103504u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103505(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103505: /* sub     exec->words[0],a            a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103505u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103506: /* macr    exec->words[3],exec->words[2],a         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103506u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103507: /* mpy     exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103507u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103508: /* move    x:(exec->addresses[2]),exec->words[0]       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103508u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103509: /* sub     exec->words[0],b            b1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[1]); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103509u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10350a: /* macr    exec->words[3],exec->words[2],b         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10350au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_10350d(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_10350d: /* mpy     exec->words[3],exec->words[1],a         a,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10350du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10350e: /* move    x:(exec->addresses[2]),exec->words[0]       b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10350eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10350f: /* sub     exec->words[0],a            a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10350fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103510: /* macr    exec->words[3],exec->words[2],a         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->accum[0] = (int32_t)((uint32_t)exec->accum[0] + (uint32_t)(exec->lows[0] > 0x800000u || (exec->lows[0] == 0x800000u && (exec->accum[0] & 1)))); exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103510u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103511: /* mpy     exec->words[3],exec->words[1],b         b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103511u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103512: /* move    x:(exec->addresses[2]),exec->words[0]       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103512u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103513: /* sub     exec->words[0],b            b1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[1]); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103513u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103514: /* macr    exec->words[3],exec->words[2],b         y:(exec->addresses[7])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->accum[1] = (int32_t)((uint32_t)exec->accum[1] + (uint32_t)(exec->lows[1] > 0x800000u || (exec->lows[1] == 0x800000u && (exec->accum[1] & 1)))); exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103514u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void efmcy_render_block_44k1_group_103515(Efmcy_Render_Block_44K1Exec *exec, EfmcyVoice *v, const int32_t *sine) {
L_103515: /* move    a,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103515u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103516: /* move    b,y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103516u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103517: /* move    b,y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x103517u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103518: /* move    x:$1d,a1 */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 29u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103518u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103519: /* move    x:$1f,b1 */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 31u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103519u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10351a: /* move    a1,y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10351au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10351b: /* move    b1,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x10351bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void efmcy_render_block_44k1(EfmcyVoice *v, const int32_t *sine) {
    Efmcy_Render_Block_44K1Exec execution;
    Efmcy_Render_Block_44K1Exec *exec;
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
    uint32_t loop_103423 = 0;
    uint32_t loop_103438 = 0;
    uint32_t loop_10344f = 0;
    uint32_t loop_10345f = 0;
    uint32_t loop_103473 = 0;
    uint32_t loop_103487 = 0;
    uint32_t loop_10349b = 0;
    uint32_t loop_1034b8 = 0;
    uint32_t loop_1034ce = 0;
    uint32_t loop_1034eb = 0;
    uint32_t loop_10350b = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_10340f:
    efmcy_render_block_44k1_group_10340f(exec,v,sine);
L_103419:
    efmcy_render_block_44k1_group_103419(exec,v,sine);
L_103423: /* do      #<$20,>$10342b */
    loop_103423 = 0x20u; if (loop_103423 == 0u) goto L_10342b;
    MD_TRANSLATION_TRACE(0x103423u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103425:
    efmcy_render_block_44k1_group_103425(exec,v,sine);
    if (--loop_103423 != 0u) goto L_103425;
L_10342b:
    efmcy_render_block_44k1_group_10342b(exec,v,sine);
L_103438: /* do      #<$20,>$103440 */
    loop_103438 = 0x20u; if (loop_103438 == 0u) goto L_103440;
    MD_TRANSLATION_TRACE(0x103438u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10343a:
    efmcy_render_block_44k1_group_10343a(exec,v,sine);
    if (--loop_103438 != 0u) goto L_10343a;
L_103440:
    efmcy_render_block_44k1_group_103440(exec,v,sine);
L_10344c:
    efmcy_render_block_44k1_group_10344c(exec,v,sine);
L_10344f: /* do      #<$20,>$103458 */
    loop_10344f = 0x20u; if (loop_10344f == 0u) goto L_103458;
    MD_TRANSLATION_TRACE(0x10344fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103451:
    efmcy_render_block_44k1_group_103451(exec,v,sine);
    if (--loop_10344f != 0u) goto L_103451;
L_103458:
    efmcy_render_block_44k1_group_103458(exec,v,sine);
L_10345f: /* do      #<$20,>$10346c */
    loop_10345f = 0x20u; if (loop_10345f == 0u) goto L_10346c;
    MD_TRANSLATION_TRACE(0x10345fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103461:
    efmcy_render_block_44k1_group_103461(exec,v,sine);
L_10346b:
    efmcy_render_block_44k1_group_10346b(exec,v,sine);
    if (--loop_10345f != 0u) goto L_103461;
L_10346c:
    efmcy_render_block_44k1_group_10346c(exec,v,sine);
L_103473: /* do      #<$20,>$103480 */
    loop_103473 = 0x20u; if (loop_103473 == 0u) goto L_103480;
    MD_TRANSLATION_TRACE(0x103473u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103475:
    efmcy_render_block_44k1_group_103475(exec,v,sine);
L_10347f:
    efmcy_render_block_44k1_group_10347f(exec,v,sine);
    if (--loop_103473 != 0u) goto L_103475;
L_103480:
    efmcy_render_block_44k1_group_103480(exec,v,sine);
L_103487: /* do      #<$20,>$103494 */
    loop_103487 = 0x20u; if (loop_103487 == 0u) goto L_103494;
    MD_TRANSLATION_TRACE(0x103487u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103489:
    efmcy_render_block_44k1_group_103489(exec,v,sine);
L_103493:
    efmcy_render_block_44k1_group_103493(exec,v,sine);
    if (--loop_103487 != 0u) goto L_103489;
L_103494:
    efmcy_render_block_44k1_group_103494(exec,v,sine);
L_10349b: /* do      #<$20,>$1034a8 */
    loop_10349b = 0x20u; if (loop_10349b == 0u) goto L_1034a8;
    MD_TRANSLATION_TRACE(0x10349bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10349d:
    efmcy_render_block_44k1_group_10349d(exec,v,sine);
L_1034a7:
    efmcy_render_block_44k1_group_1034a7(exec,v,sine);
    if (--loop_10349b != 0u) goto L_10349d;
L_1034a8:
    efmcy_render_block_44k1_group_1034a8(exec,v,sine);
L_1034b0:
    efmcy_render_block_44k1_group_1034b0(exec,v,sine);
L_1034b8: /* do      #<$f,>$1034c2 */
    loop_1034b8 = 0xfu; if (loop_1034b8 == 0u) goto L_1034c2;
    MD_TRANSLATION_TRACE(0x1034b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ba:
    efmcy_render_block_44k1_group_1034ba(exec,v,sine);
    if (--loop_1034b8 != 0u) goto L_1034ba;
L_1034c2:
    efmcy_render_block_44k1_group_1034c2(exec,v,sine);
L_1034ca:
    efmcy_render_block_44k1_group_1034ca(exec,v,sine);
L_1034ce: /* do      #<$1f,>$1034d6 */
    loop_1034ce = 0x1fu; if (loop_1034ce == 0u) goto L_1034d6;
    MD_TRANSLATION_TRACE(0x1034ceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034d0:
    efmcy_render_block_44k1_group_1034d0(exec,v,sine);
    if (--loop_1034ce != 0u) goto L_1034d0;
L_1034d6:
    efmcy_render_block_44k1_group_1034d6(exec,v,sine);
L_1034de:
    efmcy_render_block_44k1_group_1034de(exec,v,sine);
L_1034e6:
    efmcy_render_block_44k1_group_1034e6(exec,v,sine);
L_1034eb: /* do      #<$f,>$1034f5 */
    loop_1034eb = 0xfu; if (loop_1034eb == 0u) goto L_1034f5;
    MD_TRANSLATION_TRACE(0x1034ebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1034ed:
    efmcy_render_block_44k1_group_1034ed(exec,v,sine);
    if (--loop_1034eb != 0u) goto L_1034ed;
L_1034f5:
    efmcy_render_block_44k1_group_1034f5(exec,v,sine);
L_1034fd:
    efmcy_render_block_44k1_group_1034fd(exec,v,sine);
L_103505:
    efmcy_render_block_44k1_group_103505(exec,v,sine);
L_10350b: /* do      #<$f,>$103515 */
    loop_10350b = 0xfu; if (loop_10350b == 0u) goto L_103515;
    MD_TRANSLATION_TRACE(0x10350bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10350d:
    efmcy_render_block_44k1_group_10350d(exec,v,sine);
    if (--loop_10350b != 0u) goto L_10350d;
L_103515:
    efmcy_render_block_44k1_group_103515(exec,v,sine);
L_10351c: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t efmcy_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_20280c: /* lea.l -$18(a7), a7 */
    /* local register save area */
C_202810: /* movem.l d[2]-d[5]/a2-a3, (a7) */
    /* native locals replace original callee saves */
C_202814: /* movea.l $1c(a7), a1 */
    /* resolved argument pointer */
C_202818: /* movea.l $20(a7), a2 */
    /* resolved argument pointer */
C_20281c: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20281e: /* mulu.w #$5, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x5u);
C_202822: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_202824: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20282a: /* move.l d[0], d[2] */
    d[2] = d[0];
C_20282c: /* asr.l #$7, d[2] */
    d[2] = (uint32_t)((int32_t)d[2] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_20282e: /* moveq #$7f, d[1] */
    d[1] = 0x0000007fu;
C_202830: /* and.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; d[0] = lhs & rhs;
C_202832: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_202838: /* move.l (a0, d[0].l * 4), d[1] */
    d[1] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_20283c: /* lsl.l d[2], d[1] */
    d[1] = (((d[2]) & 63u) < 32u ? d[1] << ((d[2]) & 63u) : 0u);
C_20283e: /* move.l d[1], d[2] */
    d[2] = d[1];
C_202840: /* lsl.l #$3, d[2] */
    d[2] = (((0x3u) & 63u) < 32u ? d[2] << ((0x3u) & 63u) : 0u);
C_202842: /* move.l d[1], d[0] */
    d[0] = d[1];
C_202844: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_202846: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_202848: /* move.l d[0], d[2] */
    d[2] = d[0];
C_20284a: /* lsl.l #$4, d[2] */
    d[2] = (((0x4u) & 63u) < 32u ? d[2] << ((0x4u) & 63u) : 0u);
C_20284c: /* sub.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_20284e: /* add.l d[1], d[2] */
    lhs = d[2]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[2] = lhs + rhs;
C_202850: /* sub.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_202852: /* movea.l d[0], a3 */
    a3 = d[0];
C_202854: /* lea.l (a3, d[0].l * 4), a0 */
    a0 = (d[0] * 4u + (0)) + a3;
C_202858: /* movea.l d[1], a3 */
    a3 = d[1];
C_20285a: /* lea.l (a3, a0.l * 4), a3 */
    a3 = (a0 * 4u + (0)) + a3;
C_20285e: /* move.l a3, d[4] */
    d[4] = a3;
C_202860: /* move.l d[0], d[3] */
    d[3] = d[0];
C_202862: /* lsl.l #$4, d[3] */
    d[3] = (((0x4u) & 63u) < 32u ? d[3] << ((0x4u) & 63u) : 0u);
C_202864: /* sub.l d[0], d[3] */
    lhs = d[3]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[3] = lhs - rhs;
C_202866: /* movea.l d[1], a0 */
    a0 = d[1];
C_202868: /* lea.l (a0, d[3].l * 2), a0 */
    a0 = (d[3] * 2u + (0)) + a0;
C_20286c: /* move.l a0, d[3] */
    d[3] = a0;
C_20286e: /* move.l d[1], d[0] */
    d[0] = d[1];
C_202870: /* asr.l #$4, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x4u) & 63u) < 32u ? ((0x4u) & 63u) : 31u));
C_202872: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_202876: /* move.l d[2], d[0] */
    d[0] = d[2];
C_202878: /* moveq #$c, d[5] */
    d[5] = 0x0000000cu;
C_20287a: /* asr.l d[5], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_20287c: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_202880: /* move.l a3, d[0] */
    d[0] = a3;
C_202882: /* asr.l d[5], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_202884: /* move.l d[0], $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_202888: /* move.l a0, d[0] */
    d[0] = a0;
C_20288a: /* asr.l d[5], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_20288c: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_202890: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202894: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_202898: /* asr.l d[5], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_20289a: /* lea.l $24bf94.l, a3 */
    /* resolved table pointer */
C_2028a0: /* lea.l (a3, d[0].l * 4), a0 */
    a0 = d[0] * 4u + (0);
C_2028a4: /* move.l $24c(a0), $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_2028aa: /* clr.l d[0] */
    d[0] = 0u;
C_2028ac: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_2028b0: /* addi.l #$400, d[0] */
    lhs = d[0]; rhs = 0x400u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2028b6: /* muls.l d[0], d[1] */
    d[1] = d[1] * (d[0]);
C_2028ba: /* move.b #$f, d[5] */
    d[5] = (d[5] & 0xffffff00u) | ((0xfu) & 0xffu);
C_2028be: /* asr.l d[5], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_2028c0: /* move.l d[1], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[1]);
C_2028c4: /* asr.l #$8, d[2] */
    d[2] = (uint32_t)((int32_t)d[2] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2028c6: /* muls.l d[0], d[2] */
    d[2] = d[2] * (d[0]);
C_2028ca: /* asr.l d[5], d[2] */
    d[2] = (uint32_t)((int32_t)d[2] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_2028cc: /* move.l d[2], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[2]);
C_2028d0: /* asr.l #$8, d[4] */
    d[4] = (uint32_t)((int32_t)d[4] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2028d2: /* muls.l d[0], d[4] */
    d[4] = d[4] * (d[0]);
C_2028d6: /* asr.l d[5], d[4] */
    d[4] = (uint32_t)((int32_t)d[4] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_2028d8: /* move.l d[4], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[4]);
C_2028dc: /* asr.l #$8, d[3] */
    d[3] = (uint32_t)((int32_t)d[3] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2028de: /* muls.l d[0], d[3] */
    d[3] = d[3] * (d[0]);
C_2028e2: /* asr.l d[5], d[3] */
    d[3] = (uint32_t)((int32_t)d[3] >> (((d[5]) & 63u) < 32u ? ((d[5]) & 63u) : 31u));
C_2028e4: /* move.l d[3], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[3]);
C_2028e8: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_2028ec: /* mulu.w #$3, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x3u);
C_2028f0: /* addi.l #$c00, d[0] */
    lhs = d[0]; rhs = 0xc00u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2028f6: /* move.l d[0], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[0]);
C_2028fa: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_2028fe: /* mulu.w #$28, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x28u);
C_202902: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_202904: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_202906: /* lea.l (a3, d[0].l * 4), a0 */
    a0 = d[0] * 4u + (0);
C_20290a: /* move.l $578(a0), $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((1400u) + a0)) / 4u)));
C_202910: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_202914: /* mulu.w #$cd, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xcdu);
C_202918: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_20291a: /* move.l d[0], $30(a1) */
    *(y + (48u) / 4u) = (int32_t)(d[0]);
C_20291e: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_202922: /* mulu.w #$ba, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xbau);
C_202926: /* move.l #$7dfffe, d[5] */
    d[5] = 0x7dfffeu;
C_20292c: /* sub.l d[0], d[5] */
    lhs = d[5]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[5] = lhs - rhs;
C_20292e: /* move.l d[5], $34(a1) */
    *(y + (52u) / 4u) = (int32_t)(d[5]);
C_202932: /* movea.l #$400000, a0 */
    a0 = 0x400000u;
C_202938: /* move.l a0, $38(a1) */
    *(y + (56u) / 4u) = (int32_t)(a0);
C_20293c: /* moveq #$f, d[0] */
    d[0] = 0x0000000fu;
C_20293e: /* movem.l (a7), d[2]-d[5]/a2-a3 */
    /* native locals replace original callee saves */
C_202942: /* lea.l $18(a7), a7 */
    /* local register save area */
C_202946: /* rts  */
    return d[0];
}

void efmcy_update_coeffs(const uint32_t *p, EfmcyVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmcy_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmcy_dsp_init(v, 0); }
}
void efmcy_voice_init(EfmcyVoice *v) {
    uint32_t i;
    for (i = 0u; i < 33u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmcy_voice_trigger(EfmcyVoice *v) {
    efmcy_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmcy_resample_run(EfmcyVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmcyVoice
#define MD_RENDER48 efmcy_render_block_48k_float
#define MD_RENDER44 efmcy_render_block_44k1
#define MD_RESAMPLE_RUN efmcy_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmcyVoice
#define MD_SLOT_INIT efmcy_slot_init
#define MD_SLOT_PROCESS efmcy_slot_process
#define MD_VOICE_INIT efmcy_voice_init
#define MD_COEFFICIENTS efmcy_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmcy_update_coeffs
#define MD_VOICE_TRIGGER efmcy_voice_trigger
#define MD_RENDER44 efmcy_render_block_44k1
#define MD_RENDER48 efmcy_render_block_48k_float
#include "../engine_slot.inc"
#endif
