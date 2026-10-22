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
    int32_t y[41];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} TrxmaVoice;
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

static uint32_t md_read_source(TrxmaVoice *v, uint32_t address) {
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

static uint32_t md_read_x(TrxmaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
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
static void md_write_x(TrxmaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(TrxmaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2089u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(TrxmaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2089u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(TrxmaVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(TrxmaVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(TrxmaVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef TRXMA_DSP_INIT_EXEC_DEFINED
#define TRXMA_DSP_INIT_EXEC_DEFINED
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
} Trxma_Dsp_InitExec;
#endif
static void trxma_dsp_init_group_1028f4(Trxma_Dsp_InitExec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1028f4: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1028f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028f5: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1028f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028f6: /* move    #>$7f,exec->words[0] */
    exec->words[0] = (0x7fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1028f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028f8: /* move    exec->words[0],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1028f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028f9: /* move    #>$29a,exec->words[0] */
    exec->words[0] = (0x29au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1028f9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028fb: /* move    exec->words[0],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1028fbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028fc: /* move    #>$1,exec->words[0] */
    exec->words[0] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1028fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1028fe: /* move    exec->words[0],y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1028feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_dsp_init_group_1028ff(Trxma_Dsp_InitExec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1028ff: /* move    #>$0,exec->words[0] */
    exec->words[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1028ffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102901: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102901u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102902: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102902u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102903: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102903u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102904: /* move    exec->words[0],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102904u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102905: /* move    exec->words[0],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102905u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102906: /* move    exec->words[0],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102906u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102907: /* move    exec->words[0],y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102907u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_dsp_init_group_102908(Trxma_Dsp_InitExec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102908: /* move    exec->words[0],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102908u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102909: /* move    exec->words[0],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102909u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10290a: /* move    exec->words[0],y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10290au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10290b: /* move    exec->words[0],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10290bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10290c: /* move    exec->words[0],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10290cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10290d: /* move    exec->words[0],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10290du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10290e: /* move    exec->words[0],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10290eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10290f: /* move    exec->words[0],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10290fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_dsp_init_group_102910(Trxma_Dsp_InitExec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102910: /* move    exec->words[0],y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102910u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102911: /* move    exec->words[0],y:(exec->addresses[6]+$27) */
    *(v->y + 39u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102911u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102912: /* move    exec->words[0],y:(exec->addresses[6]+$28) */
    *(v->y + 40u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102912u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxma_dsp_init(TrxmaVoice *v, const int32_t *sine) {
    Trxma_Dsp_InitExec execution;
    Trxma_Dsp_InitExec *exec;
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
L_1028f4:
    trxma_dsp_init_group_1028f4(exec,v,sine);
L_1028ff:
    trxma_dsp_init_group_1028ff(exec,v,sine);
L_102908:
    trxma_dsp_init_group_102908(exec,v,sine);
L_102910:
    trxma_dsp_init_group_102910(exec,v,sine);
L_102913: /* rts */
    return;}

#ifndef TRXMA_DSP_TRIGGER_EXEC_DEFINED
#define TRXMA_DSP_TRIGGER_EXEC_DEFINED
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
} Trxma_Dsp_TriggerExec;
#endif
static void trxma_dsp_trigger_group_102914(Trxma_Dsp_TriggerExec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102914: /* move    #>$7a3e22,exec->words[0] */
    exec->words[0] = (0x7a3e22u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102914u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102916: /* move    exec->words[0],y:(exec->addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102916u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102917: /* move    #>$450d02,exec->words[0] */
    exec->words[0] = (0x450d02u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102917u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102919: /* move    exec->words[0],y:(exec->addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102919u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10291a: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10291au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10291b: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10291bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10291c: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10291cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10291d: /* move    exec->words[0],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10291du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxma_dsp_trigger(TrxmaVoice *v, const int32_t *sine) {
    Trxma_Dsp_TriggerExec execution;
    Trxma_Dsp_TriggerExec *exec;
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
L_102914:
    trxma_dsp_trigger_group_102914(exec,v,sine);
L_10291e: /* rts */
    return;}

#ifndef TRXMA_RENDER_BLOCK_44K1_EXEC_DEFINED
#define TRXMA_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Trxma_Render_Block_44K1Exec;
#endif
static void trxma_render_block_44k1_group_10291f(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10291f: /* move    #>$2,exec->addresses[5] */
    exec->addresses[5] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10291fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102921: /* move    #>$ffffff,exec->addresses[13] */
    exec->addresses[13] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102921u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102923: /* move    y:(exec->addresses[6]+$15),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 21u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102923u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102924: /* move    y:(exec->addresses[6]+$16),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 22u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102924u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102927(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102927: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102927u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102928: /* move    exec->words[1],exec->words[0] */
    exec->words[0] = (exec->words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102928u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102929: /* move    b1,x:(exec->addresses[5])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x102929u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10292a: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10292au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10292b: /* move    exec->words[0],b1 */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10292bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_10292c(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10292c: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10292cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10292d: /* move    exec->words[1],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10292du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10292e: /* move    b1,y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x10292eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10292f: /* move    b1,x:(exec->addresses[5])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x10292fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102930: /* move    #>$ffffff,exec->addresses[11] */
    exec->addresses[11] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102930u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102932: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102932u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102934: /* move    y:(exec->addresses[6]+$a),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 10u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102934u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102935: /* move    y:(exec->addresses[6]+$10),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102935u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102937(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102937: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102937u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102938: /* move    #$3,exec->addresses[12] */
    exec->addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102938u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102939: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102939u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10293b: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10293bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10293d: /* add     exec->words[0],a            a1,b1 */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10293du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10293e: /* move    #$0,exec->addresses[0] */
    exec->addresses[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10293eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10293f: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10293fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102940: /* move    #$46,exec->addresses[3] */
    exec->addresses[3] = (0x46u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102940u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102941(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102941: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102941u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102942: /* nop */

    MD_TRANSLATION_TRACE(0x102942u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102943: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102943u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102945: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102945u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102947: /* move    #$70,exec->addresses[1] */
    exec->addresses[1] = (0x70u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102947u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102948: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102948u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102949: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102949u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10294a: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10294au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_10294b(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10294b: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10294bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10294c: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10294cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10294d: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10294du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10294e: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x10294eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10294f: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10294fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102950: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102950u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102951: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102951u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102952: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102952u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102953(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102953: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102953u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102954: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102954u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102955: /* move    y:(exec->addresses[6]+$24),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102955u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102956: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102956u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102957: /* move    y:(exec->addresses[6]+$23),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 35u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102957u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102958: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102958u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102959: /* move    y:(exec->addresses[6]+$26),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 38u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102959u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10295a: /* move    exec->words[1],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10295au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_10295b(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10295b: /* move    y:(exec->addresses[6]+$25),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 37u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10295bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10295c: /* move    exec->words[1],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10295cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10295d: /* move    y:(exec->addresses[6]+$28),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 40u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10295du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10295e: /* move    exec->words[1],x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x10295eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10295f: /* move    y:(exec->addresses[6]+$27),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 39u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10295fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102960: /* move    exec->words[1],x:(exec->addresses[0]+$1) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0] + 1u, sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102960u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102961: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102961u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102962: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102962u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102963(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102963: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102963u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102966(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102966: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102966u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102967: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102967u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102968: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[3])-,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102968u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102969: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102969u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10296a: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10296au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10296b: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10296bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10296c: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10296cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10296d: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1]),exec->words[1]       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10296du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_10296e(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10296e: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10296eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10296f: /* asl     b               x:(exec->addresses[1])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10296fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102970: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[0])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102970u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102971: /* mac     exec->words[2],exec->words[0],a         b,x:(exec->addresses[3])        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102971u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102972(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102972: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102972u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102973: /* move    exec->words[3],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102973u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102974: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102974u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102975: /* move    exec->words[3],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102975u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102976: /* move    x:(exec->addresses[3])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102976u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102977: /* move    exec->words[3],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102977u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102978: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102978u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102979: /* move    exec->words[3],y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102979u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_10297a(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10297a: /* move    x:(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10297au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10297b: /* move    exec->words[3],y:(exec->addresses[6]+$27) */
    *(v->y + 39u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10297bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10297c: /* move    x:-(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10297cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10297d: /* move    exec->words[3],y:(exec->addresses[6]+$28) */
    *(v->y + 40u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x10297du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10297e: /* move    #$46,exec->addresses[0] */
    exec->addresses[0] = (0x46u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10297eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10297f: /* move    #$20,exec->addresses[3] */
    exec->addresses[3] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10297fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102982(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102982: /* move    x:(exec->addresses[0])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102982u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102983: /* move    x:(exec->addresses[0])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102983u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102984: /* move    a,y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x102984u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102985: /* move    b,y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x102985u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102986(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102986: /* move    y:(exec->addresses[6]+$c),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102986u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102987: /* move    y:(exec->addresses[6]+$d),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102987u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102988: /* move    exec->words[0],x:>$66 */
    *(md_shared_scratch_x + 102u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x102988u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10298a: /* move    exec->words[1],x:>$67 */
    *(md_shared_scratch_x + 103u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x10298au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10298c: /* move    y:(exec->addresses[6]+$6),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10298cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10298d: /* move    y:(exec->addresses[6]+$13),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 19u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10298du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10298e: /* move    y:(exec->addresses[6]+$12),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10298eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10298f: /* move    y:(exec->addresses[6]+$14),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10298fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102990(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102990: /* move    #>$22,exec->addresses[5] */
    exec->addresses[5] = (0x22u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102990u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102992: /* move    #>$22,exec->addresses[4] */
    exec->addresses[4] = (0x22u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102992u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102994: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102994u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102998(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102998: /* move    x:-(exec->addresses[5]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], -1, 1u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102998u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102999: /* cmp     exec->words[2],a */
    exec->accum[2] = sx24((int32_t)(exec->words[2])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x102999u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_10299c(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_10299c: /* move    #>$fffcb9,exec->words[0] */
    exec->words[0] = (0xfffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10299cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10299e: /* move    x:(exec->addresses[5]),b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[5], sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10299eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10299f: /* abs     b */
    if (exec->accum[1] < 0) { exec->accum[1] = (int32_t)(0u - (uint32_t)exec->accum[1] - (uint32_t)(exec->lows[1] != 0u)); exec->lows[1] = (0u - exec->lows[1]) & 0xffffffu; } exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x10299fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029a0: /* neg     b */
    exec->accum[1] = (int32_t)(0u - (uint32_t)exec->accum[1] - (uint32_t)(exec->lows[1] != 0u)); exec->lows[1] = (0u - exec->lows[1]) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029a0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029a1: /* move    x:>$67,exec->words[1] */
    exec->words[1] = (u24(*(md_shared_scratch_x + 103u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029a1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029a5(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029a5: /* tst     b */
    exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029a5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029a8(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029a8: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029a8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029a9: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029a9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029aa: /* nop */

    MD_TRANSLATION_TRACE(0x1029aau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ab: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029abu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ac: /* cmp     b,a */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x1029acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029af(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029af: /* neg     b */
    exec->accum[1] = (int32_t)(0u - (uint32_t)exec->accum[1] - (uint32_t)(exec->lows[1] != 0u)); exec->lows[1] = (0u - exec->lows[1]) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029afu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029b0: /* move    x:>$66,exec->words[1] */
    exec->words[1] = (u24(*(md_shared_scratch_x + 102u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029b2: /* move    b,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029b3: /* move    #$0,b */
    exec->accum[1] = sx24((int32_t)(((0x0u) << 16))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1029b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029b6(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029b6: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029b6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029b7: /* nop */

    MD_TRANSLATION_TRACE(0x1029b7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029b8: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029b8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029b9(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029b9: /* move    a,x:(exec->addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x1029b9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029ba(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029ba: /* move    b,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x1029bau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029bb: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1029bbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029bc: /* move    exec->words[1],y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1029bcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029bd: /* move    y:(exec->addresses[6]+$7),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 7u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1029bdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029be: /* move    y:(exec->addresses[6]+$8),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029beu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c0: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c1: /* move    #$3,exec->addresses[12] */
    exec->addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c2: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029c4(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029c4: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c6: /* add     exec->words[0],a            a1,b1 */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1029c6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c7: /* move    #$22,exec->addresses[0] */
    exec->addresses[0] = (0x22u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c8: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029c9: /* move    #$46,exec->addresses[3] */
    exec->addresses[3] = (0x46u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029c9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ca: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029cau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029cb: /* nop */

    MD_TRANSLATION_TRACE(0x1029cbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029cc: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029ccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029ce(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029ce: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029ceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d0: /* move    #$70,exec->addresses[1] */
    exec->addresses[1] = (0x70u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029d0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d1: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029d1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d2: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029d2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d3: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029d3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d4: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1029d4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d5: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029d5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d6: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029d6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029d7(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029d7: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x1029d7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d8: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029d8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029d9: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029d9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029da: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029dau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029db: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029dbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029dc: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029dcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029dd: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029ddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029de: /* move    y:(exec->addresses[6]+$18),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029deu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029df(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029df: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029dfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e0: /* move    y:(exec->addresses[6]+$17),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029e0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e1: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029e1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e2: /* move    y:(exec->addresses[6]+$1a),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 26u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029e2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e3: /* move    exec->words[1],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029e3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e4: /* move    y:(exec->addresses[6]+$19),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029e4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e5: /* move    exec->words[1],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029e5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e6: /* move    y:(exec->addresses[6]+$1c),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029e6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029e7(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029e7: /* move    exec->words[1],x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029e7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e8: /* move    y:(exec->addresses[6]+$1b),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029e8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029e9: /* move    exec->words[1],x:(exec->addresses[0]+$1) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0] + 1u, sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x1029e9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ea: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029eau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029eb: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029ebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ec: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029ecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029ef(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029ef: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029efu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f0: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f1: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[3])-,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f2: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f3: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f4: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f5: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f6: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1]),exec->words[1]       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029f7(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029f7: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f8: /* asl     b               x:(exec->addresses[1])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029f9: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[0])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029f9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029fa: /* mac     exec->words[2],exec->words[0],a         b,x:(exec->addresses[3])        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029fau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_1029fb(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_1029fb: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029fbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029fc: /* move    exec->words[3],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1029fcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029fd: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029fdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029fe: /* move    exec->words[3],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x1029feu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ff: /* move    x:(exec->addresses[3])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1029ffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a00: /* move    exec->words[3],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a01: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a02: /* move    exec->words[3],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a02u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a03(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a03: /* move    x:(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a04: /* move    exec->words[3],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a05: /* move    x:-(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a06: /* move    exec->words[3],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a06u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a07: /* move    #$46,exec->addresses[0] */
    exec->addresses[0] = (0x46u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a08: /* move    #$46,exec->addresses[3] */
    exec->addresses[3] = (0x46u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a0b(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a0b: /* move    x:(exec->addresses[0])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a0c: /* move    x:(exec->addresses[0])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102a0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a0d: /* asr     #$3,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 3u; exec->lows[0] = ((exec->lows[2] >> 3u) | ((uint32_t)exec->accum[2] << 21u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a0e: /* asr     #$3,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 3u; exec->lows[1] = ((exec->lows[2] >> 3u) | ((uint32_t)exec->accum[2] << 21u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a0f: /* move    a,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x102a0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a10: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x102a10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a11(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a11: /* move    y:(exec->addresses[6]+$9),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 9u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a12: /* move    y:(exec->addresses[6]+$f),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a14: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a15: /* move    #$3,exec->addresses[12] */
    exec->addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a16: /* move    x:(exec->addresses[2]+$140000),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a18: /* move    x:(exec->addresses[2]+$140080),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a1a: /* add     exec->words[0],a            a1,b1 */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102a1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a1b: /* move    #$44,exec->addresses[0] */
    exec->addresses[0] = (0x44u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a1c(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a1c: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a1d: /* move    #$2,exec->addresses[3] */
    exec->addresses[3] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a1e: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a1f: /* nop */

    MD_TRANSLATION_TRACE(0x102a1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a20: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a22: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a24: /* move    #$70,exec->addresses[1] */
    exec->addresses[1] = (0x70u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a25: /* move    #$1,exec->addresses[9] */
    exec->addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a26(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a26: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a27: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a28: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a29: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a2a: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a2b: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x102a2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a2c: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a2d: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a2e(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a2e: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a2f: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a30: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a31: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a32: /* move    y:(exec->addresses[6]+$1e),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 30u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a32u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a33: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a34: /* move    y:(exec->addresses[6]+$1d),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 29u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a34u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a35: /* move    exec->words[1],x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a36(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a36: /* move    y:(exec->addresses[6]+$20),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 32u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a37: /* move    exec->words[1],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a37u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a38: /* move    y:(exec->addresses[6]+$1f),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 31u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a39: /* move    exec->words[1],x:(exec->addresses[3]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a3a: /* move    y:(exec->addresses[6]+$22),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 34u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a3b: /* move    exec->words[1],x:(exec->addresses[0]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0], sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a3c: /* move    y:(exec->addresses[6]+$21),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 33u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a3cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a3d: /* move    exec->words[1],x:(exec->addresses[0]+$1) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[0] + 1u, sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x102a3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a3e(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a3e: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a3f: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[1]),exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a40: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a43(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a43: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a43u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a44: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[0])-,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a44u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a45: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[3])-,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a45u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a46: /* asl     a               x:(exec->addresses[3])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a46u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a47: /* mpy     exec->words[3],exec->words[1],b         x:(exec->addresses[3])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a47u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a48: /* mac     exec->words[0],exec->words[3],b         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a48u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a49: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a4a: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[1]),exec->words[1]       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a4b(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a4b: /* mac     exec->words[1],exec->words[2],b         a,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a4c: /* asl     b               x:(exec->addresses[1])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a4d: /* mpy     exec->words[3],exec->words[1],a         x:(exec->addresses[0])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a4e: /* mac     exec->words[2],exec->words[0],a         b,x:(exec->addresses[3])        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[3], sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a4eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a4f(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a4f: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a4fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a50: /* move    exec->words[3],y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a51: /* move    x:(exec->addresses[1])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a51u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a52: /* move    exec->words[3],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a53: /* move    x:(exec->addresses[3])-,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a54: /* move    exec->words[3],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a55: /* move    x:(exec->addresses[3]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a56: /* move    exec->words[3],y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a56u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a57(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a57: /* move    x:(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a58: /* move    exec->words[3],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a59: /* move    x:-(exec->addresses[0]),exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a5a: /* move    exec->words[3],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x102a5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a5b: /* move    y:(exec->addresses[6]+$b),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a5c: /* move    y:(exec->addresses[6]+$11),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 17u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a5cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a5d: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a5e: /* move    #$20,exec->addresses[0] */
    exec->addresses[0] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a5eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a5f(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a5f: /* move    #$2,exec->addresses[3] */
    exec->addresses[3] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a60: /* move    a,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102a60u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a61: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a64(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a64: /* move    y:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a65: /* move    x:(exec->addresses[3]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a66: /* mac     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a66u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a67: /* nop */

    MD_TRANSLATION_TRACE(0x102a67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a68: /* move    a,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x102a68u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a69(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a69: /* move    #>$2,exec->addresses[5] */
    exec->addresses[5] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a6b: /* move    y:(exec->addresses[6]+$1),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 1u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a6c: /* tst     a */
    exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a6cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a6f(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a6f: /* sub     #>$1,a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x1u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a6fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a71: /* nop */

    MD_TRANSLATION_TRACE(0x102a71u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a72: /* move    a,y:(exec->addresses[6]+$1) */
    *(v->y + 1u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102a72u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a73: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a74: /* move    y:(exec->addresses[6]+$e),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 14u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a74u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a77(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a77: /* add     exec->words[0],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a78: /* move    x:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a78u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a79: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a79u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a7a: /* mpy     exec->words[3],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a7b: /* asr     #$2,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 2u; exec->lows[1] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a7bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a7c: /* nop */

    MD_TRANSLATION_TRACE(0x102a7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a7d: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x102a7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a7e(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a7e: /* move    a,y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102a7eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a80(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a80: /* move    y:(exec->addresses[6]+$3),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 3u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a81: /* tst     a */
    exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a84(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a84: /* sub     #>$1,a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x1u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a86: /* nop */

    MD_TRANSLATION_TRACE(0x102a86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a87: /* move    a,y:(exec->addresses[6]+$3) */
    *(v->y + 3u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102a87u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a8a(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a8a: /* move    x:(exec->addresses[5])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102a8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a8b: /* asr     #$2,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 2u; exec->lows[1] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a8bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a8c: /* nop */

    MD_TRANSLATION_TRACE(0x102a8cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a8d: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x102a8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a8f(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a8f: /* move    y:(exec->addresses[6]+$4),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 4u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a90: /* tst     a */
    exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a93(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a93: /* sub     #>$1,a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x1u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a95: /* nop */

    MD_TRANSLATION_TRACE(0x102a95u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a96: /* move    a,y:(exec->addresses[6]+$4) */
    *(v->y + 4u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102a96u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a97: /* move    y:(exec->addresses[6]+$5),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a97u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a98: /* move    y:(exec->addresses[6]+$e),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 14u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102a98u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102a9b(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102a9b: /* sub     exec->words[0],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a9c: /* move    x:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a9cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a9d: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102a9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a9e: /* mpy     exec->words[3],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a9eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a9f: /* asr     #$2,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 2u; exec->lows[1] = ((exec->lows[2] >> 2u) | ((uint32_t)exec->accum[2] << 22u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102a9fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102aa0: /* nop */

    MD_TRANSLATION_TRACE(0x102aa0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102aa1: /* move    b,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x102aa1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102aa2(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102aa2: /* move    a,y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x102aa2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102aa4(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102aa4: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x102aa4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void trxma_render_block_44k1_group_102aa7(Trxma_Render_Block_44K1Exec *exec, TrxmaVoice *v, const int32_t *sine) {
L_102aa7: /* move    a,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x102aa7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void trxma_render_block_44k1(TrxmaVoice *v, const int32_t *sine) {
    Trxma_Render_Block_44K1Exec execution;
    Trxma_Render_Block_44K1Exec *exec;
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
    uint32_t loop_102925 = 0;
    uint32_t loop_102964 = 0;
    uint32_t loop_102980 = 0;
    uint32_t loop_102996 = 0;
    uint32_t loop_1029ed = 0;
    uint32_t loop_102a09 = 0;
    uint32_t loop_102a41 = 0;
    uint32_t loop_102a62 = 0;
    uint32_t loop_102a75 = 0;
    uint32_t loop_102a88 = 0;
    uint32_t loop_102a99 = 0;
    uint32_t loop_102aa5 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_10291f:
    trxma_render_block_44k1_group_10291f(exec,v,sine);
L_102925: /* do      #<$1f,>$10292c */
    loop_102925 = 0x1fu; if (loop_102925 == 0u) goto L_10292c;
    MD_TRANSLATION_TRACE(0x102925u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102927:
    trxma_render_block_44k1_group_102927(exec,v,sine);
    if (--loop_102925 != 0u) goto L_102927;
L_10292c:
    trxma_render_block_44k1_group_10292c(exec,v,sine);
L_102937:
    trxma_render_block_44k1_group_102937(exec,v,sine);
L_102941:
    trxma_render_block_44k1_group_102941(exec,v,sine);
L_10294b:
    trxma_render_block_44k1_group_10294b(exec,v,sine);
L_102953:
    trxma_render_block_44k1_group_102953(exec,v,sine);
L_10295b:
    trxma_render_block_44k1_group_10295b(exec,v,sine);
L_102963:
    trxma_render_block_44k1_group_102963(exec,v,sine);
L_102964: /* do      #<$20,>$102972 */
    loop_102964 = 0x20u; if (loop_102964 == 0u) goto L_102972;
    MD_TRANSLATION_TRACE(0x102964u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102966:
    trxma_render_block_44k1_group_102966(exec,v,sine);
L_10296e:
    trxma_render_block_44k1_group_10296e(exec,v,sine);
    if (--loop_102964 != 0u) goto L_102966;
L_102972:
    trxma_render_block_44k1_group_102972(exec,v,sine);
L_10297a:
    trxma_render_block_44k1_group_10297a(exec,v,sine);
L_102980: /* do      #<$10,>$102986 */
    loop_102980 = 0x10u; if (loop_102980 == 0u) goto L_102986;
    MD_TRANSLATION_TRACE(0x102980u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102982:
    trxma_render_block_44k1_group_102982(exec,v,sine);
    if (--loop_102980 != 0u) goto L_102982;
L_102986:
    trxma_render_block_44k1_group_102986(exec,v,sine);
L_102990:
    trxma_render_block_44k1_group_102990(exec,v,sine);
L_102996: /* do      #<$20,>$1029ba */
    loop_102996 = 0x20u; if (loop_102996 == 0u) goto L_1029ba;
    MD_TRANSLATION_TRACE(0x102996u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102998:
    trxma_render_block_44k1_group_102998(exec,v,sine);
L_10299a: /* blt     func_1029a5 */
    if (exec->flags[1]) goto L_1029a5;
    MD_TRANSLATION_TRACE(0x10299au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10299c:
    trxma_render_block_44k1_group_10299c(exec,v,sine);
L_1029a3: /* bra     func_1029a8 */
    goto L_1029a8;
    MD_TRANSLATION_TRACE(0x1029a3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029a5:
    trxma_render_block_44k1_group_1029a5(exec,v,sine);
L_1029a6: /* beq     func_1029b6 */
    if (exec->flags[0]) goto L_1029b6;
    MD_TRANSLATION_TRACE(0x1029a6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029a8:
    trxma_render_block_44k1_group_1029a8(exec,v,sine);
L_1029ad: /* bgt     func_1029b9 */
    if (exec->flags[2]) goto L_1029b9;
    MD_TRANSLATION_TRACE(0x1029adu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029af:
    trxma_render_block_44k1_group_1029af(exec,v,sine);
L_1029b4: /* bra     func_1029b9 */
    goto L_1029b9;
    MD_TRANSLATION_TRACE(0x1029b4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029b6:
    trxma_render_block_44k1_group_1029b6(exec,v,sine);
L_1029b9:
    trxma_render_block_44k1_group_1029b9(exec,v,sine);
    if (--loop_102996 != 0u) goto L_102998;
L_1029ba:
    trxma_render_block_44k1_group_1029ba(exec,v,sine);
L_1029c4:
    trxma_render_block_44k1_group_1029c4(exec,v,sine);
L_1029ce:
    trxma_render_block_44k1_group_1029ce(exec,v,sine);
L_1029d7:
    trxma_render_block_44k1_group_1029d7(exec,v,sine);
L_1029df:
    trxma_render_block_44k1_group_1029df(exec,v,sine);
L_1029e7:
    trxma_render_block_44k1_group_1029e7(exec,v,sine);
L_1029ed: /* do      #<$20,>$1029fb */
    loop_1029ed = 0x20u; if (loop_1029ed == 0u) goto L_1029fb;
    MD_TRANSLATION_TRACE(0x1029edu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1029ef:
    trxma_render_block_44k1_group_1029ef(exec,v,sine);
L_1029f7:
    trxma_render_block_44k1_group_1029f7(exec,v,sine);
    if (--loop_1029ed != 0u) goto L_1029ef;
L_1029fb:
    trxma_render_block_44k1_group_1029fb(exec,v,sine);
L_102a03:
    trxma_render_block_44k1_group_102a03(exec,v,sine);
L_102a09: /* do      #<$10,>$102a11 */
    loop_102a09 = 0x10u; if (loop_102a09 == 0u) goto L_102a11;
    MD_TRANSLATION_TRACE(0x102a09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a0b:
    trxma_render_block_44k1_group_102a0b(exec,v,sine);
    if (--loop_102a09 != 0u) goto L_102a0b;
L_102a11:
    trxma_render_block_44k1_group_102a11(exec,v,sine);
L_102a1c:
    trxma_render_block_44k1_group_102a1c(exec,v,sine);
L_102a26:
    trxma_render_block_44k1_group_102a26(exec,v,sine);
L_102a2e:
    trxma_render_block_44k1_group_102a2e(exec,v,sine);
L_102a36:
    trxma_render_block_44k1_group_102a36(exec,v,sine);
L_102a3e:
    trxma_render_block_44k1_group_102a3e(exec,v,sine);
L_102a41: /* do      #<$20,>$102a4f */
    loop_102a41 = 0x20u; if (loop_102a41 == 0u) goto L_102a4f;
    MD_TRANSLATION_TRACE(0x102a41u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a43:
    trxma_render_block_44k1_group_102a43(exec,v,sine);
L_102a4b:
    trxma_render_block_44k1_group_102a4b(exec,v,sine);
    if (--loop_102a41 != 0u) goto L_102a43;
L_102a4f:
    trxma_render_block_44k1_group_102a4f(exec,v,sine);
L_102a57:
    trxma_render_block_44k1_group_102a57(exec,v,sine);
L_102a5f:
    trxma_render_block_44k1_group_102a5f(exec,v,sine);
L_102a62: /* do      #<$20,>$102a69 */
    loop_102a62 = 0x20u; if (loop_102a62 == 0u) goto L_102a69;
    MD_TRANSLATION_TRACE(0x102a62u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a64:
    trxma_render_block_44k1_group_102a64(exec,v,sine);
    if (--loop_102a62 != 0u) goto L_102a64;
L_102a69:
    trxma_render_block_44k1_group_102a69(exec,v,sine);
L_102a6d: /* beq     func_102a80 */
    if (exec->flags[0]) goto L_102a80;
    MD_TRANSLATION_TRACE(0x102a6du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a6f:
    trxma_render_block_44k1_group_102a6f(exec,v,sine);
L_102a75: /* do      #<$20,>$102a7e */
    loop_102a75 = 0x20u; if (loop_102a75 == 0u) goto L_102a7e;
    MD_TRANSLATION_TRACE(0x102a75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a77:
    trxma_render_block_44k1_group_102a77(exec,v,sine);
    if (--loop_102a75 != 0u) goto L_102a77;
L_102a7e:
    trxma_render_block_44k1_group_102a7e(exec,v,sine);
L_102a7f: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_102a80:
    trxma_render_block_44k1_group_102a80(exec,v,sine);
L_102a82: /* beq     func_102a8f */
    if (exec->flags[0]) goto L_102a8f;
    MD_TRANSLATION_TRACE(0x102a82u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a84:
    trxma_render_block_44k1_group_102a84(exec,v,sine);
L_102a88: /* do      #<$20,>$102a8e */
    loop_102a88 = 0x20u; if (loop_102a88 == 0u) goto L_102a8e;
    MD_TRANSLATION_TRACE(0x102a88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a8a:
    trxma_render_block_44k1_group_102a8a(exec,v,sine);
    if (--loop_102a88 != 0u) goto L_102a8a;
L_102a8e: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_102a8f:
    trxma_render_block_44k1_group_102a8f(exec,v,sine);
L_102a91: /* beq     func_102aa4 */
    if (exec->flags[0]) goto L_102aa4;
    MD_TRANSLATION_TRACE(0x102a91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a93:
    trxma_render_block_44k1_group_102a93(exec,v,sine);
L_102a99: /* do      #<$20,>$102aa2 */
    loop_102a99 = 0x20u; if (loop_102a99 == 0u) goto L_102aa2;
    MD_TRANSLATION_TRACE(0x102a99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102a9b:
    trxma_render_block_44k1_group_102a9b(exec,v,sine);
    if (--loop_102a99 != 0u) goto L_102a9b;
L_102aa2:
    trxma_render_block_44k1_group_102aa2(exec,v,sine);
L_102aa3: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_102aa4:
    trxma_render_block_44k1_group_102aa4(exec,v,sine);
L_102aa5: /* do      #<$20,>$102aa8 */
    loop_102aa5 = 0x20u; if (loop_102aa5 == 0u) goto L_102aa8;
    MD_TRANSLATION_TRACE(0x102aa5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_102aa7:
    trxma_render_block_44k1_group_102aa7(exec,v,sine);
    if (--loop_102aa5 != 0u) goto L_102aa7;
L_102aa8: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t trxma_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201bc0: /* lea.l -$10(a7), a7 */
    /* local register save area */
C_201bc4: /* movem.l d[2]/a2-a4, (a7) */
    /* native locals replace original callee saves */
C_201bc8: /* movea.l $14(a7), a3 */
    /* resolved argument pointer */
C_201bcc: /* movea.l $18(a7), a4 */
    /* resolved argument pointer */
C_201bd0: /* tst.l (a3) */
    lhs = (uint32_t)*(y + (0u) / 4u); rhs = 0u;
C_201bd2: /* bne.b $201bda */
    if (lhs != rhs) goto C_201bda;
C_201bd4: /* clr.l d[0] */
    d[0] = 0u;
C_201bd6: /* bra.w $201ca6 */
    goto C_201ca6;
C_201bda: /* lea.l $4(a3), a2 */
    a2 = 4u;
C_201bde: /* clr.l d[0] */
    d[0] = 0u;
C_201be0: /* move.w (a4), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_201be2: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201be4: /* lsr.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] >> ((0x6u) & 63u) : 0u);
C_201be6: /* addi.l #$a, d[1] */
    lhs = d[1]; rhs = 0xau; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201bec: /* move.l d[1], (a2) */
    *(y + ((0u) + a2) / 4u) = (int32_t)(d[1]);
C_201bee: /* lsr.l #$1, d[0] */
    d[0] = (((0x1u) & 63u) < 32u ? d[0] >> ((0x1u) & 63u) : 0u);
C_201bf0: /* addi.l #$2a, d[0] */
    lhs = d[0]; rhs = 0x2au; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201bf6: /* move.l #$800000, d[1] */
    d[1] = 0x800000u;
C_201bfc: /* divs.l d[0], d[1] */
    d[1] = md_divs32(d[1], d[0]);
C_201c00: /* move.l d[1], $8(a3) */
    *(y + (8u) / 4u) = (int32_t)(d[1]);
C_201c04: /* lea.l $c(a3), a1 */
    a1 = 12u;
C_201c08: /* clr.l d[0] */
    d[0] = 0u;
C_201c0a: /* move.w $2(a4), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_201c0e: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_201c10: /* addq.l #$1, d[0] */
    d[0] = d[0] + 0x1u;
C_201c12: /* move.l d[0], (a1) */
    *(y + ((0u) + a1) / 4u) = (int32_t)(d[0]);
C_201c14: /* lea.l $10(a3), a0 */
    a0 = 16u;
C_201c18: /* clr.l d[0] */
    d[0] = 0u;
C_201c1a: /* move.w $4(a4), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_201c1e: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201c20: /* lsr.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] >> ((0x6u) & 63u) : 0u);
C_201c22: /* addi.l #$a, d[1] */
    lhs = d[1]; rhs = 0xau; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201c28: /* move.l d[1], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[1]);
C_201c2a: /* lsr.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] >> ((0x3u) & 63u) : 0u);
C_201c2c: /* addi.l #$10, d[0] */
    lhs = d[0]; rhs = 0x10u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201c32: /* move.l #$800000, d[1] */
    d[1] = 0x800000u;
C_201c38: /* divs.l d[0], d[1] */
    d[1] = md_divs32(d[1], d[0]);
C_201c3c: /* move.l d[1], $14(a3) */
    *(y + (20u) / 4u) = (int32_t)(d[1]);
C_201c40: /* clr.l d[0] */
    d[0] = 0u;
C_201c42: /* move.w $6(a4), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_201c46: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_201c48: /* move.l #$783a3c, d[1] */
    d[1] = 0x783a3cu;
C_201c4e: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_201c50: /* move.l d[1], $18(a3) */
    *(y + (24u) / 4u) = (int32_t)(d[1]);
C_201c54: /* clr.l d[1] */
    d[1] = 0u;
C_201c56: /* move.w $8(a4), d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_201c5a: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_201c5c: /* move.l (a2), d[0] */
    d[0] = (uint32_t)*(y + ((0u) + a2) / 4u);
C_201c5e: /* add.l (a1), d[0] */
    lhs = d[0]; rhs = (uint32_t)*(y + ((0u) + a1) / 4u); cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201c60: /* add.l (a0), d[0] */
    lhs = d[0]; rhs = (uint32_t)*(y + ((0u) + a0) / 4u); cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201c62: /* divs.l d[0], d[1] */
    d[1] = md_divs32(d[1], d[0]);
C_201c66: /* move.l d[1], $2c(a3) */
    *(y + (44u) / 4u) = (int32_t)(d[1]);
C_201c6a: /* clr.l d[0] */
    d[0] = 0u;
C_201c6c: /* move.w $a(a4), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_201c70: /* lsr.l #$4, d[0] */
    d[0] = (((0x4u) & 63u) < 32u ? d[0] >> ((0x4u) & 63u) : 0u);
C_201c72: /* addi.l #$400, d[0] */
    lhs = d[0]; rhs = 0x400u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201c78: /* move.l d[0], $28(a3) */
    *(y + (40u) / 4u) = (int32_t)(d[0]);
C_201c7c: /* clr.l d[0] */
    d[0] = 0u;
C_201c7e: /* move.w $c(a4), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_201c82: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_201c84: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201c86: /* addi.l #$600, d[1] */
    lhs = d[1]; rhs = 0x600u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201c8c: /* move.l d[1], $1c(a3) */
    *(y + (28u) / 4u) = (int32_t)(d[1]);
C_201c90: /* addi.l #$5e0, d[0] */
    lhs = d[0]; rhs = 0x5e0u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201c96: /* move.l d[0], $24(a3) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_201c9a: /* clr.l d[0] */
    d[0] = 0u;
C_201c9c: /* move.b $e(a4), d[0] */
    d[0] = (d[0] & 0xffffff00u) | ((((*(p + (14u) / 2u) >> (((14u) & 1u) ? 0u : 8u)) & 0xffu)) & 0xffu);
C_201ca0: /* move.l d[0], $20(a3) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_201ca4: /* moveq #$c, d[0] */
    d[0] = 0x0000000cu;
C_201ca6: /* movem.l (a7), d[2]/a2-a4 */
    /* native locals replace original callee saves */
C_201caa: /* lea.l $10(a7), a7 */
    /* local register save area */
C_201cae: /* rts  */
    return d[0];
}

void trxma_update_coeffs(const uint32_t *p, TrxmaVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxma_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxma_dsp_init(v, 0); }
}
void trxma_voice_init(TrxmaVoice *v) {
    uint32_t i;
    for (i = 0u; i < 41u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxma_voice_trigger(TrxmaVoice *v) {
    trxma_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxma_resample_run(TrxmaVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxmaVoice
#define MD_RENDER48 trxma_render_block_48k_float
#define MD_RENDER44 trxma_render_block_44k1
#define MD_RESAMPLE_RUN trxma_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxmaVoice
#define MD_SLOT_INIT trxma_slot_init
#define MD_SLOT_PROCESS trxma_slot_process
#define MD_VOICE_INIT trxma_voice_init
#define MD_COEFFICIENTS trxma_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxma_update_coeffs
#define MD_VOICE_TRIGGER trxma_voice_trigger
#define MD_RENDER44 trxma_render_block_44k1
#define MD_RENDER48 trxma_render_block_48k_float
#include "../engine_slot.inc"
#endif
