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
    int32_t y[21];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} TrxrsVoice;
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

static uint32_t md_read_source(TrxrsVoice *v, uint32_t address) {
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

static uint32_t md_read_x(TrxrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
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
static void md_write_x(TrxrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(TrxrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2069u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(TrxrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2069u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(TrxrsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(TrxrsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(TrxrsVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef TRXRS_DSP_INIT_EXEC_DEFINED
#define TRXRS_DSP_INIT_EXEC_DEFINED
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
} Trxrs_Dsp_InitExec;
#endif
static void trxrs_dsp_init_group_101ab0(Trxrs_Dsp_InitExec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101ab0: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ab0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ab1: /* move    exec->words[0],y:(exec->addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ab1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ab2: /* move    #>$2000,exec->words[0] */
    exec->words[0] = (0x2000u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ab2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ab4: /* move    exec->words[0],y:(exec->addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ab4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ab5: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ab5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ab6: /* move    exec->words[0],y:(exec->addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ab6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxrs_dsp_init(TrxrsVoice *v, const int32_t *sine) {
    Trxrs_Dsp_InitExec execution;
    Trxrs_Dsp_InitExec *exec;
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
L_101ab0:
    trxrs_dsp_init_group_101ab0(exec,v,sine);
L_101ab7: /* rts */
    return;}

#ifndef TRXRS_DSP_TRIGGER_EXEC_DEFINED
#define TRXRS_DSP_TRIGGER_EXEC_DEFINED
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
} Trxrs_Dsp_TriggerExec;
#endif
static void trxrs_dsp_trigger_group_101ab8(Trxrs_Dsp_TriggerExec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101ab8: /* move    #>$50c335,b */
    exec->accum[1] = sx24((int32_t)(0x50c335u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101ab8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101aba: /* move    y:(exec->addresses[6]+$9),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101abau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101abb: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101abbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101abc: /* move    #$40,exec->words[0] */
    exec->words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101abcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101abd: /* move    exec->words[0],y:(exec->addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101abdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101abe: /* move    b,y:(exec->addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x101abeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101abf: /* move    #>$200,exec->words[0] */
    exec->words[0] = (0x200u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101abfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac1: /* move    exec->words[0],y:(exec->addresses[6]+$6) */
    *(v->y + 6u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_dsp_trigger_group_101ac2(Trxrs_Dsp_TriggerExec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101ac2: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ac2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac3: /* move    exec->words[0],y:(exec->addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac4: /* move    exec->words[0],y:(exec->addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac5: /* move    exec->words[0],y:(exec->addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac6: /* move    exec->words[0],y:(exec->addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac7: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac8: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ac9: /* move    exec->words[0],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101ac9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_dsp_trigger_group_101aca(Trxrs_Dsp_TriggerExec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101aca: /* move    exec->words[0],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101acau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101acb: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x101acbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxrs_dsp_trigger(TrxrsVoice *v, const int32_t *sine) {
    Trxrs_Dsp_TriggerExec execution;
    Trxrs_Dsp_TriggerExec *exec;
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
L_101ab8:
    trxrs_dsp_trigger_group_101ab8(exec,v,sine);
L_101ac2:
    trxrs_dsp_trigger_group_101ac2(exec,v,sine);
L_101aca:
    trxrs_dsp_trigger_group_101aca(exec,v,sine);
L_101acc: /* rts */
    return;}

#ifndef TRXRS_RENDER_BLOCK_44K1_EXEC_DEFINED
#define TRXRS_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Trxrs_Render_Block_44K1Exec;
#endif
static void trxrs_render_block_44k1_group_101acd(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101acd: /* move    #>$0,exec->addresses[1] */
    exec->addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101acdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101acf: /* move    #>$1f,exec->addresses[9] */
    exec->addresses[9] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101acfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ad1: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ad1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ad3: /* move    #>$60,exec->addresses[4] */
    exec->addresses[4] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ad3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ad5: /* move    #>$1f,exec->addresses[12] */
    exec->addresses[12] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ad5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ad7: /* move    y:(exec->addresses[6]+$2),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 2u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101ad7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ad8: /* asr     #$5,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 5u; exec->lows[0] = ((exec->lows[2] >> 5u) | ((uint32_t)exec->accum[2] << 19u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101ad8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ad9: /* move    y:(exec->addresses[6]+$b),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 11u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101ad9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101ada(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101ada: /* move    y:(exec->addresses[6]+$c),b0 */
    exec->lows[1] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101adau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101adb: /* move    a1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101adbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101adc: /* move    a0,exec->words[0] */
    exec->words[0] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101adcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101adf(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101adf: /* and     #>$7fff,b */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | (u24(exec->accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x101adfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae1: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101ae1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae2: /* add     x,b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101ae2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae3: /* nop */

    MD_TRANSLATION_TRACE(0x101ae3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae4: /* nop */

    MD_TRANSLATION_TRACE(0x101ae4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae5: /* move    x:(exec->addresses[2]+$148000),a */
    exec->accum[0] = sx24((int32_t)(u24(*(sine + exec->addresses[2])))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101ae5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae7: /* move    a,x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101ae7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101ae8(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101ae8: /* move    b1,y:(exec->addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x101ae8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101ae9: /* move    b0,y:(exec->addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(exec->lows[1]));
    MD_TRANSLATION_TRACE(0x101ae9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101aea: /* move    y:(exec->addresses[6]+$a),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 10u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101aeau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101aeb: /* move    y:(exec->addresses[6]+$4),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101aebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101aec: /* move    y:(exec->addresses[6]+$3),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101aecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101aef(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101aef: /* move    x:(exec->addresses[1]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101aefu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101af0: /* cmp     exec->words[3],a */
    exec->accum[2] = sx24((int32_t)(exec->words[3])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x101af0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101af3(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101af3: /* move    #>$7eb852,a */
    exec->accum[0] = sx24((int32_t)(0x7eb852u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101af3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101af5: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101af5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101af6: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101af6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101af9(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101af9: /* neg     a */
    exec->accum[0] = (int32_t)(0u - (uint32_t)exec->accum[0] - (uint32_t)(exec->lows[0] != 0u)); exec->lows[0] = (0u - exec->lows[0]) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101af9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101afa: /* cmp     exec->words[3],a */
    exec->accum[2] = sx24((int32_t)(exec->words[3])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x101afau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101afd(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101afd: /* move    #>$8147ae,a */
    exec->accum[0] = sx24((int32_t)(0x8147aeu)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101afdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101aff: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101affu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b00: /* add     exec->words[2],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b03(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b03: /* neg     a */
    exec->accum[0] = (int32_t)(0u - (uint32_t)exec->accum[0] - (uint32_t)(exec->lows[0] != 0u)); exec->lows[0] = (0u - exec->lows[0]) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b04: /* clr     b */
    exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b05(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b05: /* move    a,x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b06(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b06: /* move    b,y:(exec->addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x101b06u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b07: /* move    exec->addresses[1],exec->addresses[3] */
    exec->addresses[3] = (exec->addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b08: /* move    y:(exec->addresses[6]+$5),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b0b(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b0b: /* move    x:(exec->addresses[1])+,exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b0c: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b0d: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b0e: /* mpy     exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b0f: /* move    a,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b10: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x101b10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b11(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b11: /* move    y:(exec->addresses[6]+$8),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 8u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101b11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b12: /* cmp     #>$300,a */
    exec->accum[2] = sx24((int32_t)(0x300u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x101b12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b16(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b16: /* move    a,b */
    exec->accum[1] = sx24((int32_t)(u24(lim24(exec->accum[0])))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101b16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b17: /* move    #>$101ba0,exec->words[0] */
    exec->words[0] = (0x101ba0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b19: /* add     exec->words[0],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b19u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b1a: /* nop */

    MD_TRANSLATION_TRACE(0x101b1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b1b: /* move    b,exec->addresses[0] */
    exec->addresses[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b1c: /* add     #>$20,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x20u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b1e: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b1f: /* move    a,y:(exec->addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x101b1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b20(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b20: /* move    y:(exec->addresses[6]+$9),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b23(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b23: /* move    x:(exec->addresses[1])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101b23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b24: /* move    x:(exec->addresses[0])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101b24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b25: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b26: /* nop */

    MD_TRANSLATION_TRACE(0x101b26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b27: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b28: /* mpy     exec->words[1],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b29: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b2a: /* move    a,x:(exec->addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b2b(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b2b: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b2c(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b2c: /* move    exec->words[1],y:(exec->addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x101b2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b2f(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b2f: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b30: /* move    y:(exec->addresses[6]+$9),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b33(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b33: /* move    x:(exec->addresses[1])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101b33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b34: /* nop */

    MD_TRANSLATION_TRACE(0x101b34u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b35: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b36: /* mpy     exec->words[1],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b37: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b37u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b38: /* move    a,x:(exec->addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b39: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b3a(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b3a: /* move    exec->words[1],y:(exec->addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x101b3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b3b(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b3b: /* move    #>$ffffff,exec->addresses[11] */
    exec->addresses[11] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b3d: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b3f: /* move    y:(exec->addresses[6]+$6),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 6u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101b3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b40: /* move    y:(exec->addresses[6]+$7),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b42: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b42u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b43: /* move    #$3,exec->addresses[12] */
    exec->addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b43u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b44: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b44u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b46: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b46u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b48(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b48: /* add     exec->words[0],a            a1,b1 */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x101b48u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b49: /* move    #$5e,exec->addresses[0] */
    exec->addresses[0] = (0x5eu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b4a: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b4b: /* move    #$20,exec->addresses[3] */
    exec->addresses[3] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b4c: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b4d: /* nop */

    MD_TRANSLATION_TRACE(0x101b4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b4e: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b4eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b50: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b52(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b52: /* move    #$42,exec->addresses[1] */
    exec->addresses[1] = (0x42u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b53: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b54: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b55: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b56: /* asl     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b56u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b57: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b58: /* add     #>$c00000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0xc00000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b5a: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b5b(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b5b: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b5c: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x101b5cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b5d: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b5e: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b5eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b5f: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b60: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b60u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b61: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b62: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b62u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b63(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b63: /* move    y:(exec->addresses[6]+$e),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b63u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b64: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b65: /* move    y:(exec->addresses[6]+$d),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b66: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b66u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b67: /* move    exec->words[1],a */
    exec->accum[0] = sx24((int32_t)(exec->words[1])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101b67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b68: /* move    y:(exec->addresses[6]+$14),a0 */
    exec->lows[0] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b68u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b69: /* move    y:(exec->addresses[6]+$10),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b6a: /* move    exec->words[1],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b6au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b6b(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b6b: /* move    y:(exec->addresses[6]+$f),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b6c: /* move    exec->words[1],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b6cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b6d: /* move    exec->words[1],b */
    exec->accum[1] = sx24((int32_t)(exec->words[1])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101b6du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b6e: /* move    y:(exec->addresses[6]+$13),b0 */
    exec->lows[1] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b6eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b6f: /* move    y:(exec->addresses[6]+$12),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b6fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b70: /* move    exec->words[1],x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b70u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b71: /* move    y:(exec->addresses[6]+$11),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b71u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b72: /* move    exec->words[1],x:(exec->addresses[0])- */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x101b72u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b73(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b73: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b74: /* mac     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b74u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b75: /* mac     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b76: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b76u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b79(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b79: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b79u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b7a: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b7b: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[3])-,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b7bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b7c: /* move    x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b7d: /* mac     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b7e: /* mac     exec->words[0],exec->words[3],b         a1,exec->words[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b7eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b7f: /* mac     exec->words[0],exec->words[3],b         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b7fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b80: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b81(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b81: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1]),exec->words[1]       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b82: /* mac     exec->words[1],exec->words[2],b         a,l:(exec->addresses[1])+ */
    exec->long_address[0] = md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x101b82u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b83: /* move    x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b83u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b84: /* mac     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b85: /* mac     exec->words[2],exec->words[0],a         b,l:(exec->addresses[3]) */
    exec->long_address[0] = exec->addresses[3]; exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[1]))); exec->parallel_words[3] = (exec->accum[1] > 0x7fffff ? 0xffffffu : exec->accum[1] < -0x800000 ? 0u : exec->lows[1]); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x101b85u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b86: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b87(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b87: /* move    l:(exec->addresses[1])+,a */
    exec->long_address[0] = md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u); exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); exec->accum[0] = sx24((int32_t)exec->parallel_words[0]); exec->lows[0] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x101b87u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b88: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b89: /* move    a1,y:(exec->addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b8a: /* move    a0,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x101b8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b8b: /* move    exec->words[3],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x101b8bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b8c: /* move    l:(exec->addresses[3])-,a */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], -(1), 0u); exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); exec->accum[0] = sx24((int32_t)exec->parallel_words[0]); exec->lows[0] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x101b8cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b8d: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b8e: /* move    a1,y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b8f(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b8f: /* move    a0,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x101b8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b90: /* move    exec->words[3],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x101b90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b91: /* move    x:(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b92: /* move    exec->words[3],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x101b92u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b93: /* move    x:-(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b94: /* move    exec->words[3],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x101b94u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b95: /* move    #>$20,exec->addresses[3] */
    exec->addresses[3] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101b95u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxrs_render_block_44k1_group_101b99(Trxrs_Render_Block_44K1Exec *exec, TrxrsVoice *v, const int32_t *sine) {
L_101b99: /* move    x:(exec->addresses[3])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101b99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b9a: /* move    x:(exec->addresses[3])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101b9au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b9b: /* asl     #$1,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b9c: /* asl     #$1,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x101b9cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b9d: /* move    a,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x101b9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b9e: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x101b9eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxrs_render_block_44k1(TrxrsVoice *v, const int32_t *sine) {
    Trxrs_Render_Block_44K1Exec execution;
    Trxrs_Render_Block_44K1Exec *exec;
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
    uint32_t loop_101add = 0;
    uint32_t loop_101aed = 0;
    uint32_t loop_101b09 = 0;
    uint32_t loop_101b21 = 0;
    uint32_t loop_101b31 = 0;
    uint32_t loop_101b77 = 0;
    uint32_t loop_101b97 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_101acd:
    trxrs_render_block_44k1_group_101acd(exec,v,sine);
L_101ada:
    trxrs_render_block_44k1_group_101ada(exec,v,sine);
L_101add: /* do      #<$20,>$101ae8 */
    loop_101add = 0x20u; if (loop_101add == 0u) goto L_101ae8;
    MD_TRANSLATION_TRACE(0x101addu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101adf:
    trxrs_render_block_44k1_group_101adf(exec,v,sine);
    if (--loop_101add != 0u) goto L_101adf;
L_101ae8:
    trxrs_render_block_44k1_group_101ae8(exec,v,sine);
L_101aed: /* do      #<$20,>$101b06 */
    loop_101aed = 0x20u; if (loop_101aed == 0u) goto L_101b06;
    MD_TRANSLATION_TRACE(0x101aedu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101aef:
    trxrs_render_block_44k1_group_101aef(exec,v,sine);
L_101af1: /* blt     func_101af9 */
    if (exec->flags[1]) goto L_101af9;
    MD_TRANSLATION_TRACE(0x101af1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101af3:
    trxrs_render_block_44k1_group_101af3(exec,v,sine);
L_101af7: /* bra     func_101b05 */
    goto L_101b05;
    MD_TRANSLATION_TRACE(0x101af7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101af9:
    trxrs_render_block_44k1_group_101af9(exec,v,sine);
L_101afb: /* blt     func_101b03 */
    if (exec->flags[1]) goto L_101b03;
    MD_TRANSLATION_TRACE(0x101afbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101afd:
    trxrs_render_block_44k1_group_101afd(exec,v,sine);
L_101b01: /* bra     func_101b05 */
    goto L_101b05;
    MD_TRANSLATION_TRACE(0x101b01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b03:
    trxrs_render_block_44k1_group_101b03(exec,v,sine);
L_101b05:
    trxrs_render_block_44k1_group_101b05(exec,v,sine);
    if (--loop_101aed != 0u) goto L_101aef;
L_101b06:
    trxrs_render_block_44k1_group_101b06(exec,v,sine);
L_101b09: /* do      #<$10,>$101b11 */
    loop_101b09 = 0x10u; if (loop_101b09 == 0u) goto L_101b11;
    MD_TRANSLATION_TRACE(0x101b09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b0b:
    trxrs_render_block_44k1_group_101b0b(exec,v,sine);
    if (--loop_101b09 != 0u) goto L_101b0b;
L_101b11:
    trxrs_render_block_44k1_group_101b11(exec,v,sine);
L_101b14: /* bge     func_101b2f */
    if (!exec->flags[1]) goto L_101b2f;
    MD_TRANSLATION_TRACE(0x101b14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b16:
    trxrs_render_block_44k1_group_101b16(exec,v,sine);
L_101b20:
    trxrs_render_block_44k1_group_101b20(exec,v,sine);
L_101b21: /* do      #<$20,>$101b2c */
    loop_101b21 = 0x20u; if (loop_101b21 == 0u) goto L_101b2c;
    MD_TRANSLATION_TRACE(0x101b21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b23:
    trxrs_render_block_44k1_group_101b23(exec,v,sine);
L_101b2b:
    trxrs_render_block_44k1_group_101b2b(exec,v,sine);
    if (--loop_101b21 != 0u) goto L_101b23;
L_101b2c:
    trxrs_render_block_44k1_group_101b2c(exec,v,sine);
L_101b2d: /* bra     func_101b3b */
    goto L_101b3b;
    MD_TRANSLATION_TRACE(0x101b2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b2f:
    trxrs_render_block_44k1_group_101b2f(exec,v,sine);
L_101b31: /* do      #<$20,>$101b3a */
    loop_101b31 = 0x20u; if (loop_101b31 == 0u) goto L_101b3a;
    MD_TRANSLATION_TRACE(0x101b31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b33:
    trxrs_render_block_44k1_group_101b33(exec,v,sine);
    if (--loop_101b31 != 0u) goto L_101b33;
L_101b3a:
    trxrs_render_block_44k1_group_101b3a(exec,v,sine);
L_101b3b:
    trxrs_render_block_44k1_group_101b3b(exec,v,sine);
L_101b48:
    trxrs_render_block_44k1_group_101b48(exec,v,sine);
L_101b52:
    trxrs_render_block_44k1_group_101b52(exec,v,sine);
L_101b5b:
    trxrs_render_block_44k1_group_101b5b(exec,v,sine);
L_101b63:
    trxrs_render_block_44k1_group_101b63(exec,v,sine);
L_101b6b:
    trxrs_render_block_44k1_group_101b6b(exec,v,sine);
L_101b73:
    trxrs_render_block_44k1_group_101b73(exec,v,sine);
L_101b77: /* do      #<$20,>$101b87 */
    loop_101b77 = 0x20u; if (loop_101b77 == 0u) goto L_101b87;
    MD_TRANSLATION_TRACE(0x101b77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b79:
    trxrs_render_block_44k1_group_101b79(exec,v,sine);
L_101b81:
    trxrs_render_block_44k1_group_101b81(exec,v,sine);
    if (--loop_101b77 != 0u) goto L_101b79;
L_101b87:
    trxrs_render_block_44k1_group_101b87(exec,v,sine);
L_101b8f:
    trxrs_render_block_44k1_group_101b8f(exec,v,sine);
L_101b97: /* do      #<$10,>$101b9f */
    loop_101b97 = 0x10u; if (loop_101b97 == 0u) goto L_101b9f;
    MD_TRANSLATION_TRACE(0x101b97u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_101b99:
    trxrs_render_block_44k1_group_101b99(exec,v,sine);
    if (--loop_101b97 != 0u) goto L_101b99;
L_101b9f: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t trxrs_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2016b8: /* move.l a2, -(a7) */
    /* local register save */
C_2016ba: /* move.l d[2], -(a7) */
    /* local register save */
C_2016bc: /* movea.l $c(a7), a2 */
    /* resolved argument pointer */
C_2016c0: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_2016c4: /* clr.l d[0] */
    d[0] = 0u;
C_2016c6: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_2016c8: /* lsr.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] >> ((0x2u) & 63u) : 0u);
C_2016ca: /* addi.l #$1000, d[0] */
    lhs = d[0]; rhs = 0x1000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2016d0: /* move.l d[0], $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_2016d4: /* move.w $2(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_2016d8: /* mulu.w #$28, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x28u);
C_2016dc: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_2016de: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2016e0: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2016e2: /* movea.l d[0], a0 */
    a0 = d[0];
C_2016e4: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2016ea: /* move.l $370(a0), $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((880u) + a0)) / 4u)));
C_2016f0: /* clr.l d[0] */
    d[0] = 0u;
C_2016f2: /* move.w $4(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2016f6: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2016f8: /* lsl.l #$5, d[1] */
    d[1] = (((0x5u) & 63u) < 32u ? d[1] << ((0x5u) & 63u) : 0u);
C_2016fa: /* move.l #$7fffff, d[2] */
    d[2] = 0x7fffffu;
C_201700: /* sub.l d[1], d[2] */
    lhs = d[2]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_201702: /* move.l d[2], $c(a2) */
    *(y + (12u) / 4u) = (int32_t)(d[2]);
C_201706: /* lsl.l #$4, d[0] */
    d[0] = (((0x4u) & 63u) < 32u ? d[0] << ((0x4u) & 63u) : 0u);
C_201708: /* move.l d[0], $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20170c: /* moveq #$5, d[0] */
    d[0] = 0x00000005u;
C_20170e: /* move.l (a7)+, d[2] */
    /* local register restore */
C_201710: /* movea.l (a7)+, a2 */
    /* local register restore */
C_201712: /* rts  */
    return d[0];
}

void trxrs_update_coeffs(const uint32_t *p, TrxrsVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxrs_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxrs_dsp_init(v, 0); }
}
void trxrs_voice_init(TrxrsVoice *v) {
    uint32_t i;
    for (i = 0u; i < 21u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxrs_voice_trigger(TrxrsVoice *v) {
    trxrs_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxrs_resample_run(TrxrsVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxrsVoice
#define MD_RENDER48 trxrs_render_block_48k_float
#define MD_RENDER44 trxrs_render_block_44k1
#define MD_RESAMPLE_RUN trxrs_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxrsVoice
#define MD_SLOT_INIT trxrs_slot_init
#define MD_SLOT_PROCESS trxrs_slot_process
#define MD_VOICE_INIT trxrs_voice_init
#define MD_COEFFICIENTS trxrs_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxrs_update_coeffs
#define MD_VOICE_TRIGGER trxrs_voice_trigger
#define MD_RENDER44 trxrs_render_block_44k1
#define MD_RENDER48 trxrs_render_block_48k_float
#include "../engine_slot.inc"
#endif
