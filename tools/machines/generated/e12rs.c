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
    int32_t y[37];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} E12RsVoice;
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

static uint32_t md_read_source(E12RsVoice *v, uint32_t address) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) { value = md_source_words[address - 0x20000u]; }
    else if (address >= 0x200000u && address < 0xffff80u) { value = 0u; }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
extern const uint32_t source_103d7b[63];
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

static uint32_t md_read_x(E12RsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1064315u && address < 1064378u) { value = *(source_103d7b + (address - 1064315u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(E12RsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(E12RsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2085u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1064315u && address < 1064378u) { value = *(source_103d7b + (address - 1064315u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(E12RsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2085u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(E12RsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(E12RsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(E12RsVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef E12RS_DSP_INIT_EXEC_DEFINED
#define E12RS_DSP_INIT_EXEC_DEFINED
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
} E12Rs_Dsp_InitExec;
#endif
static void e12rs_dsp_init_group_103718(E12Rs_Dsp_InitExec *exec, E12RsVoice *v, const int32_t *sine) {
L_103718: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103718u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103719: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x103719u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10371b: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10371bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10371d: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10371du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10371f: /* move    exec->words[0],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x10371fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12rs_dsp_init(E12RsVoice *v, const int32_t *sine) {
    E12Rs_Dsp_InitExec execution;
    E12Rs_Dsp_InitExec *exec;
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
L_103718:
    e12rs_dsp_init_group_103718(exec,v,sine);
L_103721: /* rts */
    return;}

#ifndef E12RS_DSP_TRIGGER_EXEC_DEFINED
#define E12RS_DSP_TRIGGER_EXEC_DEFINED
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
} E12Rs_Dsp_TriggerExec;
#endif
static void e12rs_dsp_trigger_group_10379c(E12Rs_Dsp_TriggerExec *exec, E12RsVoice *v, const int32_t *sine) {
L_10379c: /* move    #>$103d9c,exec->addresses[1] */
    exec->addresses[1] = (0x103d9cu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10379cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_10379e: /* move    #>$103d9f,exec->addresses[0] */
    exec->addresses[0] = (0x103d9fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10379eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a0: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037a0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a1: /* move    exec->words[1],y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1037a1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a2: /* move    exec->words[1],y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1037a2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a3: /* move    exec->words[1],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1037a3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a4: /* move    #>$7fffff,exec->words[1] */
    exec->words[1] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037a4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a6: /* move    exec->words[1],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x1037a6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_dsp_trigger_group_1037a7(E12Rs_Dsp_TriggerExec *exec, E12RsVoice *v, const int32_t *sine) {
L_1037a7: /* move    y:(exec->addresses[1])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037a7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a8: /* move    exec->words[0],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1037a8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037a9: /* move    y:(exec->addresses[0])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037a9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037aa: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1037aau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037ab: /* move    y:(exec->addresses[1])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037abu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037ac: /* move    exec->words[2],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x1037acu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037ad: /* move    y:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037adu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037ae: /* move    exec->words[2],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x1037aeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_dsp_trigger_group_1037af(E12Rs_Dsp_TriggerExec *exec, E12RsVoice *v, const int32_t *sine) {
L_1037af: /* move    y:(exec->addresses[6]+$6),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037afu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037b0: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x1037b0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037b1: /* move    y:(exec->addresses[6]+$9),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1037b1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037b2: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x1037b2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_1037b3: /* move    a,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x1037b3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12rs_dsp_trigger(E12RsVoice *v, const int32_t *sine) {
    E12Rs_Dsp_TriggerExec execution;
    E12Rs_Dsp_TriggerExec *exec;
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
L_10379c:
    e12rs_dsp_trigger_group_10379c(exec,v,sine);
L_1037a7:
    e12rs_dsp_trigger_group_1037a7(exec,v,sine);
L_1037af:
    e12rs_dsp_trigger_group_1037af(exec,v,sine);
L_1037b4: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

#ifndef E12RS_RENDER_BLOCK_44K1_EXEC_DEFINED
#define E12RS_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} E12Rs_Render_Block_44K1Exec;
#endif
static void e12rs_render_block_44k1_group_103a10(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a10: /* move    y:(exec->addresses[6]+$13),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a11: /* move    y:(exec->addresses[6]+$7),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a12: /* mpy     exec->words[2],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a13: /* move    y:(exec->addresses[6]+$8),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a13u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a14: /* move    b,y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x103a14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a15: /* move    y:(exec->addresses[6]+$15),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a16: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a17: /* clr     b */
    exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a18(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a18: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a19: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a19u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a1a: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a1b: /* move    exec->words[0],b0 */
    exec->lows[1] = (exec->words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a1c: /* asr     #$18,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[0] = (uint32_t)(exec->accum[2] >> 0u) & 0xffffffu; exec->accum[0] = exec->accum[2] >> 24u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a1d: /* add     a,b */
    add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a1e: /* asl     #$3,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 3u) | (exec->lows[2] >> 21u)); exec->lows[1] = (exec->lows[2] << 3u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a1f: /* cmp     #>$4,b */
    exec->accum[2] = sx24((int32_t)(0x4u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103a1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a23(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a23: /* move    #>$4,b */
    exec->accum[1] = sx24((int32_t)(0x4u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103a23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a25(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a25: /* tst     b */
    exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a26: /* clr     b               ifmi */
    if (exec->flags[3]) { exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); }
    MD_TRANSLATION_TRACE(0x103a26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a27: /* move    #$a0,exec->addresses[2] */
    exec->addresses[2] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a28: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a29: /* move    #$2,exec->addresses[12] */
    exec->addresses[12] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a2a: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a2b: /* move    y:(exec->addresses[6]+$11),a0 */
    exec->lows[0] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a2c: /* move    b0,exec->words[0] */
    exec->words[0] = (exec->lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a2d(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a2d: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a2e: /* move    #>$147400,exec->words[3] */
    exec->words[3] = (0x147400u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a30: /* move    exec->words[3],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103a30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a31: /* move    #>$200,exec->words[2] */
    exec->words[2] = (0x200u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a33: /* move    exec->words[2],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x103a33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a34: /* move    #>$fffffc,exec->words[3] */
    exec->words[3] = (0xfffffcu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a34u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a36: /* move    exec->words[3],y:(exec->addresses[4]) */
    md_write_y(v, md_shared_scratch_y, exec->addresses[4], sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103a36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a39(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a39: /* move    a0,exec->words[3] */
    exec->words[3] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a3a: /* mpy     exec->words[3],exec->words[2],b         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a3b: /* and     exec->words[2],b            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | (u24(exec->accum[1]) & exec->words[2])); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a3c: /* add     exec->words[2],b            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a3cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a3d: /* add     x,a             a1,x:(exec->addresses[2]) */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103a3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a3e: /* move    b1,y:(exec->addresses[2])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x103a3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a3f(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a3f: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a41: /* move    y:(exec->addresses[6]+$1c),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a41u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a42: /* move    a1,x:>$fe */
    *(md_shared_scratch_x + 254u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103a42u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a44: /* move    y:(exec->addresses[6]+$10),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a44u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a45: /* move    exec->words[3],x:>$ff */
    *(md_shared_scratch_x + 255u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x103a45u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a47: /* add     exec->words[3],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a47u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a48: /* cmp     exec->words[2],a */
    exec->accum[2] = sx24((int32_t)(exec->words[2])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103a48u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a4b(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a4b: /* move    y:(exec->addresses[6]+$13),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 19u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a4c: /* nop */

    MD_TRANSLATION_TRACE(0x103a4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a4d(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a4d: /* move    a0,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x103a4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a4e: /* move    a1,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103a4eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a4f: /* move    y:(exec->addresses[6]+$1e),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 30u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a4fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a50: /* add     #>$20,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x20u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a52: /* move    y:(exec->addresses[6]+$a),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a53: /* cmp     exec->words[0],a */
    exec->accum[2] = sx24((int32_t)(exec->words[0])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103a53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a56(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a56: /* move    y:(exec->addresses[6]+$1f),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 31u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103a56u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a57: /* add     #<$1,b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(0x1u)), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a58: /* move    y:(exec->addresses[6]+$e),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a59: /* cmp     exec->words[1],b */
    exec->accum[2] = sx24((int32_t)(exec->words[1])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103a59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a5c(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a5c: /* move    b,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x103a5cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a5d: /* sub     exec->words[0],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a5e: /* and     #<$1f,a */
    exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | (u24(exec->accum[0]) & 0x1fu));
    MD_TRANSLATION_TRACE(0x103a5eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a5f: /* move    #>$bf,b */
    exec->accum[1] = sx24((int32_t)(0xbfu)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103a5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a61: /* sub     a,b */
    sub56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a62: /* move    y:(exec->addresses[6]+$13),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a62u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a63: /* move    b,exec->addresses[0] */
    exec->addresses[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a63u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a64: /* move    y:(exec->addresses[6]+$6),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a65(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a65: /* mpy     exec->words[3],exec->words[1],b         a,exec->words[0] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a66: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a66u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a67: /* move    exec->words[1],x:>$c0 */
    *(md_shared_scratch_x + 192u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103a67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a69: /* move    x:(exec->addresses[0]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a6a: /* add     exec->words[1],b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a6au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a6b: /* nop */

    MD_TRANSLATION_TRACE(0x103a6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a6c: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a6cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a6d: /* move    exec->words[1],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103a6du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a6e(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a6e: /* move    exec->words[1],x:>$ff */
    *(md_shared_scratch_x + 255u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103a6eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a72(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a72: /* move    x:(exec->addresses[0]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a72u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a73: /* sub     exec->words[1],a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a74: /* nop */

    MD_TRANSLATION_TRACE(0x103a74u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a75: /* move    a,x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103a75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a76(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a76: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a76u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a77: /* move    y:(exec->addresses[6]+$a),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 10u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103a77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a78: /* cmp     #>$20,b */
    exec->accum[2] = sx24((int32_t)(0x20u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103a78u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a7c(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a7c: /* move    exec->words[0],b */
    exec->accum[1] = sx24((int32_t)(exec->words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103a7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a7d: /* sub     #>$20,b */
    sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(0x20u)), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a7f: /* neg     b */
    exec->accum[1] = (int32_t)(0u - (uint32_t)exec->accum[1] - (uint32_t)(exec->lows[1] != 0u)); exec->lows[1] = (0u - exec->lows[1]) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a7fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a80: /* move    #$a0,exec->addresses[0] */
    exec->addresses[0] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a81: /* move    b,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a82: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a82u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a83: /* move    exec->words[0],a */
    exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a83u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a86(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a86: /* move    exec->words[1],x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x103a86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a87(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a87: /* nop */

    MD_TRANSLATION_TRACE(0x103a87u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a88(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a88: /* move    a,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x103a88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a89(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a89: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a8a: /* move    #$2,exec->addresses[5] */
    exec->addresses[5] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a8b: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a8bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a8d: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a8e: /* move    y:(exec->addresses[6]+$12),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a8f: /* add     exec->words[2],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103a8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a90: /* move    #$3,exec->addresses[20] */
    exec->addresses[20] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a91: /* move    #$3,exec->addresses[21] */
    exec->addresses[21] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a92(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a92: /* move    a1,exec->addresses[3] */
    exec->addresses[3] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a92u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a93: /* move    #$87,exec->addresses[12] */
    exec->addresses[12] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a94: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a94u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a96: /* btst    #$0,a */
    exec->flags[4] = (((u24(lim24(exec->accum[0]))) >> 0u) & 1u);
    MD_TRANSLATION_TRACE(0x103a96u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a99(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a99: /* move    #$87,exec->addresses[4] */
    exec->addresses[4] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a9a: /* move    #$1,exec->addresses[5] */
    exec->addresses[5] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103a9au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103a9d(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103a9d: /* move    x:(exec->addresses[3])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103a9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a9e: /* move    x:(exec->addresses[3])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103a9eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a9f: /* move    a1,x:(exec->addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x103a9fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa0: /* lsl     #$c,a */
    exec->flags[4] = (u24(exec->accum[0]) >> 12u) & 1u; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((u24(exec->accum[0]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[0]) == 0u); exec->flags[3] = ((u24(exec->accum[0]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x103aa0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa1: /* move    b1,x:(exec->addresses[5])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x103aa1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa2: /* lsl     #$c,b */
    exec->flags[4] = (u24(exec->accum[1]) >> 12u) & 1u; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(exec->accum[1]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[1]) == 0u); exec->flags[3] = ((u24(exec->accum[1]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x103aa2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa3: /* move    a1,x:(exec->addresses[4])+exec->addresses[20] */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x103aa3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa4: /* move    b1,x:(exec->addresses[5])+exec->addresses[21] */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[5], exec->addresses[13], sx24((int32_t)exec->addresses[21]), 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x103aa4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103aa5(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103aa5: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aa5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa6: /* move    #$2,exec->addresses[5] */
    exec->addresses[5] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aa6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa7: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103aa7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aa9: /* move    #$0,b */
    exec->accum[1] = sx24((int32_t)(((0x0u) << 16))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103aa9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aaa: /* move    y:(exec->addresses[6]+$23),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 35u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aaau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aab: /* cmp     exec->words[2],a */
    exec->accum[2] = sx24((int32_t)(exec->words[2])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x103aabu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103aae(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103aae: /* asr     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103aaeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aaf: /* move    y:(exec->addresses[6]+$24),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aafu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab0: /* add     exec->words[2],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103ab0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab1: /* move    #$3,exec->addresses[20] */
    exec->addresses[20] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ab1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab2: /* move    #$3,exec->addresses[21] */
    exec->addresses[21] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ab2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab3: /* move    a1,exec->addresses[3] */
    exec->addresses[3] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ab3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab4: /* move    x:>$ff,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 255u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103ab4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab6: /* btst    #$0,a */
    exec->flags[4] = (((u24(lim24(exec->accum[0]))) >> 0u) & 1u);
    MD_TRANSLATION_TRACE(0x103ab6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ab9(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ab9: /* move    #$87,exec->addresses[4] */
    exec->addresses[4] = (0x87u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ab9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aba: /* move    #$1,exec->addresses[5] */
    exec->addresses[5] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103abau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103abd(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103abd: /* move    x:(exec->addresses[3])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103abdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103abe: /* move    x:(exec->addresses[3])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103abeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103abf: /* move    a1,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x103abfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac0: /* lsl     #$c,a */
    exec->flags[4] = (u24(exec->accum[0]) >> 12u) & 1u; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((u24(exec->accum[0]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[0]) == 0u); exec->flags[3] = ((u24(exec->accum[0]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x103ac0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac1: /* move    b1,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x103ac1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac2: /* lsl     #$c,b */
    exec->flags[4] = (u24(exec->accum[1]) >> 12u) & 1u; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(exec->accum[1]) << 12u) & 0xffffffu)); exec->flags[0] = (u24(exec->accum[1]) == 0u); exec->flags[3] = ((u24(exec->accum[1]) >> 23) != 0u);
    MD_TRANSLATION_TRACE(0x103ac2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac3: /* move    a1,y:(exec->addresses[4])+exec->addresses[20] */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x103ac3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac4: /* move    b1,y:(exec->addresses[5])+exec->addresses[21] */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], sx24((int32_t)exec->addresses[21]), 0u), sine, u24(exec->accum[1]));
    MD_TRANSLATION_TRACE(0x103ac4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ac5(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ac5: /* move    #>$7fffff,b */
    exec->accum[1] = sx24((int32_t)(0x7fffffu)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103ac5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ac7(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ac7: /* move    y:(exec->addresses[6]+$b),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ac7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac8: /* move    #$0,exec->addresses[0] */
    exec->addresses[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ac8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ac9: /* move    exec->addresses[0],exec->addresses[2] */
    exec->addresses[2] = (exec->addresses[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ac9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aca: /* move    b,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103acau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103acb: /* move    exec->addresses[0],exec->addresses[4] */
    exec->addresses[4] = (exec->addresses[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103acbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103acc: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103accu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ace: /* move    y:(exec->addresses[6]+$b),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103acf: /* move    x:(exec->addresses[0])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103acfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ad0(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ad0: /* mpy     exec->words[1],exec->words[0],b         y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ad0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ad1: /* mac     exec->words[3],exec->words[2],b         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ad1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ad4(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ad4: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ad4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ad5: /* mac     exec->words[3],exec->words[2],a         b,x:(exec->addresses[2])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103ad5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ad6: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ad6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ad7: /* mac     exec->words[3],exec->words[2],b         a,x:(exec->addresses[2])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103ad7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ad8(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ad8: /* move    #>$ffffff,exec->addresses[13] */
    exec->addresses[13] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ad8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ada: /* move    y:(exec->addresses[6]+$3),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103adau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103adb: /* move    y:(exec->addresses[6]+$15),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103adbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103adc: /* move    exec->words[3],a */
    exec->accum[0] = sx24((int32_t)(exec->words[3])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103adcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103add: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103addu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ae0(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ae0: /* mpy     exec->words[3],exec->words[1],a         a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103ae0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae1: /* nop */

    MD_TRANSLATION_TRACE(0x103ae1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae2: /* move    a,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103ae3(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103ae3: /* move    #$a0,exec->addresses[2] */
    exec->addresses[2] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae4: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae5: /* move    #$0,exec->addresses[3] */
    exec->addresses[3] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae6: /* move    exec->words[3],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x103ae6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae7: /* move    y:(exec->addresses[2])+,exec->addresses[4] */
    exec->addresses[4] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae8: /* move    y:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae9: /* move    #$c8,exec->addresses[0] */
    exec->addresses[0] = (0xc8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103ae9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103aec(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103aec: /* move    x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aed: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aedu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aee: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aeeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aef: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[3])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103aefu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af0: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[2]),exec->addresses[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103af0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af1: /* move    y:(exec->addresses[2])+,exec->addresses[4] */
    exec->addresses[4] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103af1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af2: /* move    b,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103af2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af3: /* mpy     exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103af3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103af4(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103af4: /* move    y:(exec->addresses[5])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103af4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af5: /* move    b,x:(exec->addresses[0])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103af5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103af6(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103af6: /* move    y:(exec->addresses[6]+$4),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103af6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af8: /* move    y:(exec->addresses[6]+$5),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 5u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103af8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103af9: /* nop */

    MD_TRANSLATION_TRACE(0x103af9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103afa: /* nop */

    MD_TRANSLATION_TRACE(0x103afau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103afb: /* move    x:(exec->addresses[2]+$103c7b),exec->words[0] */
    exec->words[0] = (md_read_source(v, exec->addresses[2] + 1064059u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103afbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103afd: /* move    x:(exec->addresses[2]+$103cfb),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[2] + 1064187u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103afdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aff: /* add     exec->words[0],a            a1,b */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103affu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b00: /* nop */

    MD_TRANSLATION_TRACE(0x103b00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b01(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b01: /* add     exec->words[1],b            a1,exec->addresses[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b02: /* nop */

    MD_TRANSLATION_TRACE(0x103b02u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b03: /* move    b1,exec->addresses[2] */
    exec->addresses[2] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b04: /* nop */

    MD_TRANSLATION_TRACE(0x103b04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b05: /* move    x:(exec->addresses[1]+$140900),exec->words[1] */
    exec->words[1] = (md_read_source(v, exec->addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b07: /* move    x:(exec->addresses[2]+$141900),exec->words[3] */
    exec->words[3] = (md_read_source(v, exec->addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b09: /* move    #$78,exec->addresses[4] */
    exec->addresses[4] = (0x78u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b0a: /* move    #$4,exec->addresses[12] */
    exec->addresses[12] = (0x4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b0au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b0b(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b0b: /* mpy     exec->words[3],exec->words[1],a         exec->words[3],exec->words[0] */
    exec->parallel_words[0] = exec->words[3]; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b0c: /* mpy     -exec->words[0],exec->words[3],b */
    mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103b0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b0d: /* move    #$c4,exec->addresses[2] */
    exec->addresses[2] = (0xc4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b0e: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103b0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b0f: /* asl     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103b0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b10: /* add     #>$c00000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0xc00000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103b10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b12: /* move    b,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x103b12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b13: /* move    a,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103b13u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b14(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b14: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b15: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x103b15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b16: /* move    #$80,exec->words[1] */
    exec->words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b17: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x103b17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b18: /* move    #$40,exec->words[1] */
    exec->words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b19: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x103b19u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b1a: /* move    #$c4,exec->addresses[0] */
    exec->addresses[0] = (0xc4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b1b: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b1d(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b1d: /* move    #>$fffffd,exec->addresses[16] */
    exec->addresses[16] = (0xfffffdu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b1f: /* move    y:(exec->addresses[6]+$17),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b20: /* move    exec->words[3],x:(exec->addresses[2])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103b20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b21: /* move    y:(exec->addresses[6]+$18),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 24u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103b21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b22: /* move    y:(exec->addresses[6]+$1b),a0 */
    exec->lows[0] = (u24(*(v->y + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b23: /* move    y:(exec->addresses[6]+$1a),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 26u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b24: /* move    a,x:(exec->addresses[2])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103b24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b25: /* move    exec->words[3],x:(exec->addresses[2])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103b25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b26(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b26: /* move    y:(exec->addresses[6]+$19),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b27: /* move    exec->words[3],x:(exec->addresses[2]) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[2], sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x103b27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b28: /* move    #$c6,exec->addresses[1] */
    exec->addresses[1] = (0xc6u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b29: /* move    x:>$e6,exec->words[3] */
    exec->words[3] = (u24(*(md_shared_scratch_x + 230u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b2b: /* move    exec->words[3],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x103b2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b2c: /* move    x:>$e7,exec->words[3] */
    exec->words[3] = (u24(*(md_shared_scratch_x + 231u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b2e: /* move    exec->words[3],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x103b2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b2f: /* move    x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b32(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b32: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b32u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b33: /* mac     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x103b33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b34: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b34u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b35: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b36: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+exec->addresses[16],exec->words[0]    y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16]), 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b37: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b37u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b38: /* nop */

    MD_TRANSLATION_TRACE(0x103b38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b39: /* move    a,l:(exec->addresses[1])+ */
    exec->long_address[0] = md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103b39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b3a(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b3a: /* move    a,y:(exec->addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x103b3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void e12rs_render_block_44k1_group_103b3b(E12Rs_Render_Block_44K1Exec *exec, E12RsVoice *v, const int32_t *sine) {
L_103b3b: /* move    x:>$e4,exec->words[1] */
    exec->words[1] = (u24(*(md_shared_scratch_x + 228u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b3d: /* move    exec->words[1],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x103b3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b3e: /* move    l:-(exec->addresses[1]),y */
    exec->long_address[0] = md_agu(&exec->addresses[1], exec->addresses[9], -(1), 1u); exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); exec->words[3] = exec->parallel_words[0]; exec->words[2] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x103b3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b3f: /* move    exec->words[3],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[3]));
    MD_TRANSLATION_TRACE(0x103b3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b40: /* move    exec->words[2],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x103b40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b41: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b41u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b43: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103b43u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void e12rs_render_block_44k1(E12RsVoice *v, const int32_t *sine) {
    E12Rs_Render_Block_44K1Exec execution;
    E12Rs_Render_Block_44K1Exec *exec;
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
    uint32_t loop_103a37 = 0;
    uint32_t loop_103a70 = 0;
    uint32_t loop_103a84 = 0;
    uint32_t loop_103a9b = 0;
    uint32_t loop_103abb = 0;
    uint32_t loop_103ad2 = 0;
    uint32_t loop_103ade = 0;
    uint32_t loop_103aea = 0;
    uint32_t loop_103b30 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_103a10:
    e12rs_render_block_44k1_group_103a10(exec,v,sine);
L_103a18:
    e12rs_render_block_44k1_group_103a18(exec,v,sine);
L_103a21: /* ble     func_103a25 */
    if (!exec->flags[2]) goto L_103a25;
    MD_TRANSLATION_TRACE(0x103a21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a23:
    e12rs_render_block_44k1_group_103a23(exec,v,sine);
L_103a25:
    e12rs_render_block_44k1_group_103a25(exec,v,sine);
L_103a2d:
    e12rs_render_block_44k1_group_103a2d(exec,v,sine);
L_103a37: /* do      #<$20,>$103a3f */
    loop_103a37 = 0x20u; if (loop_103a37 == 0u) goto L_103a3f;
    MD_TRANSLATION_TRACE(0x103a37u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a39:
    e12rs_render_block_44k1_group_103a39(exec,v,sine);
    if (--loop_103a37 != 0u) goto L_103a39;
L_103a3f:
    e12rs_render_block_44k1_group_103a3f(exec,v,sine);
L_103a49: /* blt     func_103a4d */
    if (exec->flags[1]) goto L_103a4d;
    MD_TRANSLATION_TRACE(0x103a49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a4b:
    e12rs_render_block_44k1_group_103a4b(exec,v,sine);
L_103a4d:
    e12rs_render_block_44k1_group_103a4d(exec,v,sine);
L_103a54: /* blt     func_103a88 */
    if (exec->flags[1]) goto L_103a88;
    MD_TRANSLATION_TRACE(0x103a54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a56:
    e12rs_render_block_44k1_group_103a56(exec,v,sine);
L_103a5a: /* bge     func_103a89 */
    if (!exec->flags[1]) goto L_103a89;
    MD_TRANSLATION_TRACE(0x103a5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a5c:
    e12rs_render_block_44k1_group_103a5c(exec,v,sine);
L_103a65:
    e12rs_render_block_44k1_group_103a65(exec,v,sine);
L_103a6e:
    e12rs_render_block_44k1_group_103a6e(exec,v,sine);
L_103a70: /* do      exec->words[0],>$103a76 */
    loop_103a70 = exec->words[0]; if (loop_103a70 == 0u) goto L_103a76;
    MD_TRANSLATION_TRACE(0x103a70u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a72:
    e12rs_render_block_44k1_group_103a72(exec,v,sine);
    if (--loop_103a70 != 0u) goto L_103a72;
L_103a76:
    e12rs_render_block_44k1_group_103a76(exec,v,sine);
L_103a7a: /* bge     func_103a87 */
    if (!exec->flags[1]) goto L_103a87;
    MD_TRANSLATION_TRACE(0x103a7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a7c:
    e12rs_render_block_44k1_group_103a7c(exec,v,sine);
L_103a84: /* do      exec->words[2],>$103a87 */
    loop_103a84 = exec->words[2]; if (loop_103a84 == 0u) goto L_103a87;
    MD_TRANSLATION_TRACE(0x103a84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a86:
    e12rs_render_block_44k1_group_103a86(exec,v,sine);
    if (--loop_103a84 != 0u) goto L_103a86;
L_103a87:
    e12rs_render_block_44k1_group_103a87(exec,v,sine);
L_103a88:
    e12rs_render_block_44k1_group_103a88(exec,v,sine);
L_103a89:
    e12rs_render_block_44k1_group_103a89(exec,v,sine);
L_103a92:
    e12rs_render_block_44k1_group_103a92(exec,v,sine);
L_103a97: /* bcc     func_103a9b */
    if (!exec->flags[4]) goto L_103a9b;
    MD_TRANSLATION_TRACE(0x103a97u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a99:
    e12rs_render_block_44k1_group_103a99(exec,v,sine);
L_103a9b: /* do      #<$22,>$103aa5 */
    loop_103a9b = 0x22u; if (loop_103a9b == 0u) goto L_103aa5;
    MD_TRANSLATION_TRACE(0x103a9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103a9d:
    e12rs_render_block_44k1_group_103a9d(exec,v,sine);
    if (--loop_103a9b != 0u) goto L_103a9d;
L_103aa5:
    e12rs_render_block_44k1_group_103aa5(exec,v,sine);
L_103aac: /* bge     func_103ac7 */
    if (!exec->flags[1]) goto L_103ac7;
    MD_TRANSLATION_TRACE(0x103aacu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aae:
    e12rs_render_block_44k1_group_103aae(exec,v,sine);
L_103ab7: /* bcc     func_103abb */
    if (!exec->flags[4]) goto L_103abb;
    MD_TRANSLATION_TRACE(0x103ab7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ab9:
    e12rs_render_block_44k1_group_103ab9(exec,v,sine);
L_103abb: /* do      #<$22,>$103ac5 */
    loop_103abb = 0x22u; if (loop_103abb == 0u) goto L_103ac5;
    MD_TRANSLATION_TRACE(0x103abbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103abd:
    e12rs_render_block_44k1_group_103abd(exec,v,sine);
    if (--loop_103abb != 0u) goto L_103abd;
L_103ac5:
    e12rs_render_block_44k1_group_103ac5(exec,v,sine);
L_103ac7:
    e12rs_render_block_44k1_group_103ac7(exec,v,sine);
L_103ad0:
    e12rs_render_block_44k1_group_103ad0(exec,v,sine);
L_103ad2: /* do      #<$44,>$103ad8 */
    loop_103ad2 = 0x44u; if (loop_103ad2 == 0u) goto L_103ad8;
    MD_TRANSLATION_TRACE(0x103ad2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ad4:
    e12rs_render_block_44k1_group_103ad4(exec,v,sine);
    if (--loop_103ad2 != 0u) goto L_103ad4;
L_103ad8:
    e12rs_render_block_44k1_group_103ad8(exec,v,sine);
L_103ade: /* do      #<$20,>$103ae3 */
    loop_103ade = 0x20u; if (loop_103ade == 0u) goto L_103ae3;
    MD_TRANSLATION_TRACE(0x103adeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103ae0:
    e12rs_render_block_44k1_group_103ae0(exec,v,sine);
    if (--loop_103ade != 0u) goto L_103ae0;
L_103ae3:
    e12rs_render_block_44k1_group_103ae3(exec,v,sine);
L_103aea: /* do      #<$20,>$103af6 */
    loop_103aea = 0x20u; if (loop_103aea == 0u) goto L_103af6;
    MD_TRANSLATION_TRACE(0x103aeau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103aec:
    e12rs_render_block_44k1_group_103aec(exec,v,sine);
L_103af4:
    e12rs_render_block_44k1_group_103af4(exec,v,sine);
    if (--loop_103aea != 0u) goto L_103aec;
L_103af6:
    e12rs_render_block_44k1_group_103af6(exec,v,sine);
L_103b01:
    e12rs_render_block_44k1_group_103b01(exec,v,sine);
L_103b0b:
    e12rs_render_block_44k1_group_103b0b(exec,v,sine);
L_103b14:
    e12rs_render_block_44k1_group_103b14(exec,v,sine);
L_103b1d:
    e12rs_render_block_44k1_group_103b1d(exec,v,sine);
L_103b26:
    e12rs_render_block_44k1_group_103b26(exec,v,sine);
L_103b30: /* do      #<$20,>$103b3b */
    loop_103b30 = 0x20u; if (loop_103b30 == 0u) goto L_103b3b;
    MD_TRANSLATION_TRACE(0x103b30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_103b32:
    e12rs_render_block_44k1_group_103b32(exec,v,sine);
L_103b3a:
    e12rs_render_block_44k1_group_103b3a(exec,v,sine);
    if (--loop_103b30 != 0u) goto L_103b32;
L_103b3b:
    e12rs_render_block_44k1_group_103b3b(exec,v,sine);
L_103b45: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t e12rs_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_20325e: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_203262: /* movem.l d[2]-d[3]/a2, (a7) */
    /* native locals replace original callee saves */
C_203266: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_20326a: /* movea.l $14(a7), a2 */
    /* resolved argument pointer */
C_20326e: /* move.w (a2), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_203270: /* clr.l d[0] */
    d[0] = 0u;
C_203272: /* move.w d[2], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[2] & 0xffffu)) & 0xffffu);
C_203274: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_203276: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203278: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_20327a: /* addq.l #$8, d[1] */
    d[1] = d[1] + 0x8u;
C_20327c: /* moveq #$7f, d[3] */
    d[3] = 0x0000007fu;
C_20327e: /* and.l d[3], d[0] */
    lhs = d[0]; rhs = d[3]; d[0] = lhs & rhs;
C_203280: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_203286: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_20328a: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20328c: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_203290: /* clr.l $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(0u);
C_203294: /* clr.l d[0] */
    d[0] = 0u;
C_203296: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_20329a: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20329c: /* move.b #$3, d[3] */
    d[3] = (d[3] & 0xffffff00u) | ((0x3u) & 0xffu);
C_2032a0: /* divs.l d[3], d[0] */
    d[0] = md_divs32(d[0], d[3]);
C_2032a4: /* asr.l #$6, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x6u) & 63u) < 32u ? ((0x6u) & 63u) : 31u));
C_2032a6: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2032a8: /* movea.l d[0], a0 */
    a0 = d[0];
C_2032aa: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2032b0: /* move.l $190(a0), $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((400u) + a0)) / 4u)));
C_2032b6: /* clr.l $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(0u);
C_2032ba: /* clr.l d[0] */
    d[0] = 0u;
C_2032bc: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2032c0: /* lsr.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] >> ((0x3u) & 63u) : 0u);
C_2032c2: /* mulu.w #$cccd, d[2] */
    d[2] = (d[2] & 0xffffu) * (0xcccdu);
C_2032c6: /* moveq #$15, d[1] */
    d[1] = 0x00000015u;
C_2032c8: /* lsr.l d[1], d[2] */
    d[2] = (((d[1]) & 63u) < 32u ? d[2] >> ((d[1]) & 63u) : 0u);
C_2032ca: /* movea.l d[2], a0 */
    a0 = d[2];
C_2032cc: /* lea.l $32(a0, d[0].l), a0 */
    a0 = (d[0] * 1u + (50)) + a0;
C_2032d0: /* move.l a0, d[1] */
    d[1] = a0;
C_2032d2: /* cmpa.l #$7ff, a0 */
    lhs = a0; rhs = 0x7ffu;
C_2032d8: /* ble.b $2032e2 */
    if ((int32_t)lhs <= (int32_t)rhs) goto C_2032e2;
C_2032da: /* move.l #$7ff, d[1] */
    d[1] = 0x7ffu;
C_2032e0: /* bra.b $2032ec */
    goto C_2032ec;
C_2032e2: /* move.l a0, d[0] */
    d[0] = a0;
C_2032e4: /* not.l d[0] */
    d[0] = ~d[0];
C_2032e6: /* add.l d[0], d[0] */
    lhs = d[0]; rhs = d[0]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2032e8: /* subx.l d[0], d[0] */
    lhs = d[0]; rhs = d[0]; cf_temp[0] = lhs - rhs - cf_carry[0]; cf_carry[0] = (lhs < rhs || (cf_carry[0] && lhs == rhs)); d[0] = cf_temp[0];
C_2032ea: /* and.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; d[1] = lhs & rhs;
C_2032ec: /* move.l d[1], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[1]);
C_2032f0: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_2032f4: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_2032f6: /* andi.l #$7fff80, d[0] */
    lhs = d[0]; rhs = 0x7fff80u; d[0] = lhs & rhs;
C_2032fc: /* move.l d[0], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_203300: /* move.l #$7fffff, d[0] */
    d[0] = 0x7fffffu;
C_203306: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_20330a: /* clr.l d[0] */
    d[0] = 0u;
C_20330c: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_203310: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_203312: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203314: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_203316: /* addi.l #$b, d[1] */
    lhs = d[1]; rhs = 0xbu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_20331c: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_20331e: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_203320: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_203326: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_20332a: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20332c: /* addi.l #$ff800000, d[0] */
    lhs = d[0]; rhs = 0xff800000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203332: /* move.l d[0], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_203336: /* clr.l d[0] */
    d[0] = 0u;
C_203338: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_20333c: /* cmpi.l #$3f40, d[0] */
    lhs = d[0]; rhs = 0x3f40u;
C_203342: /* bls.b $20334e */
    if (lhs <= rhs) goto C_20334e;
C_203344: /* moveq #$7f, d[3] */
    d[3] = 0x0000007fu;
C_203346: /* swap d[3] */
    d[3] = (d[3] << 16) | (d[3] >> 16);
C_203348: /* move.l d[3], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[3]);
C_20334c: /* bra.b $203372 */
    goto C_203372;
C_20334e: /* cmpi.l #$1fff, d[0] */
    lhs = d[0]; rhs = 0x1fffu;
C_203354: /* bhi.b $203360 */
    if (lhs > rhs) goto C_203360;
C_203356: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20335c: /* asr.l #$7, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_20335e: /* bra.b $20336e */
    goto C_20336e;
C_203360: /* muls.l d[0], d[0] */
    d[0] = d[0] * (d[0]);
C_203364: /* moveq #$15, d[1] */
    d[1] = 0x00000015u;
C_203366: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_203368: /* addi.l #$20, d[0] */
    lhs = d[0]; rhs = 0x20u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20336e: /* move.l d[0], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_203372: /* clr.l d[2] */
    d[2] = 0u;
C_203374: /* move.w $c(a2), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_203378: /* cmpi.l #$1fff, d[2] */
    lhs = d[2]; rhs = 0x1fffu;
C_20337e: /* bhi.b $2033aa */
    if (lhs > rhs) goto C_2033aa;
C_203380: /* move.l #$10266000, d[0] */
    d[0] = 0x10266000u;
C_203386: /* move.l $100150c.l, d[3] */
    d[3] = md_control_tempo_denominator;
C_20338c: /* divs.l d[3], d[0] */
    d[0] = md_divs32(d[0], d[3]);
C_203390: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203392: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_203394: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_203396: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_203398: /* moveq #$c, d[0] */
    d[0] = 0x0000000cu;
C_20339a: /* asr.l d[0], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[0]) & 63u) < 32u ? ((d[0]) & 63u) : 31u));
C_20339c: /* muls.l d[1], d[2] */
    d[2] = d[2] * (d[1]);
C_2033a0: /* moveq #$d, d[1] */
    d[1] = 0x0000000du;
C_2033a2: /* asr.l d[1], d[2] */
    d[2] = (uint32_t)((int32_t)d[2] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2033a4: /* move.l d[2], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[2]);
C_2033a8: /* bra.b $203400 */
    goto C_203400;
C_2033aa: /* move.l d[2], d[0] */
    d[0] = d[2];
C_2033ac: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_2033ae: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_2033b0: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_2033b4: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_2033b6: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2033b8: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_2033ba: /* moveq #$5, d[3] */
    d[3] = 0x00000005u;
C_2033bc: /* sub.l d[1], d[3] */
    lhs = d[3]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[3] = lhs - rhs;
C_2033be: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_2033c0: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_2033c6: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_2033ca: /* asr.l d[3], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[3]) & 63u) < 32u ? ((d[3]) & 63u) : 31u));
C_2033cc: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2033ce: /* lsl.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] << ((0x7u) & 63u) : 0u);
C_2033d0: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_2033d2: /* add.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2033d4: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2033d6: /* lsl.l #$3, d[1] */
    d[1] = (((0x3u) & 63u) < 32u ? d[1] << ((0x3u) & 63u) : 0u);
C_2033d8: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_2033da: /* move.l d[1], d[2] */
    d[2] = d[1];
C_2033dc: /* lsl.l #$4, d[2] */
    d[2] = (((0x4u) & 63u) < 32u ? d[2] << ((0x4u) & 63u) : 0u);
C_2033de: /* sub.l d[1], d[2] */
    lhs = d[2]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_2033e0: /* move.l d[2], d[0] */
    d[0] = d[2];
C_2033e2: /* lsl.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] << ((0x6u) & 63u) : 0u);
C_2033e4: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_2033e6: /* move.l $100150c.l, d[2] */
    d[2] = md_control_tempo_denominator;
C_2033ec: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_2033f0: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2033f2: /* lsl.l #$6, d[1] */
    d[1] = (((0x6u) & 63u) < 32u ? d[1] << ((0x6u) & 63u) : 0u);
C_2033f4: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2033f6: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_2033f8: /* moveq #$c, d[3] */
    d[3] = 0x0000000cu;
C_2033fa: /* asr.l d[3], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[3]) & 63u) < 32u ? ((d[3]) & 63u) : 31u));
C_2033fc: /* move.l d[1], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[1]);
C_203400: /* lea.l $28(a1), a0 */
    a0 = 40u;
C_203404: /* moveq #$10, d[0] */
    d[0] = 0x00000010u;
C_203406: /* cmp.l (a0), d[0] */
    lhs = d[0]; rhs = (uint32_t)*(y + ((0u) + a0) / 4u);
C_203408: /* blt.b $203410 */
    if ((int32_t)lhs < (int32_t)rhs) goto C_203410;
C_20340a: /* move.l #$11, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x11u);
C_203410: /* clr.l d[0] */
    d[0] = 0u;
C_203412: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_203416: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_203418: /* lea.l $24bb90.l, a0 */
    /* resolved table pointer */
C_20341e: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((0u + (d[0] * 4u + (0))) / 4u));
C_203422: /* lsl.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] << ((0x6u) & 63u) : 0u);
C_203424: /* move.l d[0], $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(d[0]);
C_203428: /* moveq #$c, d[0] */
    d[0] = 0x0000000cu;
C_20342a: /* movem.l (a7), d[2]-d[3]/a2 */
    /* native locals replace original callee saves */
C_20342e: /* lea.l $c(a7), a7 */
    /* local register save area */
C_203432: /* rts  */
    return d[0];
}

void e12rs_update_coeffs(const uint32_t *p, E12RsVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = e12rs_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { e12rs_dsp_init(v, 0); }
}
void e12rs_voice_init(E12RsVoice *v) {
    uint32_t i;
    for (i = 0u; i < 37u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void e12rs_voice_trigger(E12RsVoice *v) {
    e12rs_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t e12rs_resample_run(E12RsVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE E12RsVoice
#define MD_RENDER48 e12rs_render_block_48k_float
#define MD_RENDER44 e12rs_render_block_44k1
#define MD_RESAMPLE_RUN e12rs_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE E12RsVoice
#define MD_SLOT_INIT e12rs_slot_init
#define MD_SLOT_PROCESS e12rs_slot_process
#define MD_VOICE_INIT e12rs_voice_init
#define MD_COEFFICIENTS e12rs_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER e12rs_update_coeffs
#define MD_VOICE_TRIGGER e12rs_voice_trigger
#define MD_RENDER44 e12rs_render_block_44k1
#define MD_RENDER48 e12rs_render_block_48k_float
#include "../engine_slot.inc"
#endif
