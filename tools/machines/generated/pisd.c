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
    int32_t y[64];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;
    int32_t x_private[64]; int32_t external[1536]; uint32_t virtual_voice; int32_t scratch_y20_history;
} PisdVoice;
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

static uint32_t md_read_source(PisdVoice *v, uint32_t address) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) { value = md_source_words[address - 0x20000u]; }
    else if (address >= 0x200000u && address < 0xffff80u) { value = 0u; }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
extern const uint32_t source_142b49[512];
extern const uint32_t source_142d49[514];
extern const uint32_t source_142949[512];
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

static uint32_t md_read_x(PisdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }
    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { value = u24(v->x_private[address - 0x800u - v->virtual_voice * 64u]); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1321801u && address < 1322313u) { value = *(source_142b49 + (address - 1321801u)); }
    else if (address >= 1322313u && address < 1322827u) { value = *(source_142d49 + (address - 1322313u)); }
    else if (address >= 1321289u && address < 1321801u) { value = *(source_142949 + (address - 1321289u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(PisdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }
    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { v->x_private[address - 0x800u - v->virtual_voice * 64u] = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(PisdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }
    else if (address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u) { value = u24(*(v->y + (address - 0x800u - v->virtual_voice*64u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1321801u && address < 1322313u) { value = *(source_142b49 + (address - 1321801u)); }
    else if (address >= 1322313u && address < 1322827u) { value = *(source_142d49 + (address - 1322313u)); }
    else if (address >= 1321289u && address < 1321801u) { value = *(source_142949 + (address - 1321289u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(PisdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }
    else if (address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u) { *(v->y + (address - 0x800u - v->virtual_voice*64u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(PisdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(PisdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(PisdVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef PISD_DSP_INIT_EXEC_DEFINED
#define PISD_DSP_INIT_EXEC_DEFINED
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
} Pisd_Dsp_InitExec;
#endif
static void pisd_dsp_init_group_142d49(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d49: /* lua     (exec->addresses[6]+$29),exec->addresses[3] */
    exec->addresses[3] = (md_index(exec->addresses[6],exec->addresses[14],0x29u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d4a: /* lua     (exec->addresses[6]+$1d),exec->addresses[4] */
    exec->addresses[4] = (md_index(exec->addresses[6],exec->addresses[14],0x1du)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d4b: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d4c: /* move    #$0,exec->words[1] */
    exec->words[1] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d4d: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_init_group_142d50(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d50: /* move    x,l:(exec->addresses[3])+ */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = (uint32_t)(sx24((int32_t)exec->words[1])); exec->parallel_words[3] = exec->words[0]; md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x142d50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_init_group_142d52(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d52: /* move    exec->words[0],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[0]);
    MD_TRANSLATION_TRACE(0x142d52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_init_group_142d53(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d53: /* move    #>$135600,a */
    exec->accum[0] = sx24((int32_t)(0x135600u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142d53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d55: /* move    y:>$142,exec->words[0] */
    exec->words[0] = (v->virtual_voice) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d57: /* mpyi    #>$300,exec->words[0],b */
    mpy_ss32(sx24((int32_t)(0x300u)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142d57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d59: /* move    b0,exec->words[1] */
    exec->words[1] = (exec->lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d5a: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142d5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d5b: /* move    #>$7ffcb9,exec->words[0] */
    exec->words[0] = (0x7ffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d5d: /* move    exec->words[0],y:(exec->addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d5f: /* move    exec->words[0],y:(exec->addresses[6]+$6) */
    *(v->y + 6u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_init_group_142d61(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d61: /* move    a,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142d61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d63: /* add     #>$200,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x200u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142d63u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d65: /* move    #>$10b,exec->words[0] */
    exec->words[0] = (0x10bu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d67: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d69: /* move    a,y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142d69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d6b: /* add     #>$200,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x200u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142d6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d6d: /* move    #>$ff,exec->words[0] */
    exec->words[0] = (0xffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d6du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d6f: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d6fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_init_group_142d71(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d71: /* move    a,y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142d71u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d73: /* move    #>$b5,exec->words[0] */
    exec->words[0] = (0xb5u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d75: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d77: /* move    #>$19999a,exec->words[0] */
    exec->words[0] = (0x19999au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d79: /* move    exec->words[0],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d79u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d7b: /* move    #>$59999a,exec->words[0] */
    exec->words[0] = (0x59999au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d7bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d7d: /* move    exec->words[0],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d7f: /* move    #>$a,exec->words[0] */
    exec->words[0] = (0xau) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d7fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_init_group_142d81(Pisd_Dsp_InitExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d81: /* move    exec->words[0],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d83: /* move    #>$1,exec->words[0] */
    exec->words[0] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d83u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d85: /* move    exec->words[0],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d85u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void pisd_dsp_init(PisdVoice *v, const int32_t *sine) {
    Pisd_Dsp_InitExec execution;
    Pisd_Dsp_InitExec *exec;
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
    exec->addresses[6] = (0x800u + 64u * v->virtual_voice);
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
L_142d49:
    pisd_dsp_init_group_142d49(exec,v,sine);
L_142d4f: /* rep     #<$10 */
    for (exec->repeat = 0x10u; exec->repeat > 0u; exec->repeat--) { exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = (uint32_t)(sx24((int32_t)exec->words[1])); exec->parallel_words[3] = exec->words[0]; md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]); } goto L_142d51;
    MD_TRANSLATION_TRACE(0x142d4fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d50:
    pisd_dsp_init_group_142d50(exec,v,sine);
L_142d51: /* rep     #<$5 */
    for (exec->repeat = 0x5u; exec->repeat > 0u; exec->repeat--) { md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[0]); } goto L_142d53;
    MD_TRANSLATION_TRACE(0x142d51u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d52:
    pisd_dsp_init_group_142d52(exec,v,sine);
L_142d53:
    pisd_dsp_init_group_142d53(exec,v,sine);
L_142d61:
    pisd_dsp_init_group_142d61(exec,v,sine);
L_142d71:
    pisd_dsp_init_group_142d71(exec,v,sine);
L_142d81:
    pisd_dsp_init_group_142d81(exec,v,sine);
L_142d87: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

#ifndef PISD_DSP_TRIGGER_EXEC_DEFINED
#define PISD_DSP_TRIGGER_EXEC_DEFINED
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
} Pisd_Dsp_TriggerExec;
#endif
static void pisd_dsp_trigger_group_142d88(Pisd_Dsp_TriggerExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d88: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d89: /* move    exec->words[0],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d8a: /* move    exec->words[0],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d8b: /* move    #>$142f4b,exec->words[0] */
    exec->words[0] = (0x142f4bu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d8bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d8d: /* move    exec->words[0],x:(exec->addresses[6]+$11) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[6] + 17u, sine, exec->words[0]);
    MD_TRANSLATION_TRACE(0x142d8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d8e: /* move    #>$7fffff,exec->words[0] */
    exec->words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d90: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d91: /* move    exec->words[0],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_dsp_trigger_group_142d92(Pisd_Dsp_TriggerExec *exec, PisdVoice *v, const int32_t *sine) {
L_142d92: /* move    exec->words[0],y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d92u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d93: /* move    exec->words[0],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142d93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void pisd_dsp_trigger(PisdVoice *v, const int32_t *sine) {
    Pisd_Dsp_TriggerExec execution;
    Pisd_Dsp_TriggerExec *exec;
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
    exec->addresses[6] = (0x800u + 64u * v->virtual_voice);
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
L_142d88:
    pisd_dsp_trigger_group_142d88(exec,v,sine);
L_142d92:
    pisd_dsp_trigger_group_142d92(exec,v,sine);
L_142d94: /* rts */
    return;}

#ifndef PISD_RENDER_BLOCK_44K1_EXEC_DEFINED
#define PISD_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Pisd_Render_Block_44K1Exec;
#endif
static void pisd_render_block_44k1_group_142d95(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142d95: /* move    y:(exec->addresses[6]+$13),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d95u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d96: /* move    y:(exec->addresses[6]+$f),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d96u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d97: /* move    #>$7f7cee,exec->words[3] */
    exec->words[3] = (0x7f7ceeu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d97u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d99: /* mpy     exec->words[3],exec->words[1],b         #>$7c28f6,exec->words[2] */
    exec->parallel_words[0] = 0x7c28f6u; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d9b: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142d9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d9c: /* move    b,y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x142d9cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d9d: /* move    a,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142d9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142d9e: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d9eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142d9f(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142d9f: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142d9fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da0: /* move    y:(exec->addresses[6]+$3),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142da0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da1: /* move    y:(exec->addresses[6]+$5),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142da1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da2: /* mpy     exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142da2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da3: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142da3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da4: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142da4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da5: /* add     #>$400000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142da5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da7: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142da7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142da8(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142da8: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142da8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142da9: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142da9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142daa: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142daau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dab: /* move    a,y:>$fe */
    *(md_shared_scratch_y + 254u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142dabu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dad: /* asr     #$16,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 22u; exec->lows[0] = ((exec->lows[2] >> 22u) | ((uint32_t)exec->accum[2] << 2u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142dadu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dae: /* move    x:(exec->addresses[6]+$11),b */
    exec->accum[1] = sx24((int32_t)(md_read_x(v, md_shared_scratch_x, exec->addresses[6] + 17u, sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142daeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142daf: /* move    y:(exec->addresses[6]+$11),b0 */
    exec->lows[1] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dafu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db0: /* move    a0,exec->words[0] */
    exec->words[0] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142db0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142db1(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142db1: /* asl     a               a1,exec->words[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142db1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db2: /* move    #$0,exec->addresses[0] */
    exec->addresses[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142db2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db3: /* move    a0,exec->words[2] */
    exec->words[2] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142db3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db4: /* tfr     b,a             a1,exec->words[3] */
    exec->parallel_words[0] = u24(exec->accum[0]); exec->accum[0] = exec->accum[1]; exec->lows[0] = exec->lows[1]; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142db4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db5: /* add     x,b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142db5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142db8(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142db8: /* add     y,a             a,l:(exec->addresses[0])+ */
    exec->long_address[0] = md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)exec->words[3]), exec->words[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x142db8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db9: /* add     y,b             b,l:(exec->addresses[0])+ */
    exec->long_address[0] = md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[1]))); exec->parallel_words[3] = (exec->accum[1] > 0x7fffff ? 0xffffffu : exec->accum[1] < -0x800000 ? 0u : exec->lows[1]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[3]), exec->words[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x142db9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142dba(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142dba: /* move    #>$1433b6,exec->words[0] */
    exec->words[0] = (0x1433b6u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dbau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dbc: /* cmp     exec->words[0],a */
    exec->accum[2] = sx24((int32_t)(exec->words[0])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x142dbcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dbd: /* tge     exec->words[0],a */
    if (!exec->flags[1]) { exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; }
    MD_TRANSLATION_TRACE(0x142dbdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dbe: /* move    a1,x:(exec->addresses[6]+$11) */
    md_write_x(v, md_shared_scratch_x, exec->addresses[6] + 17u, sine, u24(exec->accum[0]));
    MD_TRANSLATION_TRACE(0x142dbeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dbf: /* move    a0,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x142dbfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dc0: /* move    #$0,exec->words[3] */
    exec->words[3] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dc0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dc1: /* move    #>$7fffff,exec->words[2] */
    exec->words[2] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dc1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dc3: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dc3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142dc4(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142dc4: /* move    #$20,exec->addresses[5] */
    exec->addresses[5] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dc4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142dc7(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142dc7: /* move    y:(exec->addresses[4])+,a0 */
    exec->lows[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dc7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dc8: /* asr     a               y:(exec->addresses[4])-,b0 */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], -1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 1u; exec->lows[0] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->lows[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dc8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dc9: /* asr     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = exec->accum[2] >> 1u; exec->lows[1] = ((exec->lows[2] >> 1u) | ((uint32_t)exec->accum[2] << 23u)) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142dc9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dca: /* sub     y,a             a0,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = exec->lows[0]; sub56(exec->accum[0], exec->lows[0], sx24((int32_t)exec->words[3]), exec->words[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142dcau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dcb: /* sub     y,b             b0,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = exec->lows[1]; sub56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[3]), exec->words[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142dcbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dcc: /* clr     a               a0,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = exec->lows[0]; exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142dccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dcd: /* clr     b               b0,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = exec->lows[1]; exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142dcdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142dce(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142dce: /* move    #$20,exec->addresses[5] */
    exec->addresses[5] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dcf: /* move    x:$0,exec->addresses[1] */
    exec->addresses[1] = (u24(*(md_shared_scratch_x + 0u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dcfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd0: /* move    #$1,exec->addresses[0] */
    exec->addresses[0] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd1: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd2: /* move    #$40,exec->addresses[3] */
    exec->addresses[3] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd3: /* move    x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd4: /* move    x:(exec->addresses[0])+,exec->addresses[2] */
    exec->addresses[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd5: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142dd8(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142dd8: /* move    x:(exec->addresses[0])+,exec->addresses[1] */
    exec->addresses[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd9: /* mac     -exec->words[2],exec->words[0],b        x:(exec->addresses[2])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dd9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dda: /* mpy     exec->words[2],exec->words[0],a         x:(exec->addresses[2])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ddau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ddb: /* move    b,y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x142ddbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ddc: /* move    x:(exec->addresses[0])+,exec->addresses[2] */
    exec->addresses[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ddcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ddd: /* mac     -exec->words[2],exec->words[0],a        x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dde: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ddeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ddf: /* move    a,y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x142ddfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142de0(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142de0: /* move    #>$7f5c29,exec->words[1] */
    exec->words[1] = (0x7f5c29u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142de0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de2: /* move    y:(exec->addresses[6]+$26),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 38u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142de2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de3: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142de3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de4: /* move    y:(exec->addresses[6]+$7),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142de4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de5: /* move    y:(exec->addresses[6]+$24),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142de5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de6: /* move    a,y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142de6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de7: /* mpy     exec->words[1],exec->words[0],a         #>$47ae14,exec->words[1] */
    exec->parallel_words[0] = 0x47ae14u; mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142de7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142de9: /* move    y:(exec->addresses[6]+$25),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 37u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142de9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142dea(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142dea: /* move    a,y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142deau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142deb: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142debu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dec: /* move    #>$142b49,exec->addresses[0] */
    exec->addresses[0] = (0x142b49u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142decu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dee: /* move    a,y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142deeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142def: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142defu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142df0: /* mpyi    #>$1e,exec->words[0],b */
    mpy_ss32(sx24((int32_t)(0x1eu)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142df0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142df2: /* move    #>$5a827a,exec->words[0] */
    exec->words[0] = (0x5a827au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142df2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142df4: /* move    b,exec->addresses[16] */
    exec->addresses[16] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142df4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142df5(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142df5: /* move    y:(exec->addresses[6]+$26),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 38u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142df5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142df6: /* move    #>$333333,a */
    exec->accum[0] = sx24((int32_t)(0x333333u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142df6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142df8: /* maci    #>$d9999a,exec->words[2],a */
    mpy_ss32(sx24((int32_t)(0xd9999au)), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142df8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dfa: /* move    x:(exec->addresses[0]+exec->addresses[16]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_index(exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16])), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142dfau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dfb: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142dfbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dfc: /* move    exec->words[1],x:>$fb */
    *(md_shared_scratch_x + 251u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x142dfcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dfe: /* move    b,x:>$fc */
    *(md_shared_scratch_x + 252u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x142dfeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e00: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e01(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e01: /* move    exec->words[1],x:>$f9 */
    *(md_shared_scratch_x + 249u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x142e01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e03: /* move    y:(exec->addresses[6]+$17),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e05: /* move    y:(exec->addresses[6]+$14),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e07: /* move    #>$1ff,exec->addresses[10] */
    exec->addresses[10] = (0x1ffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e09: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e0a: /* move    y:(exec->addresses[6]+$4),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e0au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e0b: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e0c: /* move    #$60,exec->addresses[1] */
    exec->addresses[1] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e0d(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e0d: /* move    (exec->addresses[2])+exec->addresses[18] */
    (void)md_agu(&exec->addresses[2],exec->addresses[10],sx24((int32_t)exec->addresses[18]),0u);
    MD_TRANSLATION_TRACE(0x142e0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e0e: /* move    y:(exec->addresses[6]+$12),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 18u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142e0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e0f: /* move    y:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e10: /* move    y:(exec->addresses[6]+$1d),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 29u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e11: /* move    x:(exec->addresses[2])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e14(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e14: /* mac     exec->words[2],exec->words[0],a         a,x:(exec->addresses[1])+       a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e15: /* mac     -exec->words[2],exec->words[0],a        y:(exec->addresses[0])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e16: /* mac     exec->words[3],exec->words[1],b         b,x:(exec->addresses[3])+       b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e17: /* mac     -exec->words[3],exec->words[1],b        x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e18(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e18: /* move    y:(exec->addresses[6]+$15),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e1a: /* move    y:(exec->addresses[6]+$18),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e1c: /* move    b,y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x142e1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e1d: /* move    y:(exec->addresses[6]+$1e),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 30u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e1e: /* move    (exec->addresses[2])+exec->addresses[18] */
    (void)md_agu(&exec->addresses[2],exec->addresses[10],sx24((int32_t)exec->addresses[18]),0u);
    MD_TRANSLATION_TRACE(0x142e1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e1f: /* move    #$a0,exec->addresses[3] */
    exec->addresses[3] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e20: /* move    a,y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142e20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e21: /* move    x:(exec->addresses[2])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e24(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e24: /* mac     exec->words[3],exec->words[1],b         b,x:(exec->addresses[3])+       b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e25: /* mac     -exec->words[3],exec->words[1],b        x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e26(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e26: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e27: /* move    b,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x142e27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e28: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e29: /* clr     b */
    exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e2a: /* move    y:(exec->addresses[6]+$22),b1 */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(*(v->y + 34u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x142e2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e2b: /* move    y:(exec->addresses[6]+$23),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 35u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e2e(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e2e: /* add     exec->words[1],b            exec->words[1],a1 */
    exec->parallel_words[0] = exec->words[1]; add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x142e2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e2f: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x142e2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e30: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e31: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e32: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x142e32u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e33: /* move    a1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e34(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e34: /* move    exec->words[1],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x142e34u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e35: /* move    b1,y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x142e35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e36: /* move    b1,y:>$fa */
    *(md_shared_scratch_y + 250u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x142e36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e38: /* move    x:>$f9,exec->words[2] */
    exec->words[2] = (u24(*(md_shared_scratch_x + 249u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e3a: /* move    y:(exec->addresses[6]+$16),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 22u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e3c: /* move    y:(exec->addresses[6]+$19),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e3cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e3e: /* move    y:(exec->addresses[6]+$1f),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 31u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142e3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e3f: /* move    #$c0,exec->addresses[3] */
    exec->addresses[3] = (0xc0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e40(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e40: /* move    (exec->addresses[2])+exec->addresses[18] */
    (void)md_agu(&exec->addresses[2],exec->addresses[10],sx24((int32_t)exec->addresses[18]),0u);
    MD_TRANSLATION_TRACE(0x142e40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e41: /* move    x:(exec->addresses[2])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e41u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e42: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e42u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e43: /* move    y:(exec->addresses[6]+$27),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 39u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e43u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e44: /* move    #>$d9999a,exec->words[1] */
    exec->words[1] = (0xd9999au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e44u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e48(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e48: /* mac     exec->words[3],exec->words[2],a         a,x:(exec->addresses[3])+       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e48u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e49: /* mac     -exec->words[3],exec->words[2],a        y:(exec->addresses[4]),exec->words[0] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[4], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e4a: /* add     exec->words[0],b            b,exec->words[0]            b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = u24(lim24(exec->accum[1])); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142e4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e4b: /* mac     exec->words[1],exec->words[0],b         x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e4c(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e4c: /* move    a,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x142e4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e4d: /* move    b,y:(exec->addresses[6]+$27) */
    *(v->y + 39u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x142e4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e4e: /* move    #>$ffffff,exec->addresses[10] */
    exec->addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e4eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e50: /* move    y:>$fa,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_y + 250u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142e50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e52: /* abs     a */
    if (exec->accum[0] < 0) { exec->accum[0] = (int32_t)(0u - (uint32_t)exec->accum[0] - (uint32_t)(exec->lows[0] != 0u)); exec->lows[0] = (0u - exec->lows[0]) & 0xffffffu; } exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e53: /* move    y:(exec->addresses[6]+$24),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e54: /* cmp     exec->words[1],a */
    exec->accum[2] = sx24((int32_t)(exec->words[1])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x142e54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e56(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e56: /* move    #>$7fffff,exec->words[0] */
    exec->words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e56u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e58: /* move    exec->words[0],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x142e58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e59(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e59: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e5a: /* move    y:(exec->addresses[6]+$6),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e5b: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e5c: /* mpy     exec->words[1],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e5cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e5d: /* move    #$40,exec->addresses[2] */
    exec->addresses[2] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e5e: /* move    exec->addresses[2],exec->addresses[5] */
    exec->addresses[5] = (exec->addresses[2]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e5eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e5f: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e60: /* mpyi    #>$266666,exec->words[2],a */
    mpy_ss32(sx24((int32_t)(0x266666u)), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e60u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e62(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e62: /* mpyi    #>$59999a,exec->words[2],b */
    mpy_ss32(sx24((int32_t)(0x59999au)), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142e62u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e64: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e65: /* move    #$5f,exec->addresses[3] */
    exec->addresses[3] = (0x5fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e66: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e66u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e67: /* move    b,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e68: /* move    x:(exec->addresses[3]),b */
    exec->accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e68u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e69: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e6a: /* move    y:(exec->addresses[5])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142e6au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e6d(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e6d: /* asl     a               b,x:(exec->addresses[3])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142e6du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e6e: /* mac     exec->words[3],exec->words[2],a         x:(exec->addresses[3]),b */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e6eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e6f: /* mac     exec->words[1],exec->words[2],b         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e6fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e70: /* move    a,x:(exec->addresses[2])+       y:(exec->addresses[5])+,a */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, u24(lim24(exec->accum[0]))); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142e70u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e71(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e71: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x142e71u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e72: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e72u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e73: /* move    #$a0,exec->addresses[1] */
    exec->addresses[1] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e74: /* move    #$c0,exec->addresses[2] */
    exec->addresses[2] = (0xc0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e74u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e75: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e76: /* move    exec->addresses[7],y:>$ff */
    *(md_shared_scratch_y + 255u) = sx24((int32_t)(exec->addresses[7]));
    MD_TRANSLATION_TRACE(0x142e76u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e78: /* move    #>$1ff,exec->addresses[15] */
    exec->addresses[15] = (0x1ffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e78u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e7a: /* move    y:(exec->addresses[6]+$16),exec->addresses[7] */
    exec->addresses[7] = (u24(*(v->y + 22u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e7c(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e7c: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e7d: /* move    #$20,exec->addresses[4] */
    exec->addresses[4] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e7e: /* move    #>$7eb852,exec->words[3] */
    exec->words[3] = (0x7eb852u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e7eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e80: /* move    x:>$fb,exec->words[1] */
    exec->words[1] = (u24(*(md_shared_scratch_x + 251u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e82: /* move    x:>$fc,exec->words[2] */
    exec->words[2] = (u24(*(md_shared_scratch_x + 252u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e82u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e84: /* move    x:(exec->addresses[0]),exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e85: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[1]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e85u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e86: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e87(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e87: /* mpy     exec->words[0],exec->words[3],a         x:(exec->addresses[3])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e87u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e88: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[2]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e89: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e8c(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e8c: /* mpy     exec->words[0],exec->words[3],a         x:(exec->addresses[1])+,exec->words[0]      a,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142e8cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e8d: /* mac     -exec->words[2],exec->words[0],a        x:(exec->addresses[2])+,exec->words[0]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142e8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e8e: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e8f: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[1]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e90: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0]),exec->words[0]       a,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142e90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e91: /* mpy     exec->words[0],exec->words[3],a         x:(exec->addresses[3])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e92: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[2]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e92u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e93: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e94(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e94: /* move    y:(exec->addresses[6]+$14),exec->addresses[0] */
    exec->addresses[0] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e94u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e96: /* move    y:(exec->addresses[6]+$15),exec->addresses[1] */
    exec->addresses[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e96u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e98: /* move    exec->addresses[15],exec->addresses[8] */
    exec->addresses[8] = (exec->addresses[15]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e98u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e99: /* move    exec->addresses[15],exec->addresses[9] */
    exec->addresses[9] = (exec->addresses[15]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e9a: /* move    #$20,exec->addresses[4] */
    exec->addresses[4] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142e9au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e9b: /* move    y:(exec->addresses[4])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e9c: /* move    y:(exec->addresses[4])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142e9cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142e9f(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142e9f: /* move    b,x:(exec->addresses[0])+       y:(exec->addresses[4])+,b */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[1]))); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142e9fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ea0: /* move    a,x:(exec->addresses[0])+       y:(exec->addresses[4])+,a */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[0]))); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142ea0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ea1(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ea1: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ea1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ea2: /* move    y:(exec->addresses[5])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142ea2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ea3: /* move    y:(exec->addresses[5])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142ea3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ea6(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ea6: /* move    b,x:(exec->addresses[1])+       y:(exec->addresses[5])+,b */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[1]))); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142ea6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ea7: /* move    a,x:(exec->addresses[1])+       y:(exec->addresses[5])+,a */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[0]))); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142ea7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ea8(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ea8: /* move    exec->addresses[0],y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->addresses[0]));
    MD_TRANSLATION_TRACE(0x142ea8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eaa: /* move    exec->addresses[1],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->addresses[1]));
    MD_TRANSLATION_TRACE(0x142eaau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eac: /* move    exec->addresses[7],y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(exec->addresses[7]));
    MD_TRANSLATION_TRACE(0x142eacu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eae: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eaeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb0: /* move    y:>$ff,exec->addresses[7] */
    exec->addresses[7] = (u24(*(md_shared_scratch_y + 255u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb2: /* move    exec->addresses[8],exec->addresses[9] */
    exec->addresses[9] = (exec->addresses[8]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb3: /* move    #$1f,exec->addresses[15] */
    exec->addresses[15] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb4: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142eb5(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142eb5: /* move    #$a0,exec->addresses[1] */
    exec->addresses[1] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb6: /* move    #$c0,exec->addresses[2] */
    exec->addresses[2] = (0xc0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb7: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb8: /* move    y:(exec->addresses[6]+$20),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 32u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x142eb8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eb9: /* move    y:(exec->addresses[6]+$21),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 33u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eb9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eba: /* move    y:(exec->addresses[6]+$1c),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ebau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ebb: /* move    x:(exec->addresses[0]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x142ebbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ebc: /* add     b,a             x:(exec->addresses[2])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ebcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ebf(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ebf: /* move    x:(exec->addresses[1])+,a       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine))); exec->lows[0] = 0u; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ebfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec0: /* add     exec->words[0],a            x:(exec->addresses[3])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ec0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec1: /* add     exec->words[0],a            exec->words[3],x:(exec->addresses[0])+ */
    exec->parallel_words[0] = exec->words[3]; add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142ec1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec2: /* sub     exec->words[2],b            b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ec2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec3: /* add     a,b             x:(exec->addresses[0]),a        a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ec3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec4: /* add     exec->words[3],a            x:(exec->addresses[2])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[3])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ec4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec5: /* mac     -exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142ec5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ec6(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ec6: /* move    exec->words[2],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x142ec6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec7: /* move    b,y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x142ec7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ec8: /* move    #>$142f33,exec->addresses[0] */
    exec->addresses[0] = (0x142f33u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ec8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eca: /* move    #$b4,exec->addresses[5] */
    exec->addresses[5] = (0xb4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ecau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ecb: /* move    y:>$fe,exec->words[0] */
    exec->words[0] = (u24(*(md_shared_scratch_y + 254u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ecbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ecd: /* move    x:(exec->addresses[0])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ecdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ece: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ecf: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ecfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ed0(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ed0: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142ed0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed1: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142ed1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed2: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ed2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed3: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142ed3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed4: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142ed4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed5: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142ed5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed6: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142ed6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed7: /* move    #$b4,exec->addresses[4] */
    exec->addresses[4] = (0xb4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ed7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ed8(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ed8: /* move    a,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x142ed8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ed9: /* move    b,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x142ed9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eda: /* move    #$a0,exec->addresses[5] */
    exec->addresses[5] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142edau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142edb: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142edbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142edc: /* mpy     -exec->words[2],exec->words[2],b        #>$7fffff,exec->words[0] */
    exec->parallel_words[0] = 0x7fffffu; mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142edcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ede: /* tfr     exec->words[0],a            #>$155555,exec->words[1] */
    exec->parallel_words[0] = 0x155555u; exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142edeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ee2(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ee2: /* add     b,a             b,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ee2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ee3: /* mpy     exec->words[1],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142ee3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ee4: /* move    b,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ee4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ee5: /* mac     exec->words[3],exec->words[2],a         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ee5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ee6: /* mpy     -exec->words[2],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142ee6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ee7: /* tfr     exec->words[0],a            a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142ee7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ee8(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ee8: /* move    #>$142f43,exec->addresses[5] */
    exec->addresses[5] = (0x142f43u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ee8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eea: /* move    #$0,exec->addresses[3] */
    exec->addresses[3] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eeau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eeb: /* move    #$64,exec->addresses[4] */
    exec->addresses[4] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eec: /* move    #$2,exec->addresses[20] */
    exec->addresses[20] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eed: /* move    y:(exec->addresses[5])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eedu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eee: /* move    exec->words[0],x:(exec->addresses[3])+      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[0]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142eeeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eef: /* move    exec->words[2],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x142eefu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef0: /* move    y:(exec->addresses[5])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ef1(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ef1: /* move    exec->words[0],x:(exec->addresses[3])+      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[0]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef2: /* move    exec->words[2],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x142ef2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef3: /* move    y:(exec->addresses[6]+$8),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef4: /* move    y:(exec->addresses[5])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef5: /* mpy     exec->words[1],exec->words[0],a         y:(exec->addresses[5])+,exec->words[0] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef6: /* mpy     exec->words[1],exec->words[0],b         y:(exec->addresses[5])+,exec->words[0] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef7: /* mpy     exec->words[1],exec->words[0],a         a,x:(exec->addresses[3])+       y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142ef7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ef8: /* mpy     exec->words[1],exec->words[2],b         b,x:(exec->addresses[3])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142ef8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142ef9(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142ef9: /* move    a,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x142ef9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142efa: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x142efau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142efb: /* move    #>$142f3b,exec->addresses[1] */
    exec->addresses[1] = (0x142f3bu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142efbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142efd: /* move    #$a0,exec->addresses[5] */
    exec->addresses[5] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142efdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142efe: /* move    #$0,exec->addresses[2] */
    exec->addresses[2] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142efeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142eff: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142effu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f00: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f01: /* mpy     exec->words[1],exec->words[0],b         y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f02(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f02: /* move    #>$142949,exec->addresses[0] */
    exec->addresses[0] = (0x142949u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f02u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f04: /* move    b,exec->addresses[16] */
    exec->addresses[16] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f07(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f07: /* move    x:(exec->addresses[0]+exec->addresses[16]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_index(exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16])), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f08: /* mpy     exec->words[1],exec->words[2],a         exec->words[1],exec->words[3] */
    exec->parallel_words[0] = exec->words[1]; mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f09: /* sub     #>$400000,a */
    sub56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142f09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f0b: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142f0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f0c: /* mpy     -exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x142f0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f0d: /* add     b,a             x:(exec->addresses[1])+,exec->words[1]      a,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f0e: /* mpy     exec->words[1],exec->words[0],b         b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142f0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f0f: /* move    x:(exec->addresses[2])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f10(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f10: /* mpy     exec->words[1],exec->words[2],a         b,exec->addresses[16] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[16] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f11: /* asl     a               y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f12: /* move    exec->words[3],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x142f12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f13: /* move    a,y:(exec->addresses[4])+exec->addresses[20] */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x142f13u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f14(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f14: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f15: /* move    #$3f,exec->addresses[1] */
    exec->addresses[1] = (0x3fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f16: /* move    #$64,exec->addresses[4] */
    exec->addresses[4] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f17: /* lua     (exec->addresses[6]+$29),exec->addresses[3] */
    exec->addresses[3] = (md_index(exec->addresses[6],exec->addresses[14],0x29u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f18: /* move    exec->addresses[15],exec->addresses[8] */
    exec->addresses[8] = (exec->addresses[15]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f19: /* move    #$80,exec->addresses[2] */
    exec->addresses[2] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f19u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f1a: /* move    #$1,exec->addresses[10] */
    exec->addresses[10] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f1b: /* move    #$2,exec->addresses[12] */
    exec->addresses[12] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f1c(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f1c: /* move    #$6,exec->addresses[20] */
    exec->addresses[20] = (0x6u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f1d: /* move    (exec->addresses[7])- */
    (void)md_agu(&exec->addresses[7],exec->addresses[15],-1,0u);
    MD_TRANSLATION_TRACE(0x142f1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f1e: /* move    x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f21(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f21: /* move    l:(exec->addresses[3])+,a */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); exec->accum[0] = sx24((int32_t)exec->parallel_words[0]); exec->lows[0] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x142f21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f22: /* move    x:(exec->addresses[3])-,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f25(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f25: /* mac     exec->words[1],exec->words[2],a         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f26: /* mac     exec->words[1],exec->words[2],a         b,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f27: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[1]),b        b,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x142f27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f28: /* add     a,b             x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f29(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f29: /* move    (exec->addresses[4])+exec->addresses[20] */
    (void)md_agu(&exec->addresses[4],exec->addresses[12],sx24((int32_t)exec->addresses[20]),0u);
    MD_TRANSLATION_TRACE(0x142f29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f2a: /* move    b,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[1]))); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f2b: /* move    a,l:(exec->addresses[3])+ */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x142f2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f2c: /* move    exec->words[1],x:(exec->addresses[3])+      b,y:(exec->addresses[7]) */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]); md_write_y(v, md_shared_scratch_y, exec->addresses[7], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x142f2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f2d: /* move    #$3f,exec->addresses[1] */
    exec->addresses[1] = (0x3fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pisd_render_block_44k1_group_142f2e(Pisd_Render_Block_44K1Exec *exec, PisdVoice *v, const int32_t *sine) {
L_142f2e: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f30: /* move    exec->addresses[12],exec->addresses[10] */
    exec->addresses[10] = (exec->addresses[12]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f31: /* move    exec->addresses[12],exec->addresses[8] */
    exec->addresses[8] = (exec->addresses[12]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x142f31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void pisd_render_block_44k1(PisdVoice *v, const int32_t *sine) {
    Pisd_Render_Block_44K1Exec execution;
    Pisd_Render_Block_44K1Exec *exec;
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
    exec->addresses[6] = (0x800u + 64u * v->virtual_voice);
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
    uint32_t loop_142db6 = 0;
    uint32_t loop_142dc5 = 0;
    uint32_t loop_142dd6 = 0;
    uint32_t loop_142e12 = 0;
    uint32_t loop_142e22 = 0;
    uint32_t loop_142e2c = 0;
    uint32_t loop_142e46 = 0;
    uint32_t loop_142e6b = 0;
    uint32_t loop_142e8a = 0;
    uint32_t loop_142e9d = 0;
    uint32_t loop_142ea4 = 0;
    uint32_t loop_142ebd = 0;
    uint32_t loop_142ee0 = 0;
    uint32_t loop_142f05 = 0;
    uint32_t loop_142f1f = 0;
    uint32_t loop_142f23 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_142d95:
    pisd_render_block_44k1_group_142d95(exec,v,sine);
L_142d9f:
    pisd_render_block_44k1_group_142d9f(exec,v,sine);
L_142da8:
    pisd_render_block_44k1_group_142da8(exec,v,sine);
L_142db1:
    pisd_render_block_44k1_group_142db1(exec,v,sine);
L_142db6: /* do      #<$10,>$142dba */
    loop_142db6 = 0x10u; if (loop_142db6 == 0u) goto L_142dba;
    MD_TRANSLATION_TRACE(0x142db6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142db8:
    pisd_render_block_44k1_group_142db8(exec,v,sine);
    if (--loop_142db6 != 0u) goto L_142db8;
L_142dba:
    pisd_render_block_44k1_group_142dba(exec,v,sine);
L_142dc4:
    pisd_render_block_44k1_group_142dc4(exec,v,sine);
L_142dc5: /* do      #<$10,>$142dce */
    loop_142dc5 = 0x10u; if (loop_142dc5 == 0u) goto L_142dce;
    MD_TRANSLATION_TRACE(0x142dc5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dc7:
    pisd_render_block_44k1_group_142dc7(exec,v,sine);
    if (--loop_142dc5 != 0u) goto L_142dc7;
L_142dce:
    pisd_render_block_44k1_group_142dce(exec,v,sine);
L_142dd6: /* do      #<$10,>$142de0 */
    loop_142dd6 = 0x10u; if (loop_142dd6 == 0u) goto L_142de0;
    MD_TRANSLATION_TRACE(0x142dd6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142dd8:
    pisd_render_block_44k1_group_142dd8(exec,v,sine);
    if (--loop_142dd6 != 0u) goto L_142dd8;
L_142de0:
    pisd_render_block_44k1_group_142de0(exec,v,sine);
L_142dea:
    pisd_render_block_44k1_group_142dea(exec,v,sine);
L_142df5:
    pisd_render_block_44k1_group_142df5(exec,v,sine);
L_142e01:
    pisd_render_block_44k1_group_142e01(exec,v,sine);
L_142e0d:
    pisd_render_block_44k1_group_142e0d(exec,v,sine);
L_142e12: /* do      #<$20,>$142e18 */
    loop_142e12 = 0x20u; if (loop_142e12 == 0u) goto L_142e18;
    MD_TRANSLATION_TRACE(0x142e12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e14:
    pisd_render_block_44k1_group_142e14(exec,v,sine);
    if (--loop_142e12 != 0u) goto L_142e14;
L_142e18:
    pisd_render_block_44k1_group_142e18(exec,v,sine);
L_142e22: /* do      #<$20,>$142e26 */
    loop_142e22 = 0x20u; if (loop_142e22 == 0u) goto L_142e26;
    MD_TRANSLATION_TRACE(0x142e22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e24:
    pisd_render_block_44k1_group_142e24(exec,v,sine);
    if (--loop_142e22 != 0u) goto L_142e24;
L_142e26:
    pisd_render_block_44k1_group_142e26(exec,v,sine);
L_142e2c: /* do      #<$10,>$142e34 */
    loop_142e2c = 0x10u; if (loop_142e2c == 0u) goto L_142e34;
    MD_TRANSLATION_TRACE(0x142e2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e2e:
    pisd_render_block_44k1_group_142e2e(exec,v,sine);
    if (--loop_142e2c != 0u) goto L_142e2e;
L_142e34:
    pisd_render_block_44k1_group_142e34(exec,v,sine);
L_142e40:
    pisd_render_block_44k1_group_142e40(exec,v,sine);
L_142e46: /* do      #<$20,>$142e4c */
    loop_142e46 = 0x20u; if (loop_142e46 == 0u) goto L_142e4c;
    MD_TRANSLATION_TRACE(0x142e46u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e48:
    pisd_render_block_44k1_group_142e48(exec,v,sine);
    if (--loop_142e46 != 0u) goto L_142e48;
L_142e4c:
    pisd_render_block_44k1_group_142e4c(exec,v,sine);
L_142e55: /* blt     func_142e59 */
    if (exec->flags[1]) goto L_142e59;
    MD_TRANSLATION_TRACE(0x142e55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e56:
    pisd_render_block_44k1_group_142e56(exec,v,sine);
L_142e59:
    pisd_render_block_44k1_group_142e59(exec,v,sine);
L_142e62:
    pisd_render_block_44k1_group_142e62(exec,v,sine);
L_142e6b: /* do      #<$20,>$142e71 */
    loop_142e6b = 0x20u; if (loop_142e6b == 0u) goto L_142e71;
    MD_TRANSLATION_TRACE(0x142e6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e6d:
    pisd_render_block_44k1_group_142e6d(exec,v,sine);
    if (--loop_142e6b != 0u) goto L_142e6d;
L_142e71:
    pisd_render_block_44k1_group_142e71(exec,v,sine);
L_142e7c:
    pisd_render_block_44k1_group_142e7c(exec,v,sine);
L_142e87:
    pisd_render_block_44k1_group_142e87(exec,v,sine);
L_142e8a: /* do      #<$20,>$142e94 */
    loop_142e8a = 0x20u; if (loop_142e8a == 0u) goto L_142e94;
    MD_TRANSLATION_TRACE(0x142e8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e8c:
    pisd_render_block_44k1_group_142e8c(exec,v,sine);
    if (--loop_142e8a != 0u) goto L_142e8c;
L_142e94:
    pisd_render_block_44k1_group_142e94(exec,v,sine);
L_142e9d: /* do      #<$10,>$142ea1 */
    loop_142e9d = 0x10u; if (loop_142e9d == 0u) goto L_142ea1;
    MD_TRANSLATION_TRACE(0x142e9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142e9f:
    pisd_render_block_44k1_group_142e9f(exec,v,sine);
    if (--loop_142e9d != 0u) goto L_142e9f;
L_142ea1:
    pisd_render_block_44k1_group_142ea1(exec,v,sine);
L_142ea4: /* do      #<$10,>$142ea8 */
    loop_142ea4 = 0x10u; if (loop_142ea4 == 0u) goto L_142ea8;
    MD_TRANSLATION_TRACE(0x142ea4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ea6:
    pisd_render_block_44k1_group_142ea6(exec,v,sine);
    if (--loop_142ea4 != 0u) goto L_142ea6;
L_142ea8:
    pisd_render_block_44k1_group_142ea8(exec,v,sine);
L_142eb5:
    pisd_render_block_44k1_group_142eb5(exec,v,sine);
L_142ebd: /* do      #<$20,>$142ec6 */
    loop_142ebd = 0x20u; if (loop_142ebd == 0u) goto L_142ec6;
    MD_TRANSLATION_TRACE(0x142ebdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ebf:
    pisd_render_block_44k1_group_142ebf(exec,v,sine);
    if (--loop_142ebd != 0u) goto L_142ebf;
L_142ec6:
    pisd_render_block_44k1_group_142ec6(exec,v,sine);
L_142ed0:
    pisd_render_block_44k1_group_142ed0(exec,v,sine);
L_142ed8:
    pisd_render_block_44k1_group_142ed8(exec,v,sine);
L_142ee0: /* do      #<$8,>$142ee8 */
    loop_142ee0 = 0x8u; if (loop_142ee0 == 0u) goto L_142ee8;
    MD_TRANSLATION_TRACE(0x142ee0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142ee2:
    pisd_render_block_44k1_group_142ee2(exec,v,sine);
    if (--loop_142ee0 != 0u) goto L_142ee2;
L_142ee8:
    pisd_render_block_44k1_group_142ee8(exec,v,sine);
L_142ef1:
    pisd_render_block_44k1_group_142ef1(exec,v,sine);
L_142ef9:
    pisd_render_block_44k1_group_142ef9(exec,v,sine);
L_142f02:
    pisd_render_block_44k1_group_142f02(exec,v,sine);
L_142f05: /* do      #<$8,>$142f14 */
    loop_142f05 = 0x8u; if (loop_142f05 == 0u) goto L_142f14;
    MD_TRANSLATION_TRACE(0x142f05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f07:
    pisd_render_block_44k1_group_142f07(exec,v,sine);
L_142f10:
    pisd_render_block_44k1_group_142f10(exec,v,sine);
    if (--loop_142f05 != 0u) goto L_142f07;
L_142f14:
    pisd_render_block_44k1_group_142f14(exec,v,sine);
L_142f1c:
    pisd_render_block_44k1_group_142f1c(exec,v,sine);
L_142f1f: /* do      #<$8,>$142f2e */
    loop_142f1f = 0x8u; if (loop_142f1f == 0u) goto L_142f2e;
    MD_TRANSLATION_TRACE(0x142f1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f21:
    pisd_render_block_44k1_group_142f21(exec,v,sine);
L_142f23: /* do      #<$20,>$142f29 */
    loop_142f23 = 0x20u; if (loop_142f23 == 0u) goto L_142f29;
    MD_TRANSLATION_TRACE(0x142f23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_142f25:
    pisd_render_block_44k1_group_142f25(exec,v,sine);
    if (--loop_142f23 != 0u) goto L_142f25;
L_142f29:
    pisd_render_block_44k1_group_142f29(exec,v,sine);
    if (--loop_142f1f != 0u) goto L_142f21;
L_142f2e:
    pisd_render_block_44k1_group_142f2e(exec,v,sine);
L_142f32: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t pisd_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_203c26: /* move.l a2, -(a7) */
    /* local register save */
C_203c28: /* move.l d[2], -(a7) */
    /* local register save */
C_203c2a: /* movea.l $c(a7), a2 */
    /* resolved argument pointer */
C_203c2e: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_203c32: /* clr.l d[0] */
    d[0] = 0u;
C_203c34: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_203c36: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_203c38: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203c3a: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_203c3c: /* moveq #$1, d[2] */
    d[2] = 0x00000001u;
C_203c3e: /* and.l d[2], d[1] */
    lhs = d[1]; rhs = d[2]; d[1] = lhs & rhs;
C_203c40: /* addi.l #$a, d[1] */
    lhs = d[1]; rhs = 0xau; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_203c46: /* move.b #$7f, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x7fu) & 0xffu);
C_203c4a: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_203c4c: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_203c52: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_203c56: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203c58: /* addi.l #$fffffc18, d[0] */
    lhs = d[0]; rhs = 0xfffffc18u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203c5e: /* move.l d[0], $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_203c62: /* move.w $2(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_203c66: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_203c68: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203c6a: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_203c70: /* move.l d[0], $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_203c74: /* move.w $4(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_203c78: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_203c7a: /* andi.l #$1fffe0, d[0] */
    lhs = d[0]; rhs = 0x1fffe0u; d[0] = lhs & rhs;
C_203c80: /* move.l d[0], $c(a2) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_203c84: /* clr.l d[0] */
    d[0] = 0u;
C_203c86: /* move.w $e(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_203c8a: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_203c8c: /* lea.l $24da14.l, a0 */
    /* resolved table pointer */
C_203c92: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((7812u + (d[0] * 4u + (0))) / 4u));
C_203c96: /* move.l d[0], $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_203c9a: /* move.w $8(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_203c9e: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_203ca0: /* andi.l #$1fffe0, d[0] */
    lhs = d[0]; rhs = 0x1fffe0u; d[0] = lhs & rhs;
C_203ca6: /* move.l d[0], $14(a2) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_203caa: /* move.w $a(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_203cae: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_203cb0: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_203cb2: /* move.l d[0], $18(a2) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_203cb6: /* move.w $c(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_203cba: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_203cbc: /* andi.l #$7fc, d[0] */
    lhs = d[0]; rhs = 0x7fcu; d[0] = lhs & rhs;
C_203cc2: /* movea.l d[0], a0 */
    a0 = d[0];
C_203cc4: /* adda.l #$24c794, a0 */
    /* resolved pointer base */
C_203cca: /* move.l $390(a0), $1c(a2) */
    *(y + (28u) / 4u) = (int32_t)(*(control_24bb90 + ((3076u + ((912u) + a0)) / 4u)));
C_203cd0: /* move.w $6(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_203cd4: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_203cd6: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_203cd8: /* move.l d[0], $20(a2) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_203cdc: /* moveq #$9, d[0] */
    d[0] = 0x00000009u;
C_203cde: /* move.l (a7)+, d[2] */
    /* local register restore */
C_203ce0: /* movea.l (a7)+, a2 */
    /* local register restore */
C_203ce2: /* rts  */
    return d[0];
}

void pisd_update_coeffs(const uint32_t *p, PisdVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = pisd_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { pisd_dsp_init(v, 0); }
}
void pisd_voice_init(PisdVoice *v) {
    uint32_t i;
    for (i = 0u; i < 64u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;
    for (i=0u;i<64u;i++) v->x_private[i] = 0; for (i=0u;i<1536u;i++) v->external[i] = 0; v->virtual_voice = 0u; v->scratch_y20_history = 0;

}
void pisd_voice_trigger(PisdVoice *v) {
    pisd_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t pisd_resample_run(PisdVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE PisdVoice
#define MD_RENDER48 pisd_render_block_48k_float
#define MD_RENDER44 pisd_render_block_44k1
#define MD_RESAMPLE_RUN pisd_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE PisdVoice
#define MD_SLOT_INIT pisd_slot_init
#define MD_SLOT_PROCESS pisd_slot_process
#define MD_VOICE_INIT pisd_voice_init
#define MD_COEFFICIENTS pisd_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER pisd_update_coeffs
#define MD_VOICE_TRIGGER pisd_voice_trigger
#define MD_RENDER44 pisd_render_block_44k1
#define MD_RENDER48 pisd_render_block_48k_float
#include "../engine_slot.inc"
#endif
