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
    int32_t y[47];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} TrxcyVoice;
extern uint32_t md_control_tempo_denominator;
extern uint32_t md_divs32(uint32_t, uint32_t);
extern uint32_t md_divu32(uint32_t, uint32_t);

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

static uint32_t md_read_source(TrxcyVoice *v, uint32_t address) {
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

static uint32_t md_read_x(TrxcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(TrxcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(TrxcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2095u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(TrxcyVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2095u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(TrxcyVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(TrxcyVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(TrxcyVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef TRXCY_DSP_INIT_EXEC_DEFINED
#define TRXCY_DSP_INIT_EXEC_DEFINED
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
} Trxcy_Dsp_InitExec;
#endif
static void trxcy_dsp_init_group_1023c9(Trxcy_Dsp_InitExec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023c9: /* move    #>$680,exec->words[0] */
    exec->words[0] = (0x680u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023c9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023cb: /* move    exec->words[0],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023cbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023cc: /* move    #$40,exec->words[0] */
    exec->words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023ccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023cd: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023cdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023ce: /* move    #>$0,exec->words[0] */
    exec->words[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023ceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d0: /* move    exec->words[0],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d1: /* move    exec->words[0],y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d2: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_dsp_init_group_1023d3(Trxcy_Dsp_InitExec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023d3: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d4: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d5: /* move    exec->words[0],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d6: /* move    exec->words[0],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d7: /* move    exec->words[0],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d8: /* move    exec->words[0],y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023d9: /* move    exec->words[0],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023d9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023da: /* move    exec->words[0],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023dau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_dsp_init_group_1023db(Trxcy_Dsp_InitExec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023db: /* move    exec->words[0],y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023dbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023dc: /* move    exec->words[0],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023dcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023dd: /* move    exec->words[0],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023ddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023de: /* move    exec->words[0],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023deu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023df: /* move    exec->words[0],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023dfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e0: /* move    exec->words[0],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e1: /* move    exec->words[0],y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e2: /* move    exec->words[0],y:(exec->addresses[6]+$27) */
    *(v->y + 39u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_dsp_init_group_1023e3(Trxcy_Dsp_InitExec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023e3: /* move    exec->words[0],y:(exec->addresses[6]+$28) */
    *(v->y + 40u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e4: /* move    exec->words[0],y:(exec->addresses[6]+$29) */
    *(v->y + 41u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e5: /* move    exec->words[0],y:(exec->addresses[6]+$2a) */
    *(v->y + 42u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e6: /* move    exec->words[0],y:(exec->addresses[6]+$2b) */
    *(v->y + 43u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e7: /* move    exec->words[0],y:(exec->addresses[6]+$2c) */
    *(v->y + 44u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e8: /* move    exec->words[0],y:(exec->addresses[6]+$2d) */
    *(v->y + 45u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023e9: /* move    exec->words[0],y:(exec->addresses[6]+$2e) */
    *(v->y + 46u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023e9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxcy_dsp_init(TrxcyVoice *v, const int32_t *sine) {
    Trxcy_Dsp_InitExec execution;
    Trxcy_Dsp_InitExec *exec;
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
L_1023c9:
    trxcy_dsp_init_group_1023c9(exec,v,sine);
L_1023d3:
    trxcy_dsp_init_group_1023d3(exec,v,sine);
L_1023db:
    trxcy_dsp_init_group_1023db(exec,v,sine);
L_1023e3:
    trxcy_dsp_init_group_1023e3(exec,v,sine);
L_1023ea: /* rts */
    return;}

#ifndef TRXCY_DSP_TRIGGER_EXEC_DEFINED
#define TRXCY_DSP_TRIGGER_EXEC_DEFINED
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
} Trxcy_Dsp_TriggerExec;
#endif
static void trxcy_dsp_trigger_group_1023eb(Trxcy_Dsp_TriggerExec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023eb: /* move    #>$50c335,b */
    exec->accum[1] = sx24((int32_t)(0x50c335u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1023ebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023ed: /* move    y:(exec->addresses[6]+$13),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023edu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023ee: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1023eeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023ef: /* move    #>$50c335,a */
    exec->accum[0] = sx24((int32_t)(0x50c335u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1023efu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f1: /* move    y:(exec->addresses[6]+$14),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023f1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f2: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1023f2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f3: /* move    b,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x1023f3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f4: /* move    a,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x1023f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_dsp_trigger_group_1023f5(Trxcy_Dsp_TriggerExec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023f5: /* move    y:(exec->addresses[6]+$c),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f6: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f7: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023f8: /* move    exec->words[0],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1023f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxcy_dsp_trigger(TrxcyVoice *v, const int32_t *sine) {
    Trxcy_Dsp_TriggerExec execution;
    Trxcy_Dsp_TriggerExec *exec;
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
L_1023eb:
    trxcy_dsp_trigger_group_1023eb(exec,v,sine);
L_1023f5:
    trxcy_dsp_trigger_group_1023f5(exec,v,sine);
L_1023f9: /* rts */
    return;}

#ifndef TRXCY_RENDER_BLOCK_44K1_EXEC_DEFINED
#define TRXCY_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Trxcy_Render_Block_44K1Exec;
#endif
static void trxcy_render_block_44k1_group_1023fa(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1023fa: /* move    #>$60,exec->addresses[4] */
    exec->addresses[4] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023fau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023fc: /* move    #$1f,exec->addresses[12] */
    exec->addresses[12] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023fd: /* move    #>$60,exec->addresses[5] */
    exec->addresses[5] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1023ff: /* move    #$1f,exec->addresses[13] */
    exec->addresses[13] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1023ffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102400: /* move    y:(exec->addresses[6]+$15),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 21u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102400u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102401: /* move    #>$102600,exec->words[2] */
    exec->words[2] = (0x102600u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102401u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102403: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102403u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102404: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102404u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102405(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102405: /* move    b,exec->addresses[3] */
    exec->addresses[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102405u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102406: /* move    #>$71,exec->addresses[11] */
    exec->addresses[11] = (0x71u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102406u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10240a(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10240a: /* move    x:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10240au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10240b: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10240bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10240c: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10240cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10240d: /* mpy     exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10240du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10240e: /* move    a,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x10240eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10240f: /* move    b,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10240fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102410(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102410: /* move    exec->addresses[3],b */
    exec->accum[1] = sx24((int32_t)(exec->addresses[3])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102410u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102411: /* sub     exec->words[2],b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102411u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102412: /* nop */

    MD_TRANSLATION_TRACE(0x102412u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102413: /* move    b,y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102413u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102414: /* move    y:(exec->addresses[6]+$16),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 22u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102414u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102415: /* move    #>$102700,exec->words[2] */
    exec->words[2] = (0x102700u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102415u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102417: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102417u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102418: /* move    y:(exec->addresses[6]+$3),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102418u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102419(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102419: /* move    b,exec->addresses[3] */
    exec->addresses[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102419u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10241a: /* move    #>$65,exec->addresses[11] */
    exec->addresses[11] = (0x65u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10241au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10241e(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10241e: /* move    x:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10241eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10241f: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10241fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102420: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[4])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102420u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102421: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102421u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102422: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102422u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102423: /* add     exec->words[3],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102423u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102424: /* move    a,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102424u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102425: /* move    b,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102425u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102426(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102426: /* move    exec->addresses[3],b */
    exec->accum[1] = sx24((int32_t)(exec->addresses[3])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102426u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102427: /* sub     exec->words[2],b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102427u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102428: /* nop */

    MD_TRANSLATION_TRACE(0x102428u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102429: /* move    b,y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102429u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10242a: /* move    y:(exec->addresses[6]+$17),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 23u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10242au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10242b: /* move    #>$102780,exec->words[2] */
    exec->words[2] = (0x102780u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10242bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10242d: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10242du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10242e: /* move    y:(exec->addresses[6]+$4),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10242eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10242f(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10242f: /* move    b,exec->addresses[3] */
    exec->addresses[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10242fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102430: /* move    #>$55,exec->addresses[11] */
    exec->addresses[11] = (0x55u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102430u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102434(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102434: /* move    x:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102434u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102435: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102435u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102436: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[4])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102436u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102437: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102437u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102438: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102438u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102439: /* add     exec->words[3],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102439u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10243a: /* move    a,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x10243au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10243b: /* move    b,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10243bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10243c(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10243c: /* move    exec->addresses[3],b */
    exec->accum[1] = sx24((int32_t)(exec->addresses[3])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10243cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10243d: /* sub     exec->words[2],b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10243du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10243e: /* nop */

    MD_TRANSLATION_TRACE(0x10243eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10243f: /* move    b,y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10243fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102440: /* move    y:(exec->addresses[6]+$18),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 24u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102440u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102441: /* move    #>$102800,exec->words[2] */
    exec->words[2] = (0x102800u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102441u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102443: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102443u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102444: /* move    y:(exec->addresses[6]+$5),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102444u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102445(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102445: /* move    b,exec->addresses[3] */
    exec->addresses[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102445u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102446: /* move    #>$4f,exec->addresses[11] */
    exec->addresses[11] = (0x4fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102446u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10244a(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10244a: /* move    x:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10244au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10244b: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10244bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10244c: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[4])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10244cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10244d: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10244du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10244e: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10244eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10244f: /* add     exec->words[3],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10244fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102450: /* move    a,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102450u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102451: /* move    b,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102451u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102452(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102452: /* move    exec->addresses[3],b */
    exec->accum[1] = sx24((int32_t)(exec->addresses[3])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102452u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102453: /* sub     exec->words[2],b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102453u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102454: /* nop */

    MD_TRANSLATION_TRACE(0x102454u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102455: /* move    b,y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102455u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102456: /* move    y:(exec->addresses[6]+$19),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 25u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102456u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102457: /* move    #>$102880,exec->words[2] */
    exec->words[2] = (0x102880u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102457u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102459: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102459u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10245a: /* move    y:(exec->addresses[6]+$6),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10245au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10245b(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10245b: /* move    b,exec->addresses[3] */
    exec->addresses[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10245bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10245c: /* move    #>$39,exec->addresses[11] */
    exec->addresses[11] = (0x39u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10245cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102460(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102460: /* move    x:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102460u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102461: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102461u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102462: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[4])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102462u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102463: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102463u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102464: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102464u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102465: /* add     exec->words[3],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102465u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102466: /* move    a,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102466u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102467: /* move    b,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102467u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102468(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102468: /* move    exec->addresses[3],b */
    exec->accum[1] = sx24((int32_t)(exec->addresses[3])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102468u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102469: /* sub     exec->words[2],b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102469u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10246a: /* nop */

    MD_TRANSLATION_TRACE(0x10246au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10246b: /* move    b,y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10246bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10246c: /* move    y:(exec->addresses[6]+$1a),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 26u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10246cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10246d: /* move    #>$1028c0,exec->words[2] */
    exec->words[2] = (0x1028c0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10246du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10246f: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10246fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102470: /* move    y:(exec->addresses[6]+$7),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102470u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102471(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102471: /* move    b,exec->addresses[3] */
    exec->addresses[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102471u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102472: /* move    #>$33,exec->addresses[11] */
    exec->addresses[11] = (0x33u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102472u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102476(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102476: /* move    x:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102476u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102477: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102477u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102478: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[4])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102478u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102479: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102479u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10247a: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10247au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10247b: /* add     exec->words[3],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10247bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10247c: /* move    a,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x10247cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10247d: /* move    b,x:(exec->addresses[5])+ */
    *(md_shared_scratch_x + (96u + ((exec->addresses[5]++) & 31u))) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x10247du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10247e(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10247e: /* move    exec->addresses[3],b */
    exec->accum[1] = sx24((int32_t)(exec->addresses[3])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10247eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10247f: /* sub     exec->words[2],b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10247fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102480: /* nop */

    MD_TRANSLATION_TRACE(0x102480u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102481: /* move    b,y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x102481u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102482: /* move    #>$ffffff,exec->addresses[11] */
    exec->addresses[11] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102482u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102484: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102484u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102486: /* move    y:(exec->addresses[6]+$11),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 17u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102486u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102487: /* move    y:(exec->addresses[6]+$c),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102487u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102489(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102489: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102489u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10248a: /* move    #$3,exec->addresses[12] */
    exec->addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10248au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10248b: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10248bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10248d: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10248du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10248f: /* add     exec->words[0],a            a1,b1 */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10248fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102490: /* move    #$5e,exec->addresses[0] */
    exec->addresses[0] = (0x5eu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102490u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102491: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102491u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102492: /* move    #$4,exec->addresses[3] */
    exec->addresses[3] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102492u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102493(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102493: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102493u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102494: /* nop */

    MD_TRANSLATION_TRACE(0x102494u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102495: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102495u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102497: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102497u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102499: /* move    #$0,exec->addresses[1] */
    exec->addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102499u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10249a: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10249au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10249b: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10249bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10249c: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10249cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10249d(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10249d: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10249du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10249e: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10249eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10249f: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10249fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a0: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1024a0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a1: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024a1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a2: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024a2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a3: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024a3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a4: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024a4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024a5(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024a5: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024a5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a6: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024a6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a7: /* move    y:(exec->addresses[6]+$1c),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024a7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a8: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024a8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024a9: /* move    y:(exec->addresses[6]+$1b),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024a9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024aa: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024aau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ab: /* move    y:(exec->addresses[6]+$1e),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 30u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024abu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ac: /* move    exec->words[1],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024ad(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024ad: /* move    y:(exec->addresses[6]+$1d),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 29u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024adu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ae: /* move    exec->words[1],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024aeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024af: /* move    y:(exec->addresses[6]+$20),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 32u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024afu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b0: /* move    exec->words[1],x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b1: /* move    y:(exec->addresses[6]+$1f),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 31u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024b1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b2: /* move    exec->words[1],x:(exec->addresses[0]+$1) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0] + 1u, sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1024b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b3: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b4: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024b4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024b5(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024b5: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024b5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024b8(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024b8: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b9: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024b9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ba: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[3])-,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024bau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024bb: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024bc: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024bcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024bd: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024bdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024be: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024beu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024bf: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1]),exec->words[1]       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024bfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024c0(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024c0: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c1: /* asl     b               x:(exec->addresses[1])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c2: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[0])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c3: /* mac     exec->words[2],exec->words[0],a         b,x:(exec->addresses[3])        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024c4(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024c4: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c5: /* move    exec->words[3],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1024c5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c6: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c7: /* move    exec->words[3],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1024c7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c8: /* move    x:(exec->addresses[3])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024c8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024c9: /* move    exec->words[3],y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1024c9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ca: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024cau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024cb: /* move    exec->words[3],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1024cbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024cc(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024cc: /* move    x:(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024ccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024cd: /* move    exec->words[3],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1024cdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ce: /* move    x:-(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024ceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024cf: /* move    exec->words[3],y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1024cfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024d0: /* move    y:(exec->addresses[6]+$8),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 8u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1024d0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024d1: /* move    y:(exec->addresses[6]+$e),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024d1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024d3: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024d3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024d4: /* move    #$1,exec->addresses[12] */
    exec->addresses[12] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024d4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024d5(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024d5: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024d5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024d7: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024d7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024d9: /* add     exec->words[0],a            a1,b */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1024d9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024da: /* move    #$4,exec->addresses[0] */
    exec->addresses[0] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024dau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024db: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024dbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024dc: /* move    #$2,exec->addresses[3] */
    exec->addresses[3] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024dcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024dd: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024ddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024de: /* move    #$2,exec->addresses[16] */
    exec->addresses[16] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024deu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024df(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024df: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024dfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e1: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024e1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e3: /* move    #$0,exec->addresses[1] */
    exec->addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024e3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e4: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024e4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e5: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e6: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1024e6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e7: /* move    #$3,exec->addresses[2] */
    exec->addresses[2] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024e8: /* move    #>$ffffff,exec->addresses[10] */
    exec->addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024e8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024ea(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024ea: /* asl     a               b,exec->words[1] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024eau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024eb: /* move    y:(exec->addresses[6]+$21),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 33u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024ebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ec: /* move    exec->words[3],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x1024ecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ed: /* move    y:(exec->addresses[6]+$22),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 34u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024edu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ee: /* move    exec->words[3],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x1024eeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024ef: /* add     exec->words[1],a            a,exec->words[0] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024efu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f0: /* sub     #>$400000,a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1024f0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f2: /* neg     a */
    exec->accum[0] = (int32_t)(0u - (uint32_t)exec->accum[0] - (uint32_t)(exec->lows[0] != 0u)); exec->lows[0] = (0u - exec->lows[0]) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1024f2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024f3(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024f3: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1024f3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f4: /* move    b,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f5: /* move    a,y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1024f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f6: /* move    y:(exec->addresses[6]+$24),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f7: /* move    exec->words[2],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x1024f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f8: /* move    y:(exec->addresses[6]+$23),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 35u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024f9: /* move    exec->words[2],x:(exec->addresses[2]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x1024f9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024fa: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024fau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1024fb(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1024fb: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024fbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024fc: /* mac     exec->words[0],exec->words[3],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1024fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024fd: /* mac     exec->words[1],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1024fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102500(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102500: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102500u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102501: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102501u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102502: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102502u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102503: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102503u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102504: /* asl     b               x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102504u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102505: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102505u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102506: /* mac     exec->words[0],exec->words[3],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102506u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102507: /* mac     exec->words[1],exec->words[2],a         b,x:(exec->addresses[0])+exec->addresses[16] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16]), 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102507u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102508(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102508: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102508u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102509: /* move    exec->words[3],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102509u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10250a: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10250au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10250b: /* move    exec->words[3],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10250bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10250c: /* move    x:(exec->addresses[2])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10250cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10250d: /* move    exec->words[3],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10250du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10250e: /* move    x:(exec->addresses[2]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10250eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10250f: /* move    exec->words[3],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10250fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102510(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102510: /* move    y:(exec->addresses[6]+$9),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 9u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102510u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102511: /* move    y:(exec->addresses[6]+$f),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102511u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102513: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102513u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102514: /* move    #$3,exec->addresses[12] */
    exec->addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102514u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102515: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102515u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102517: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102517u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102519: /* add     exec->words[0],a            a1,b1 */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102519u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10251a: /* move    #$5e,exec->addresses[0] */
    exec->addresses[0] = (0x5eu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10251au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10251b(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10251b: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10251bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10251c: /* move    #$26,exec->addresses[3] */
    exec->addresses[3] = (0x26u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10251cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10251d: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10251du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10251e: /* nop */

    MD_TRANSLATION_TRACE(0x10251eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10251f: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10251fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102521: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102521u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102523: /* move    #$0,exec->addresses[1] */
    exec->addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102523u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102524: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102524u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102525(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102525: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102525u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102526: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102526u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102527: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102527u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102528: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102528u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102529: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102529u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10252a: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x10252au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10252b: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10252bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10252c: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10252cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10252d(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10252d: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10252du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10252e: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10252eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10252f: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10252fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102530: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102530u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102531: /* move    y:(exec->addresses[6]+$26),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 38u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102531u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102532: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102532u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102533: /* move    y:(exec->addresses[6]+$25),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 37u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102533u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102534: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102534u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102535(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102535: /* move    y:(exec->addresses[6]+$28),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 40u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102535u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102536: /* move    exec->words[1],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102536u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102537: /* move    y:(exec->addresses[6]+$27),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 39u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102537u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102538: /* move    exec->words[1],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102538u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102539: /* move    y:(exec->addresses[6]+$2a),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 42u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102539u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10253a: /* move    exec->words[1],x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10253au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10253b: /* move    y:(exec->addresses[6]+$29),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 41u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10253bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10253c: /* move    exec->words[1],x:(exec->addresses[0]+$1) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0] + 1u, sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10253cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10253d(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10253d: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10253du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10253e: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10253eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10253f: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10253fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102542(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102542: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102542u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102543: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102543u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102544: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[3])-,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102544u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102545: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102545u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102546: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102546u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102547: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102547u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102548: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102548u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102549: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1]),exec->words[1]       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102549u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10254a(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10254a: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10254au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10254b: /* asl     b               x:(exec->addresses[1])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10254bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10254c: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[0])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10254cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10254d: /* mac     exec->words[2],exec->words[0],a         b,x:(exec->addresses[3])        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10254du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10254e(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10254e: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10254eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10254f: /* move    exec->words[3],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10254fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102550: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102550u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102551: /* move    exec->words[3],y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102551u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102552: /* move    x:(exec->addresses[3])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102552u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102553: /* move    exec->words[3],y:(exec->addresses[6]+$27) */
    *(v->y + 39u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102553u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102554: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102554u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102555: /* move    exec->words[3],y:(exec->addresses[6]+$28) */
    *(v->y + 40u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102555u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102556(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102556: /* move    x:(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102556u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102557: /* move    exec->words[3],y:(exec->addresses[6]+$29) */
    *(v->y + 41u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102557u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102558: /* move    x:-(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102558u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102559: /* move    exec->words[3],y:(exec->addresses[6]+$2a) */
    *(v->y + 42u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102559u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10255a: /* move    y:(exec->addresses[6]+$a),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 10u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10255au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10255b: /* move    y:(exec->addresses[6]+$10),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10255bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10255d: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10255du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10255e: /* move    #$1,exec->addresses[12] */
    exec->addresses[12] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10255eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10255f(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10255f: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10255fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102561: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102561u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102563: /* add     exec->words[0],a            a1,b */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102563u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102564: /* move    #$26,exec->addresses[0] */
    exec->addresses[0] = (0x26u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102564u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102565: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102565u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102566: /* move    #$24,exec->addresses[3] */
    exec->addresses[3] = (0x24u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102566u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102567: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102567u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102568: /* move    #$2,exec->addresses[16] */
    exec->addresses[16] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102568u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102569(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102569: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102569u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10256b: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10256bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10256d: /* move    #$0,exec->addresses[1] */
    exec->addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10256du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10256e: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10256eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10256f: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10256fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102570: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102570u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102571: /* move    #$25,exec->addresses[2] */
    exec->addresses[2] = (0x25u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102571u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102572: /* move    #>$ffffff,exec->addresses[10] */
    exec->addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102572u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102574(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102574: /* asl     a               b,exec->words[1] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102574u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102575: /* move    y:(exec->addresses[6]+$2b),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 43u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102575u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102576: /* move    exec->words[3],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x102576u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102577: /* move    y:(exec->addresses[6]+$2c),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 44u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102577u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102578: /* move    exec->words[3],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x102578u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102579: /* add     exec->words[1],a            a,exec->words[0] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102579u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10257a: /* sub     #>$400000,a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10257au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10257c: /* neg     a */
    exec->accum[0] = (int32_t)(0u - (uint32_t)exec->accum[0] - (uint32_t)(exec->lows[0] != 0u)); exec->lows[0] = (0u - exec->lows[0]) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10257cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10257d(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10257d: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x10257du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10257e: /* move    b,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10257eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10257f: /* move    a,y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x10257fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102580: /* move    y:(exec->addresses[6]+$2e),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 46u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102580u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102581: /* move    exec->words[2],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x102581u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102582: /* move    y:(exec->addresses[6]+$2d),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 45u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102582u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102583: /* move    exec->words[2],x:(exec->addresses[2]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x102583u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102584: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102584u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102585(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102585: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102585u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102586: /* mac     exec->words[0],exec->words[3],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102586u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102587: /* mac     exec->words[1],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102587u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10258a(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10258a: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10258au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10258b: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10258bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10258c: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10258cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10258d: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10258du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10258e: /* asl     b               x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10258eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10258f: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10258fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102590: /* mac     exec->words[0],exec->words[3],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102590u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102591: /* mac     exec->words[1],exec->words[2],a         b,x:(exec->addresses[0])+exec->addresses[16] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16]), 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102591u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_102592(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_102592: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102592u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102593: /* move    exec->words[3],y:(exec->addresses[6]+$2b) */
    *(v->y + 43u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102593u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102594: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102594u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102595: /* move    exec->words[3],y:(exec->addresses[6]+$2c) */
    *(v->y + 44u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102595u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102596: /* move    x:(exec->addresses[2])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102596u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102597: /* move    exec->words[3],y:(exec->addresses[6]+$2d) */
    *(v->y + 45u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102597u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102598: /* move    x:(exec->addresses[2]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102598u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102599: /* move    exec->words[3],y:(exec->addresses[6]+$2e) */
    *(v->y + 46u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102599u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_10259a(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_10259a: /* move    #$4,exec->addresses[0] */
    exec->addresses[0] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10259au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10259b: /* move    #$26,exec->addresses[3] */
    exec->addresses[3] = (0x26u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10259bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10259c: /* move    y:(exec->addresses[6]+$13),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10259cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10259d: /* move    y:(exec->addresses[6]+$14),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10259du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1025a0(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1025a0: /* move    x:(exec->addresses[0]),exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025a0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a1: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025a1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a2: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025a2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a3: /* mpy     exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025a3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a4: /* move    a,x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1025a4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a5: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1025a5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1025a6(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1025a6: /* move    y:(exec->addresses[6]+$d),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025a6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a7: /* move    y:(exec->addresses[6]+$1),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025a7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a8: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025a8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a9: /* mpy     exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025a9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025aa: /* move    a,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x1025aau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025ab: /* move    b,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x1025abu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025ac: /* move    #$4,exec->addresses[0] */
    exec->addresses[0] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025ad: /* move    #$26,exec->addresses[3] */
    exec->addresses[3] = (0x26u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025adu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1025ae(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1025ae: /* move    y:(exec->addresses[6]+$b),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025aeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025af: /* move    y:(exec->addresses[6]+$12),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025afu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b0: /* move    x:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b1: /* move    x:(exec->addresses[3])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025b1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b2: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b3: /* mac     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1025b6(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1025b6: /* mpy     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b7: /* asr     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 1u; exec->lows[1] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025b7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b8: /* mac     exec->words[3],exec->words[1],a         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b9: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1025b9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025ba: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025bau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025bb: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025bc: /* mac     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1025bcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025bd: /* move    a,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1025bdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxcy_render_block_44k1_group_1025be(Trxcy_Render_Block_44K1Exec *exec, TrxcyVoice *v, const int32_t *sine) {
L_1025be: /* asr     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 1u; exec->lows[1] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025beu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025bf: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025bfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025c0: /* mac     exec->words[3],exec->words[1],a */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025c0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025c1: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1025c1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025c2: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1025c2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025c3: /* move    a,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1025c3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxcy_render_block_44k1(TrxcyVoice *v, const int32_t *sine) {
    Trxcy_Render_Block_44K1Exec execution;
    Trxcy_Render_Block_44K1Exec *exec;
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
    uint32_t loop_102408 = 0;
    uint32_t loop_10241c = 0;
    uint32_t loop_102432 = 0;
    uint32_t loop_102448 = 0;
    uint32_t loop_10245e = 0;
    uint32_t loop_102474 = 0;
    uint32_t loop_1024b6 = 0;
    uint32_t loop_1024fe = 0;
    uint32_t loop_102540 = 0;
    uint32_t loop_102588 = 0;
    uint32_t loop_10259e = 0;
    uint32_t loop_1025b4 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_1023fa:
    trxcy_render_block_44k1_group_1023fa(exec,v,sine);
L_102405:
    trxcy_render_block_44k1_group_102405(exec,v,sine);
L_102408: /* do      #<$10,>$102410 */
    loop_102408 = 0x10u; if (loop_102408 == 0u) goto L_102410;
    MD_TRANSLATION_TRACE(0x102408u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10240a:
    trxcy_render_block_44k1_group_10240a(exec,v,sine);
    if (--loop_102408 != 0u) goto L_10240a;
L_102410:
    trxcy_render_block_44k1_group_102410(exec,v,sine);
L_102419:
    trxcy_render_block_44k1_group_102419(exec,v,sine);
L_10241c: /* do      #<$10,>$102426 */
    loop_10241c = 0x10u; if (loop_10241c == 0u) goto L_102426;
    MD_TRANSLATION_TRACE(0x10241cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10241e:
    trxcy_render_block_44k1_group_10241e(exec,v,sine);
    if (--loop_10241c != 0u) goto L_10241e;
L_102426:
    trxcy_render_block_44k1_group_102426(exec,v,sine);
L_10242f:
    trxcy_render_block_44k1_group_10242f(exec,v,sine);
L_102432: /* do      #<$10,>$10243c */
    loop_102432 = 0x10u; if (loop_102432 == 0u) goto L_10243c;
    MD_TRANSLATION_TRACE(0x102432u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102434:
    trxcy_render_block_44k1_group_102434(exec,v,sine);
    if (--loop_102432 != 0u) goto L_102434;
L_10243c:
    trxcy_render_block_44k1_group_10243c(exec,v,sine);
L_102445:
    trxcy_render_block_44k1_group_102445(exec,v,sine);
L_102448: /* do      #<$10,>$102452 */
    loop_102448 = 0x10u; if (loop_102448 == 0u) goto L_102452;
    MD_TRANSLATION_TRACE(0x102448u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10244a:
    trxcy_render_block_44k1_group_10244a(exec,v,sine);
    if (--loop_102448 != 0u) goto L_10244a;
L_102452:
    trxcy_render_block_44k1_group_102452(exec,v,sine);
L_10245b:
    trxcy_render_block_44k1_group_10245b(exec,v,sine);
L_10245e: /* do      #<$10,>$102468 */
    loop_10245e = 0x10u; if (loop_10245e == 0u) goto L_102468;
    MD_TRANSLATION_TRACE(0x10245eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102460:
    trxcy_render_block_44k1_group_102460(exec,v,sine);
    if (--loop_10245e != 0u) goto L_102460;
L_102468:
    trxcy_render_block_44k1_group_102468(exec,v,sine);
L_102471:
    trxcy_render_block_44k1_group_102471(exec,v,sine);
L_102474: /* do      #<$10,>$10247e */
    loop_102474 = 0x10u; if (loop_102474 == 0u) goto L_10247e;
    MD_TRANSLATION_TRACE(0x102474u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102476:
    trxcy_render_block_44k1_group_102476(exec,v,sine);
    if (--loop_102474 != 0u) goto L_102476;
L_10247e:
    trxcy_render_block_44k1_group_10247e(exec,v,sine);
L_102489:
    trxcy_render_block_44k1_group_102489(exec,v,sine);
L_102493:
    trxcy_render_block_44k1_group_102493(exec,v,sine);
L_10249d:
    trxcy_render_block_44k1_group_10249d(exec,v,sine);
L_1024a5:
    trxcy_render_block_44k1_group_1024a5(exec,v,sine);
L_1024ad:
    trxcy_render_block_44k1_group_1024ad(exec,v,sine);
L_1024b5:
    trxcy_render_block_44k1_group_1024b5(exec,v,sine);
L_1024b6: /* do      #<$20,>$1024c4 */
    loop_1024b6 = 0x20u; if (loop_1024b6 == 0u) goto L_1024c4;
    MD_TRANSLATION_TRACE(0x1024b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1024b8:
    trxcy_render_block_44k1_group_1024b8(exec,v,sine);
L_1024c0:
    trxcy_render_block_44k1_group_1024c0(exec,v,sine);
    if (--loop_1024b6 != 0u) goto L_1024b8;
L_1024c4:
    trxcy_render_block_44k1_group_1024c4(exec,v,sine);
L_1024cc:
    trxcy_render_block_44k1_group_1024cc(exec,v,sine);
L_1024d5:
    trxcy_render_block_44k1_group_1024d5(exec,v,sine);
L_1024df:
    trxcy_render_block_44k1_group_1024df(exec,v,sine);
L_1024ea:
    trxcy_render_block_44k1_group_1024ea(exec,v,sine);
L_1024f3:
    trxcy_render_block_44k1_group_1024f3(exec,v,sine);
L_1024fb:
    trxcy_render_block_44k1_group_1024fb(exec,v,sine);
L_1024fe: /* do      #<$20,>$102508 */
    loop_1024fe = 0x20u; if (loop_1024fe == 0u) goto L_102508;
    MD_TRANSLATION_TRACE(0x1024feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102500:
    trxcy_render_block_44k1_group_102500(exec,v,sine);
    if (--loop_1024fe != 0u) goto L_102500;
L_102508:
    trxcy_render_block_44k1_group_102508(exec,v,sine);
L_102510:
    trxcy_render_block_44k1_group_102510(exec,v,sine);
L_10251b:
    trxcy_render_block_44k1_group_10251b(exec,v,sine);
L_102525:
    trxcy_render_block_44k1_group_102525(exec,v,sine);
L_10252d:
    trxcy_render_block_44k1_group_10252d(exec,v,sine);
L_102535:
    trxcy_render_block_44k1_group_102535(exec,v,sine);
L_10253d:
    trxcy_render_block_44k1_group_10253d(exec,v,sine);
L_102540: /* do      #<$20,>$10254e */
    loop_102540 = 0x20u; if (loop_102540 == 0u) goto L_10254e;
    MD_TRANSLATION_TRACE(0x102540u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102542:
    trxcy_render_block_44k1_group_102542(exec,v,sine);
L_10254a:
    trxcy_render_block_44k1_group_10254a(exec,v,sine);
    if (--loop_102540 != 0u) goto L_102542;
L_10254e:
    trxcy_render_block_44k1_group_10254e(exec,v,sine);
L_102556:
    trxcy_render_block_44k1_group_102556(exec,v,sine);
L_10255f:
    trxcy_render_block_44k1_group_10255f(exec,v,sine);
L_102569:
    trxcy_render_block_44k1_group_102569(exec,v,sine);
L_102574:
    trxcy_render_block_44k1_group_102574(exec,v,sine);
L_10257d:
    trxcy_render_block_44k1_group_10257d(exec,v,sine);
L_102585:
    trxcy_render_block_44k1_group_102585(exec,v,sine);
L_102588: /* do      #<$20,>$102592 */
    loop_102588 = 0x20u; if (loop_102588 == 0u) goto L_102592;
    MD_TRANSLATION_TRACE(0x102588u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10258a:
    trxcy_render_block_44k1_group_10258a(exec,v,sine);
    if (--loop_102588 != 0u) goto L_10258a;
L_102592:
    trxcy_render_block_44k1_group_102592(exec,v,sine);
L_10259a:
    trxcy_render_block_44k1_group_10259a(exec,v,sine);
L_10259e: /* do      #<$20,>$1025a6 */
    loop_10259e = 0x20u; if (loop_10259e == 0u) goto L_1025a6;
    MD_TRANSLATION_TRACE(0x10259eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025a0:
    trxcy_render_block_44k1_group_1025a0(exec,v,sine);
    if (--loop_10259e != 0u) goto L_1025a0;
L_1025a6:
    trxcy_render_block_44k1_group_1025a6(exec,v,sine);
L_1025ae:
    trxcy_render_block_44k1_group_1025ae(exec,v,sine);
L_1025b4: /* do      #<$f,>$1025be */
    loop_1025b4 = 0xfu; if (loop_1025b4 == 0u) goto L_1025be;
    MD_TRANSLATION_TRACE(0x1025b4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1025b6:
    trxcy_render_block_44k1_group_1025b6(exec,v,sine);
    if (--loop_1025b4 != 0u) goto L_1025b6;
L_1025be:
    trxcy_render_block_44k1_group_1025be(exec,v,sine);
L_1025c4: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t trxcy_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201976: /* lea.l -$1c(a7), a7 */
    /* local register save area */
C_20197a: /* movem.l d[2]-d[7]/a2, (a7) */
    /* native locals replace original callee saves */
C_20197e: /* movea.l $20(a7), a1 */
    /* resolved argument pointer */
C_201982: /* movea.l $24(a7), a2 */
    /* resolved argument pointer */
C_201986: /* move.w (a2), d[3] */
    d[3] = (d[3] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_201988: /* clr.l d[1] */
    d[1] = 0u;
C_20198a: /* move.w d[3], d[1] */
    d[1] = (d[1] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_20198c: /* move.l d[1], d[0] */
    d[0] = d[1];
C_20198e: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_201990: /* cmpi.l #$1fff, d[1] */
    lhs = d[1]; rhs = 0x1fffu;
C_201996: /* bhi.b $2019ca */
    if (lhs > rhs) goto C_2019ca;
C_201998: /* clr.l d[7] */
    d[7] = 0u;
C_20199a: /* move.w d[0], d[7] */
    d[7] = (d[7] & 0xffff0000u) | (((d[0] & 0xffffu)) & 0xffffu);
C_20199c: /* move.l d[1], d[0] */
    d[0] = d[1];
C_20199e: /* lsr.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] >> ((0x2u) & 63u) : 0u);
C_2019a0: /* mulu.w #$4925, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x4925u);
C_2019a4: /* moveq #$11, d[2] */
    d[2] = 0x00000011u;
C_2019a6: /* lsr.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] >> ((d[2]) & 63u) : 0u);
C_2019a8: /* move.l d[0], d[6] */
    d[6] = d[0];
C_2019aa: /* addi.l #$ffffff85, d[6] */
    lhs = d[6]; rhs = 0xffffff85u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[6] = lhs + rhs;
C_2019b0: /* move.l d[0], d[5] */
    d[5] = d[0];
C_2019b2: /* addi.l #$ffffff29, d[5] */
    lhs = d[5]; rhs = 0xffffff29u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[5] = lhs + rhs;
C_2019b8: /* move.l d[7], d[4] */
    d[4] = d[7];
C_2019ba: /* addi.l #$ffffff00, d[4] */
    lhs = d[4]; rhs = 0xffffff00u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[4] = lhs + rhs;
C_2019c0: /* cmpi.l #$8bf, d[1] */
    lhs = d[1]; rhs = 0x8bfu;
C_2019c6: /* bls.b $201a20 */
    if (lhs <= rhs) goto C_201a20;
C_2019c8: /* bra.b $201a34 */
    goto C_201a34;
C_2019ca: /* clr.l d[2] */
    d[2] = 0u;
C_2019cc: /* move.w d[0], d[2] */
    d[2] = (d[2] & 0xffff0000u) | (((d[0] & 0xffffu)) & 0xffffu);
C_2019ce: /* move.l #$200, d[7] */
    d[7] = 0x200u;
C_2019d4: /* sub.l d[2], d[7] */
    lhs = d[7]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[7] = lhs - rhs;
C_2019d6: /* cmpi.l #$2a2f, d[1] */
    lhs = d[1]; rhs = 0x2a2fu;
C_2019dc: /* bls.w $201ab2 */
    if (lhs <= rhs) goto C_201ab2;
C_2019e0: /* move.l d[1], d[0] */
    d[0] = d[1];
C_2019e2: /* lsr.l #$1, d[0] */
    d[0] = (((0x1u) & 63u) < 32u ? d[0] >> ((0x1u) & 63u) : 0u);
C_2019e4: /* mulu.w #$c30d, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xc30du);
C_2019e8: /* moveq #$14, d[4] */
    d[4] = 0x00000014u;
C_2019ea: /* lsr.l d[4], d[0] */
    d[0] = (((d[4]) & 63u) < 32u ? d[0] >> ((d[4]) & 63u) : 0u);
C_2019ec: /* move.l #$200, d[6] */
    d[6] = 0x200u;
C_2019f2: /* sub.l d[0], d[6] */
    lhs = d[6]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[6] = lhs - rhs;
C_2019f4: /* movea.l d[2], a0 */
    a0 = d[2];
C_2019f6: /* lea.l -$100(a0), a0 */
    a0 = ((-256)) + a0;
C_2019fa: /* move.w d[3], d[2] */
    d[2] = (d[2] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_2019fc: /* mulu.w #$469f, d[2] */
    d[2] = (d[2] & 0xffffu) * (0x469fu);
C_201a00: /* cmpi.l #$3461, d[1] */
    lhs = d[1]; rhs = 0x3461u;
C_201a06: /* bls.w $201a90 */
    if (lhs <= rhs) goto C_201a90;
C_201a0a: /* move.w d[3], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201a0c: /* mulu.w #$e071, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xe071u);
C_201a10: /* moveq #$16, d[1] */
    d[1] = 0x00000016u;
C_201a12: /* lsr.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] >> ((d[1]) & 63u) : 0u);
C_201a14: /* move.l #$1b9, d[5] */
    d[5] = 0x1b9u;
C_201a1a: /* sub.l d[0], d[5] */
    lhs = d[5]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[5] = lhs - rhs;
C_201a1c: /* bra.w $201aa4 */
    goto C_201aa4;
C_201a20: /* move.w d[3], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201a22: /* mulu.w #$bc53, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xbc53u);
C_201a26: /* moveq #$16, d[1] */
    d[1] = 0x00000016u;
C_201a28: /* lsr.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] >> ((d[1]) & 63u) : 0u);
C_201a2a: /* move.l #$ffffff1a, d[2] */
    d[2] = 0xffffff1au;
C_201a30: /* sub.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_201a32: /* bra.b $201a52 */
    goto C_201a52;
C_201a34: /* move.w d[3], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201a36: /* mulu.w #$469f, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x469fu);
C_201a3a: /* moveq #$13, d[2] */
    d[2] = 0x00000013u;
C_201a3c: /* lsr.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] >> ((d[2]) & 63u) : 0u);
C_201a3e: /* move.l d[0], d[2] */
    d[2] = d[0];
C_201a40: /* addi.l #$fffffeb3, d[2] */
    lhs = d[2]; rhs = 0xfffffeb3u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[2] = lhs + rhs;
C_201a46: /* clr.l d[0] */
    d[0] = 0u;
C_201a48: /* move.w d[3], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201a4a: /* cmpi.l #$12f1, d[0] */
    lhs = d[0]; rhs = 0x12f1u;
C_201a50: /* bhi.b $201a78 */
    if (lhs > rhs) goto C_201a78;
C_201a52: /* move.w d[3], d[1] */
    d[1] = (d[1] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201a54: /* mulu.w #$5c99, d[1] */
    d[1] = (d[1] & 0xffffu) * (0x5c99u);
C_201a58: /* clr.w d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((0u) & 0xffffu);
C_201a5a: /* swap d[1] */
    d[1] = (d[1] << 16) | (d[1] >> 16);
C_201a5c: /* move.l d[3], d[0] */
    d[0] = d[3];
C_201a5e: /* sub.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_201a60: /* andi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu; d[0] = lhs & rhs;
C_201a66: /* lsr.l #$1, d[0] */
    d[0] = (((0x1u) & 63u) < 32u ? d[0] >> ((0x1u) & 63u) : 0u);
C_201a68: /* add.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201a6a: /* lsr.l #$5, d[1] */
    d[1] = (((0x5u) & 63u) < 32u ? d[1] >> ((0x5u) & 63u) : 0u);
C_201a6c: /* move.l #$ffffff66, d[0] */
    d[0] = 0xffffff66u;
C_201a72: /* sub.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_201a74: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201a76: /* bra.b $201ada */
    goto C_201ada;
C_201a78: /* clr.l d[0] */
    d[0] = 0u;
C_201a7a: /* move.w d[3], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201a7c: /* lsr.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] >> ((0x2u) & 63u) : 0u);
C_201a7e: /* mulu.w #$4925, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x4925u);
C_201a82: /* moveq #$11, d[1] */
    d[1] = 0x00000011u;
C_201a84: /* lsr.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] >> ((d[1]) & 63u) : 0u);
C_201a86: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201a88: /* addi.l #$fffffe54, d[1] */
    lhs = d[1]; rhs = 0xfffffe54u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201a8e: /* bra.b $201ada */
    goto C_201ada;
C_201a90: /* lsr.l #$2, d[1] */
    d[1] = (((0x2u) & 63u) < 32u ? d[1] >> ((0x2u) & 63u) : 0u);
C_201a92: /* move.w d[1], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[1] & 0xffffu)) & 0xffffu);
C_201a94: /* mulu.w #$4925, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x4925u);
C_201a98: /* moveq #$11, d[4] */
    d[4] = 0x00000011u;
C_201a9a: /* lsr.l d[4], d[0] */
    d[0] = (((d[4]) & 63u) < 32u ? d[0] >> ((d[4]) & 63u) : 0u);
C_201a9c: /* move.l d[0], d[5] */
    d[5] = d[0];
C_201a9e: /* addi.l #$ffffff29, d[5] */
    lhs = d[5]; rhs = 0xffffff29u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[5] = lhs + rhs;
C_201aa4: /* move.l a0, d[4] */
    d[4] = a0;
C_201aa6: /* moveq #$13, d[0] */
    d[0] = 0x00000013u;
C_201aa8: /* lsr.l d[0], d[2] */
    d[2] = (((d[0]) & 63u) < 32u ? d[2] >> ((d[0]) & 63u) : 0u);
C_201aaa: /* addi.l #$fffffeb3, d[2] */
    lhs = d[2]; rhs = 0xfffffeb3u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[2] = lhs + rhs;
C_201ab0: /* bra.b $201a78 */
    goto C_201a78;
C_201ab2: /* lsr.l #$2, d[1] */
    d[1] = (((0x2u) & 63u) < 32u ? d[1] >> ((0x2u) & 63u) : 0u);
C_201ab4: /* move.w d[1], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[1] & 0xffffu)) & 0xffffu);
C_201ab6: /* mulu.w #$4925, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x4925u);
C_201aba: /* moveq #$11, d[1] */
    d[1] = 0x00000011u;
C_201abc: /* lsr.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] >> ((d[1]) & 63u) : 0u);
C_201abe: /* move.l d[0], d[6] */
    d[6] = d[0];
C_201ac0: /* addi.l #$ffffff85, d[6] */
    lhs = d[6]; rhs = 0xffffff85u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[6] = lhs + rhs;
C_201ac6: /* move.l d[0], d[5] */
    d[5] = d[0];
C_201ac8: /* addi.l #$ffffff29, d[5] */
    lhs = d[5]; rhs = 0xffffff29u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[5] = lhs + rhs;
C_201ace: /* move.l d[2], d[4] */
    d[4] = d[2];
C_201ad0: /* addi.l #$ffffff00, d[4] */
    lhs = d[4]; rhs = 0xffffff00u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[4] = lhs + rhs;
C_201ad6: /* bra.w $201a34 */
    goto C_201a34;
C_201ada: /* clr.l d[0] */
    d[0] = 0u;
C_201adc: /* move.w d[3], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[3] & 0xffffu)) & 0xffffu);
C_201ade: /* lsr.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] >> ((0x3u) & 63u) : 0u);
C_201ae0: /* move.l #$800, d[3] */
    d[3] = 0x800u;
C_201ae6: /* sub.l d[0], d[3] */
    lhs = d[3]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[3] = lhs - rhs;
C_201ae8: /* move.l d[3], d[0] */
    d[0] = d[3];
C_201aea: /* muls.l d[3], d[7] */
    d[7] = d[7] * (d[3]);
C_201aee: /* addi.l #$63fbc, d[7] */
    lhs = d[7]; rhs = 0x63fbcu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[7] = lhs + rhs;
C_201af4: /* move.l d[7], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[7]);
C_201af8: /* muls.l d[3], d[6] */
    d[6] = d[6] * (d[3]);
C_201afc: /* addi.l #$63fbc, d[6] */
    lhs = d[6]; rhs = 0x63fbcu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[6] = lhs + rhs;
C_201b02: /* move.l d[6], $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(d[6]);
C_201b06: /* muls.l d[3], d[5] */
    d[5] = d[5] * (d[3]);
C_201b0a: /* addi.l #$63fbc, d[5] */
    lhs = d[5]; rhs = 0x63fbcu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[5] = lhs + rhs;
C_201b10: /* move.l d[5], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[5]);
C_201b14: /* muls.l d[3], d[4] */
    d[4] = d[4] * (d[3]);
C_201b18: /* addi.l #$63fbc, d[4] */
    lhs = d[4]; rhs = 0x63fbcu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[4] = lhs + rhs;
C_201b1e: /* move.l d[4], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[4]);
C_201b22: /* muls.l d[3], d[2] */
    d[2] = d[2] * (d[3]);
C_201b26: /* addi.l #$63fbc, d[2] */
    lhs = d[2]; rhs = 0x63fbcu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[2] = lhs + rhs;
C_201b2c: /* move.l d[2], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[2]);
C_201b30: /* muls.l d[1], d[0] */
    d[0] = d[0] * (d[1]);
C_201b34: /* addi.l #$63fbc, d[0] */
    lhs = d[0]; rhs = 0x63fbcu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201b3a: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_201b3e: /* clr.l d[0] */
    d[0] = 0u;
C_201b40: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_201b44: /* add.l d[0], d[0] */
    lhs = d[0]; rhs = d[0]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201b46: /* move.l d[0], d[4] */
    d[4] = d[0];
C_201b48: /* addi.l #$7f67fd, d[4] */
    lhs = d[4]; rhs = 0x7f67fdu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[4] = lhs + rhs;
C_201b4e: /* move.l d[4], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[4]);
C_201b52: /* addi.l #$7f263b, d[0] */
    lhs = d[0]; rhs = 0x7f263bu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201b58: /* move.l d[0], $34(a1) */
    *(y + (52u) / 4u) = (int32_t)(d[0]);
C_201b5c: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_201b60: /* mulu.w #$180, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x180u);
C_201b64: /* addi.l #$200000, d[0] */
    lhs = d[0]; rhs = 0x200000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201b6a: /* move.l d[0], $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(d[0]);
C_201b6e: /* clr.l d[0] */
    d[0] = 0u;
C_201b70: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_201b74: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_201b76: /* addi.l #$600, d[0] */
    lhs = d[0]; rhs = 0x600u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201b7c: /* move.l d[0], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_201b80: /* clr.l d[0] */
    d[0] = 0u;
C_201b82: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_201b86: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_201b88: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201b8a: /* addi.l #$400, d[1] */
    lhs = d[1]; rhs = 0x400u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201b90: /* move.l d[1], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[1]);
C_201b94: /* addi.l #$600, d[0] */
    lhs = d[0]; rhs = 0x600u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201b9a: /* move.l d[0], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[0]);
C_201b9e: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_201ba2: /* mulu.w #$aaab, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xaaabu);
C_201ba6: /* moveq #$18, d[2] */
    d[2] = 0x00000018u;
C_201ba8: /* lsr.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] >> ((d[2]) & 63u) : 0u);
C_201baa: /* addi.l #$20, d[0] */
    lhs = d[0]; rhs = 0x20u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201bb0: /* move.l d[0], $30(a1) */
    *(y + (48u) / 4u) = (int32_t)(d[0]);
C_201bb4: /* moveq #$e, d[0] */
    d[0] = 0x0000000eu;
C_201bb6: /* movem.l (a7), d[2]-d[7]/a2 */
    /* native locals replace original callee saves */
C_201bba: /* lea.l $1c(a7), a7 */
    /* local register save area */
C_201bbe: /* rts  */
    return d[0];
}

void trxcy_update_coeffs(const uint32_t *p, TrxcyVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxcy_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxcy_dsp_init(v, 0); }
}
void trxcy_voice_init(TrxcyVoice *v) {
    uint32_t i;
    for (i = 0u; i < 47u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxcy_voice_trigger(TrxcyVoice *v) {
    trxcy_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxcy_resample_run(TrxcyVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxcyVoice
#define MD_RENDER48 trxcy_render_block_48k_float
#define MD_RENDER44 trxcy_render_block_44k1
#define MD_RESAMPLE_RUN trxcy_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxcyVoice
#define MD_SLOT_INIT trxcy_slot_init
#define MD_SLOT_PROCESS trxcy_slot_process
#define MD_VOICE_INIT trxcy_voice_init
#define MD_COEFFICIENTS trxcy_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxcy_update_coeffs
#define MD_VOICE_TRIGGER trxcy_voice_trigger
#define MD_RENDER44 trxcy_render_block_44k1
#define MD_RENDER48 trxcy_render_block_48k_float
#include "../engine_slot.inc"
#endif
