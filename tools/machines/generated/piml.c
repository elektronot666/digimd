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
} PimlVoice;
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

static uint32_t md_read_source(PimlVoice *v, uint32_t address) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) { value = md_source_words[address - 0x20000u]; }
    else if (address >= 0x200000u && address < 0xffff80u) { value = 0u; }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
extern const uint32_t source_143e0d[323];
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

static uint32_t md_read_x(PimlVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }
    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { value = u24(v->x_private[address - 0x800u - v->virtual_voice * 64u]); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1326605u && address < 1326928u) { value = *(source_143e0d + (address - 1326605u)); }
    else if (address >= 1321289u && address < 1321801u) { value = *(source_142949 + (address - 1321289u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(PimlVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }
    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { v->x_private[address - 0x800u - v->virtual_voice * 64u] = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(PimlVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }
    else if (address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u) { value = u24(*(v->y + (address - 0x800u - v->virtual_voice*64u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1326605u && address < 1326928u) { value = *(source_143e0d + (address - 1326605u)); }
    else if (address >= 1321289u && address < 1321801u) { value = *(source_142949 + (address - 1321289u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(PimlVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }
    else if (address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u) { *(v->y + (address - 0x800u - v->virtual_voice*64u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(PimlVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(PimlVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(PimlVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef PIML_DSP_INIT_EXEC_DEFINED
#define PIML_DSP_INIT_EXEC_DEFINED
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
} Piml_Dsp_InitExec;
#endif
static void piml_dsp_init_group_143e0d(Piml_Dsp_InitExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e0d: /* lua     (exec->addresses[6]+$21),exec->addresses[5] */
    exec->addresses[5] = (md_index(exec->addresses[6],exec->addresses[14],0x21u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e0e: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e0f: /* move    exec->words[0],y:(exec->addresses[6]+$3) */
    *(v->y + 3u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e10: /* move    exec->words[0],y:(exec->addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e11: /* move    exec->words[0],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e12: /* move    exec->words[0],y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e13: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e13u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e14: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_dsp_init_group_143e15(Piml_Dsp_InitExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e15: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_dsp_init_group_143e17(Piml_Dsp_InitExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e17: /* move    exec->words[0],y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->words[0]);
    MD_TRANSLATION_TRACE(0x143e17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_dsp_init_group_143e18(Piml_Dsp_InitExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e18: /* move    #>$7fffac,exec->words[0] */
    exec->words[0] = (0x7fffacu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e1a: /* move    exec->words[0],y:(exec->addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e1b: /* move    #>$135600,a */
    exec->accum[0] = sx24((int32_t)(0x135600u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x143e1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e1d: /* move    y:>$142,exec->words[0] */
    exec->words[0] = (v->virtual_voice) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e1f: /* mpyi    #>$300,exec->words[0],b */
    mpy_ss32(sx24((int32_t)(0x300u)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e21: /* move    #>$7fdf3b,exec->words[0] */
    exec->words[0] = (0x7fdf3bu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e23: /* move    b0,exec->words[1] */
    exec->words[1] = (exec->lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e24: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_dsp_init_group_143e25(Piml_Dsp_InitExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e25: /* move    exec->words[0],y:(exec->addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e26: /* move    a,y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e27: /* add     #>$200,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x200u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e29: /* move    #>$7f,exec->words[0] */
    exec->words[0] = (0x7fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e2b: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e2c: /* move    a,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e2d: /* add     #>$200,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x200u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e2f: /* move    #>$5f,exec->words[0] */
    exec->words[0] = (0x5fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_dsp_init_group_143e31(Piml_Dsp_InitExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e31: /* move    exec->words[0],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e32: /* move    a,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e32u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e33: /* move    #>$3f,exec->words[0] */
    exec->words[0] = (0x3fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e35: /* move    exec->words[0],y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e36: /* move    #>$a,exec->words[0] */
    exec->words[0] = (0xau) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e38: /* move    exec->words[0],y:(exec->addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e39: /* move    #>$1,exec->words[0] */
    exec->words[0] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e3b: /* move    exec->words[0],y:(exec->addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void piml_dsp_init(PimlVoice *v, const int32_t *sine) {
    Piml_Dsp_InitExec execution;
    Piml_Dsp_InitExec *exec;
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
L_143e0d:
    piml_dsp_init_group_143e0d(exec,v,sine);
L_143e15:
    piml_dsp_init_group_143e15(exec,v,sine);
L_143e16: /* rep     #<$18 */
    for (exec->repeat = 0x18u; exec->repeat > 0u; exec->repeat--) { md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->words[0]); } goto L_143e18;
    MD_TRANSLATION_TRACE(0x143e16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e17:
    piml_dsp_init_group_143e17(exec,v,sine);
L_143e18:
    piml_dsp_init_group_143e18(exec,v,sine);
L_143e25:
    piml_dsp_init_group_143e25(exec,v,sine);
L_143e31:
    piml_dsp_init_group_143e31(exec,v,sine);
L_143e3c: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

#ifndef PIML_DSP_TRIGGER_EXEC_DEFINED
#define PIML_DSP_TRIGGER_EXEC_DEFINED
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
} Piml_Dsp_TriggerExec;
#endif
static void piml_dsp_trigger_group_143e3d(Piml_Dsp_TriggerExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e3d: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e3e: /* move    exec->words[0],y:(exec->addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e3f: /* move    exec->words[0],y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e40: /* move    #>$143f50,exec->words[0] */
    exec->words[0] = (0x143f50u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e42: /* move    exec->words[0],y:(exec->addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e42u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e43: /* move    #>$7fffac,exec->words[0] */
    exec->words[0] = (0x7fffacu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e43u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e45: /* move    exec->words[0],y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e45u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e46: /* move    exec->words[0],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e46u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_dsp_trigger_group_143e47(Piml_Dsp_TriggerExec *exec, PimlVoice *v, const int32_t *sine) {
L_143e47: /* move    exec->words[0],y:(exec->addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x143e47u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void piml_dsp_trigger(PimlVoice *v, const int32_t *sine) {
    Piml_Dsp_TriggerExec execution;
    Piml_Dsp_TriggerExec *exec;
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
L_143e3d:
    piml_dsp_trigger_group_143e3d(exec,v,sine);
L_143e47:
    piml_dsp_trigger_group_143e47(exec,v,sine);
L_143e48: /* rts */
    return;}

#ifndef PIML_RENDER_BLOCK_44K1_EXEC_DEFINED
#define PIML_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Piml_Render_Block_44K1Exec;
#endif
static void piml_render_block_44k1_group_143e49(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e49: /* move    y:(exec->addresses[6]+$e),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e4a: /* move    y:(exec->addresses[6]+$6),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e4b: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e4c: /* move    y:(exec->addresses[6]+$5),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e4d: /* move    a,y:(exec->addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e4e: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e4eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e4f: /* add     #>$400000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e4fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e51: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e51u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e52(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e52: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e53: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e54: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e55: /* move    a,y:>$fe */
    *(md_shared_scratch_y + 254u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e57: /* asr     #$16,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 22u; exec->lows[0] = ((exec->lows[2] >> 22u) | ((uint32_t)exec->accum[2] << 2u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e58: /* move    y:(exec->addresses[6]+$c),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 12u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x143e58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e59: /* move    y:(exec->addresses[6]+$b),b0 */
    exec->lows[1] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e5a: /* move    #$0,exec->addresses[0] */
    exec->addresses[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e5b(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e5b: /* move    #$20,exec->addresses[4] */
    exec->addresses[4] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e5c: /* move    #$40,exec->addresses[2] */
    exec->addresses[2] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e5cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e5d: /* move    #>$ffffff,exec->words[2] */
    exec->words[2] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e5f: /* move    #>$7fffff,exec->words[3] */
    exec->words[3] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e61: /* move    a0,exec->words[0] */
    exec->words[0] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e62: /* move    a1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e62u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e63: /* move    y,l:(exec->addresses[2]) */
    exec->long_address[0] = exec->addresses[2]; exec->parallel_words[0] = (uint32_t)(sx24((int32_t)exec->words[3])); exec->parallel_words[3] = exec->words[2]; md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x143e63u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e66(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e66: /* add     x,b             l:(exec->addresses[2]),a */
    exec->long_address[0] = exec->addresses[2]; exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = sx24((int32_t)exec->parallel_words[0]); exec->lows[0] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x143e66u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e67: /* sub     b,a */
    sub56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e68: /* move    b,l:(exec->addresses[0])+ */
    exec->long_address[0] = md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[1]))); exec->parallel_words[3] = (exec->accum[1] > 0x7fffff ? 0xffffffu : exec->accum[1] < -0x800000 ? 0u : exec->lows[1]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x143e68u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e69: /* move    a0,y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->lows[0]);
    MD_TRANSLATION_TRACE(0x143e69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e6a(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e6a: /* move    #$0,exec->addresses[0] */
    exec->addresses[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e6au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e6b: /* move    exec->addresses[0],exec->addresses[4] */
    exec->addresses[4] = (exec->addresses[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e6c: /* cmp     #>$1449c9,b */
    exec->accum[2] = sx24((int32_t)(0x1449c9u)); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[1] == exec->accum[2] && exec->lows[1] == exec->lows[2]); exec->flags[1] = (exec->accum[1] < exec->accum[2] || (exec->accum[1] == exec->accum[2] && exec->lows[1] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x143e6cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e70(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e70: /* move    #>$1449c9,b */
    exec->accum[1] = sx24((int32_t)(0x1449c9u)); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x143e70u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e72(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e72: /* move    x:(exec->addresses[0])+,exec->addresses[1] */
    exec->addresses[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e72u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e73: /* move    #$20,exec->addresses[5] */
    exec->addresses[5] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e74: /* move    #$40,exec->addresses[3] */
    exec->addresses[3] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e74u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e75: /* move    b0,y:(exec->addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(exec->lows[1]));
    MD_TRANSLATION_TRACE(0x143e75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e76: /* move    b1,y:(exec->addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x143e76u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e77: /* move    x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e7a(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e7a: /* move    x:(exec->addresses[0])+,exec->addresses[2] */
    exec->addresses[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e7b: /* mpysu   exec->words[0],exec->words[2],b */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[2], &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e7bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e7c: /* move    x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e7d: /* macsu   exec->words[0],exec->words[2],b */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[2], &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e7e: /* move    x:(exec->addresses[2])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e7eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e7f: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x143e7fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e80: /* move    x:(exec->addresses[0])+,exec->addresses[1] */
    exec->addresses[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e81: /* mpysu   exec->words[0],exec->words[2],a */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[2], &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e82(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e82: /* move    x:(exec->addresses[2])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e82u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e83: /* macsu   exec->words[0],exec->words[2],a */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[2], &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e83u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e84: /* move    x:(exec->addresses[1])+,exec->words[0]      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e85: /* move    a,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x143e85u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e86(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e86: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e87: /* move    exec->addresses[0],exec->addresses[1] */
    exec->addresses[1] = (exec->addresses[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e87u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e88: /* move    y:(exec->addresses[6]+$3),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e89: /* move    x:(exec->addresses[0])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e8a: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e8b: /* asl     b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e8bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e8e(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e8e: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e8f: /* asl     a               b,x:(exec->addresses[1])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x143e8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e90: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e91: /* asl     b               a,x:(exec->addresses[1])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[1] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x143e91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e92(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e92: /* move    #>$7f5c29,exec->words[1] */
    exec->words[1] = (0x7f5c29u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e92u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e94: /* move    y:(exec->addresses[6]+$1e),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 30u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e94u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e95: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e95u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e96: /* move    #>$7ef9db,exec->words[1] */
    exec->words[1] = (0x7ef9dbu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e96u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e98: /* move    y:(exec->addresses[6]+$1c),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e98u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e99: /* move    a,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e9a: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e9au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e9b: /* move    #>$47ae14,exec->words[1] */
    exec->words[1] = (0x47ae14u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143e9d(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143e9d: /* move    y:(exec->addresses[6]+$1d),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 29u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143e9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e9e: /* move    a,y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143e9eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e9f: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143e9fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea0: /* move    a,y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143ea0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea1: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ea1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea2: /* move    y:(exec->addresses[6]+$8),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ea2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea3: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ea3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea4: /* move    #$60,exec->addresses[1] */
    exec->addresses[1] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ea4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ea5(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ea5: /* move    y:(exec->addresses[6]+$d),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 13u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x143ea5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea6: /* move    x:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ea6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ea9(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ea9: /* mac     exec->words[2],exec->words[0],a         a,x:(exec->addresses[1])+       a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ea9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eaa: /* mac     -exec->words[2],exec->words[0],a        x:(exec->addresses[0])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eaau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143eab(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143eab: /* move    a,y:(exec->addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143eabu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eac: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eacu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ead: /* move    y:(exec->addresses[6]+$18),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 24u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x143eadu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eae: /* move    y:(exec->addresses[6]+$19),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eaeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eaf: /* move    #>$59999a,exec->words[1] */
    exec->words[1] = (0x59999au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eafu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb1: /* move    x:(exec->addresses[0]),a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x143eb1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb2: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143eb2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143eb5(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143eb5: /* clr     a               a,x:(exec->addresses[0])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x143eb5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb6: /* sub     exec->words[2],b            b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eb6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb7: /* add     a,b             x:(exec->addresses[0]),a        a,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eb7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb8: /* move    exec->words[3],a */
    exec->accum[0] = sx24((int32_t)(exec->words[3])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x143eb8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb9: /* mac     -exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143eb9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143eba(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143eba: /* move    exec->words[2],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x143ebau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ebb: /* move    b,y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x143ebbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ebc: /* move    #>$143f38,exec->addresses[4] */
    exec->addresses[4] = (0x143f38u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ebcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ebe: /* move    #$b4,exec->addresses[5] */
    exec->addresses[5] = (0xb4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ebeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ebf: /* move    y:>$fe,exec->words[0] */
    exec->words[0] = (u24(*(md_shared_scratch_y + 254u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ebfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ec1: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ec1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ec4(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ec4: /* mpy     exec->words[2],exec->words[0],a         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ec4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ec5: /* mpy     exec->words[2],exec->words[0],b         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ec5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ec6: /* move    a,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x143ec6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ec7: /* move    b,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x143ec7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ec8(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ec8: /* move    #$b4,exec->addresses[4] */
    exec->addresses[4] = (0xb4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ec8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ec9: /* move    #$a0,exec->addresses[5] */
    exec->addresses[5] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ec9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eca: /* move    #>$7fffff,exec->words[0] */
    exec->words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ecau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ecc: /* move    #>$155555,exec->words[1] */
    exec->words[1] = (0x155555u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ece: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ecf: /* mpy     -exec->words[2],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143ecfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ed2(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ed2: /* move    exec->words[0],a */
    exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x143ed2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed3: /* add     b,a             b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ed3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed4: /* mpy     exec->words[3],exec->words[1],b         b,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ed4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed5: /* move    b,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ed5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed6: /* mac     exec->words[3],exec->words[2],a         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ed6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed7: /* mpy     -exec->words[2],exec->words[2],b        a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x143ed7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ed8(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ed8: /* move    #$64,exec->addresses[4] */
    exec->addresses[4] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ed8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed9: /* move    #$2,exec->addresses[20] */
    exec->addresses[20] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ed9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eda: /* move    #>$143f48,exec->addresses[2] */
    exec->addresses[2] = (0x143f48u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143edau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143edc: /* move    #$e8,exec->addresses[1] */
    exec->addresses[1] = (0xe8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143edcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143edd: /* move    y:(exec->addresses[6]+$7),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ee0(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ee0: /* move    x:(exec->addresses[2])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ee0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ee1: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143ee1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ee2: /* move    a,x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x143ee2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ee3(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ee3: /* move    #>$143f40,exec->addresses[2] */
    exec->addresses[2] = (0x143f40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ee3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ee5: /* move    #$e0,exec->addresses[1] */
    exec->addresses[1] = (0xe0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ee5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ee6: /* move    y:(exec->addresses[6]+$4),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ee6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ee9(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ee9: /* move    x:(exec->addresses[2])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ee9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eea: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143eeau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eeb: /* move    a,x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x143eebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143eec(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143eec: /* move    #$e0,exec->addresses[1] */
    exec->addresses[1] = (0xe0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eecu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eed: /* move    #$e8,exec->addresses[2] */
    exec->addresses[2] = (0xe8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eedu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eee: /* move    #$a0,exec->addresses[5] */
    exec->addresses[5] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eeeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eef: /* move    #>$142949,exec->addresses[0] */
    exec->addresses[0] = (0x142949u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143eefu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ef1: /* move    y:(exec->addresses[6]+$9),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ef1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ef2: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ef2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ef3: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143ef3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ef4: /* move    y:(exec->addresses[5])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ef4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ef5(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ef5: /* move    b,exec->addresses[16] */
    exec->addresses[16] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ef5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143ef8(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143ef8: /* move    x:(exec->addresses[0]+exec->addresses[16]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_index(exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16])), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ef8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ef9: /* mpy     exec->words[1],exec->words[2],a         exec->words[1],exec->words[3] */
    exec->parallel_words[0] = exec->words[1]; mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143ef9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143efa: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143efau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143efb: /* add     #>$800000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x800000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143efbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143efd: /* mpy     -exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143efdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143efe: /* add     b,a             x:(exec->addresses[1])+,exec->words[1]      a,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143efeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eff: /* mpy     exec->words[1],exec->words[0],b         b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x143effu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f00: /* move    x:(exec->addresses[2])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f00u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f01(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f01: /* mpy     exec->words[1],exec->words[2],a         b,exec->addresses[16] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[16] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f02: /* asl     a               y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f02u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f03: /* move    exec->words[3],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x143f03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f04: /* move    a,y:(exec->addresses[4])+exec->addresses[20] */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x143f04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f05(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f05: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f06: /* move    #$40,exec->addresses[1] */
    exec->addresses[1] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f06u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f07: /* move    #$64,exec->addresses[4] */
    exec->addresses[4] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f08: /* lua     (exec->addresses[6]+$21),exec->addresses[3] */
    exec->addresses[3] = (md_index(exec->addresses[6],exec->addresses[14],0x21u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f09: /* move    #$80,exec->addresses[2] */
    exec->addresses[2] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f0a: /* move    #$1,exec->addresses[10] */
    exec->addresses[10] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f0au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f0b: /* move    #$2,exec->addresses[12] */
    exec->addresses[12] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f0e(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f0e: /* move    y:(exec->addresses[3])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x143f0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f0f: /* move    y:(exec->addresses[3])+,a0 */
    exec->lows[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f10: /* move    y:(exec->addresses[3])+,exec->words[1] */
    exec->words[1] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f11: /* move    x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f12: /* mac     exec->words[1],exec->words[2],a         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f13: /* mac     exec->words[1],exec->words[2],a         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f13u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f14: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[1]),b        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f17(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f17: /* add     a,b             x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f18: /* mac     exec->words[1],exec->words[2],a         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f19: /* mac     exec->words[1],exec->words[2],a         b,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f19u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f1a: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[1]),b        y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f1b(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f1b: /* add     a,b */
    add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143f1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f1c: /* lua     (exec->addresses[3]-$3),exec->addresses[3] */
    exec->addresses[3] = (md_index(exec->addresses[3],exec->addresses[11],-0x3u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f1d: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f1e: /* move    b,x:(exec->addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x143f1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f1f: /* move    #$40,exec->addresses[1] */
    exec->addresses[1] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f20: /* lua     (exec->addresses[4]+$6),exec->addresses[4] */
    exec->addresses[4] = (md_index(exec->addresses[4],exec->addresses[12],0x6u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f21: /* move    a,y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x143f21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f22: /* move    a0,y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->lows[0]);
    MD_TRANSLATION_TRACE(0x143f22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f23(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f23: /* move    exec->words[1],y:(exec->addresses[3])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x143f23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f24(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f24: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f26: /* move    #>$ffffff,exec->addresses[10] */
    exec->addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f28: /* move    y:(exec->addresses[6]+$a),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f29: /* move    y:(exec->addresses[6]+$2),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f2a: /* mpy     exec->words[3],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143f2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f2b: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f2c: /* move    a,y:(exec->addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x143f2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f2d: /* mpyi    #>$400000,exec->words[2],b */
    mpy_ss32(sx24((int32_t)(0x400000u)), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x143f2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f2f(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f2f: /* move    b,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f30: /* move    x:(exec->addresses[0])+,exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f31: /* mpy     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f32: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x143f32u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void piml_render_block_44k1_group_143f35(Piml_Render_Block_44K1Exec *exec, PimlVoice *v, const int32_t *sine) {
L_143f35: /* mpy     exec->words[2],exec->words[0],a         x:(exec->addresses[0])+,exec->words[0]      a,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x143f35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f36: /* mpy     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[0]      b,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, exec->addresses[7]++, sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x143f36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void piml_render_block_44k1(PimlVoice *v, const int32_t *sine) {
    Piml_Render_Block_44K1Exec execution;
    Piml_Render_Block_44K1Exec *exec;
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
    uint32_t loop_143e64 = 0;
    uint32_t loop_143e78 = 0;
    uint32_t loop_143e8c = 0;
    uint32_t loop_143ea7 = 0;
    uint32_t loop_143eb3 = 0;
    uint32_t loop_143ec2 = 0;
    uint32_t loop_143ed0 = 0;
    uint32_t loop_143ede = 0;
    uint32_t loop_143ee7 = 0;
    uint32_t loop_143ef6 = 0;
    uint32_t loop_143f0c = 0;
    uint32_t loop_143f15 = 0;
    uint32_t loop_143f33 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_143e49:
    piml_render_block_44k1_group_143e49(exec,v,sine);
L_143e52:
    piml_render_block_44k1_group_143e52(exec,v,sine);
L_143e5b:
    piml_render_block_44k1_group_143e5b(exec,v,sine);
L_143e64: /* do      #<$20,>$143e6a */
    loop_143e64 = 0x20u; if (loop_143e64 == 0u) goto L_143e6a;
    MD_TRANSLATION_TRACE(0x143e64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e66:
    piml_render_block_44k1_group_143e66(exec,v,sine);
    if (--loop_143e64 != 0u) goto L_143e66;
L_143e6a:
    piml_render_block_44k1_group_143e6a(exec,v,sine);
L_143e6e: /* blt     func_143e72 */
    if (exec->flags[1]) goto L_143e72;
    MD_TRANSLATION_TRACE(0x143e6eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e70:
    piml_render_block_44k1_group_143e70(exec,v,sine);
L_143e72:
    piml_render_block_44k1_group_143e72(exec,v,sine);
L_143e78: /* do      #<$10,>$143e86 */
    loop_143e78 = 0x10u; if (loop_143e78 == 0u) goto L_143e86;
    MD_TRANSLATION_TRACE(0x143e78u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e7a:
    piml_render_block_44k1_group_143e7a(exec,v,sine);
L_143e82:
    piml_render_block_44k1_group_143e82(exec,v,sine);
    if (--loop_143e78 != 0u) goto L_143e7a;
L_143e86:
    piml_render_block_44k1_group_143e86(exec,v,sine);
L_143e8c: /* do      #<$10,>$143e92 */
    loop_143e8c = 0x10u; if (loop_143e8c == 0u) goto L_143e92;
    MD_TRANSLATION_TRACE(0x143e8cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143e8e:
    piml_render_block_44k1_group_143e8e(exec,v,sine);
    if (--loop_143e8c != 0u) goto L_143e8e;
L_143e92:
    piml_render_block_44k1_group_143e92(exec,v,sine);
L_143e9d:
    piml_render_block_44k1_group_143e9d(exec,v,sine);
L_143ea5:
    piml_render_block_44k1_group_143ea5(exec,v,sine);
L_143ea7: /* do      #<$20,>$143eab */
    loop_143ea7 = 0x20u; if (loop_143ea7 == 0u) goto L_143eab;
    MD_TRANSLATION_TRACE(0x143ea7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ea9:
    piml_render_block_44k1_group_143ea9(exec,v,sine);
    if (--loop_143ea7 != 0u) goto L_143ea9;
L_143eab:
    piml_render_block_44k1_group_143eab(exec,v,sine);
L_143eb3: /* do      #<$20,>$143eba */
    loop_143eb3 = 0x20u; if (loop_143eb3 == 0u) goto L_143eba;
    MD_TRANSLATION_TRACE(0x143eb3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143eb5:
    piml_render_block_44k1_group_143eb5(exec,v,sine);
    if (--loop_143eb3 != 0u) goto L_143eb5;
L_143eba:
    piml_render_block_44k1_group_143eba(exec,v,sine);
L_143ec2: /* do      #<$4,>$143ec8 */
    loop_143ec2 = 0x4u; if (loop_143ec2 == 0u) goto L_143ec8;
    MD_TRANSLATION_TRACE(0x143ec2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ec4:
    piml_render_block_44k1_group_143ec4(exec,v,sine);
    if (--loop_143ec2 != 0u) goto L_143ec4;
L_143ec8:
    piml_render_block_44k1_group_143ec8(exec,v,sine);
L_143ed0: /* do      #<$8,>$143ed8 */
    loop_143ed0 = 0x8u; if (loop_143ed0 == 0u) goto L_143ed8;
    MD_TRANSLATION_TRACE(0x143ed0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ed2:
    piml_render_block_44k1_group_143ed2(exec,v,sine);
    if (--loop_143ed0 != 0u) goto L_143ed2;
L_143ed8:
    piml_render_block_44k1_group_143ed8(exec,v,sine);
L_143ede: /* do      #<$8,>$143ee3 */
    loop_143ede = 0x8u; if (loop_143ede == 0u) goto L_143ee3;
    MD_TRANSLATION_TRACE(0x143edeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ee0:
    piml_render_block_44k1_group_143ee0(exec,v,sine);
    if (--loop_143ede != 0u) goto L_143ee0;
L_143ee3:
    piml_render_block_44k1_group_143ee3(exec,v,sine);
L_143ee7: /* do      #<$8,>$143eec */
    loop_143ee7 = 0x8u; if (loop_143ee7 == 0u) goto L_143eec;
    MD_TRANSLATION_TRACE(0x143ee7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ee9:
    piml_render_block_44k1_group_143ee9(exec,v,sine);
    if (--loop_143ee7 != 0u) goto L_143ee9;
L_143eec:
    piml_render_block_44k1_group_143eec(exec,v,sine);
L_143ef5:
    piml_render_block_44k1_group_143ef5(exec,v,sine);
L_143ef6: /* do      #<$8,>$143f05 */
    loop_143ef6 = 0x8u; if (loop_143ef6 == 0u) goto L_143f05;
    MD_TRANSLATION_TRACE(0x143ef6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143ef8:
    piml_render_block_44k1_group_143ef8(exec,v,sine);
L_143f01:
    piml_render_block_44k1_group_143f01(exec,v,sine);
    if (--loop_143ef6 != 0u) goto L_143ef8;
L_143f05:
    piml_render_block_44k1_group_143f05(exec,v,sine);
L_143f0c: /* do      #<$8,>$143f24 */
    loop_143f0c = 0x8u; if (loop_143f0c == 0u) goto L_143f24;
    MD_TRANSLATION_TRACE(0x143f0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f0e:
    piml_render_block_44k1_group_143f0e(exec,v,sine);
L_143f15: /* do      #<$1f,>$143f1b */
    loop_143f15 = 0x1fu; if (loop_143f15 == 0u) goto L_143f1b;
    MD_TRANSLATION_TRACE(0x143f15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f17:
    piml_render_block_44k1_group_143f17(exec,v,sine);
    if (--loop_143f15 != 0u) goto L_143f17;
L_143f1b:
    piml_render_block_44k1_group_143f1b(exec,v,sine);
L_143f23:
    piml_render_block_44k1_group_143f23(exec,v,sine);
    if (--loop_143f0c != 0u) goto L_143f0e;
L_143f24:
    piml_render_block_44k1_group_143f24(exec,v,sine);
L_143f2f:
    piml_render_block_44k1_group_143f2f(exec,v,sine);
L_143f33: /* do      #<$10,>$143f37 */
    loop_143f33 = 0x10u; if (loop_143f33 == 0u) goto L_143f37;
    MD_TRANSLATION_TRACE(0x143f33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_143f35:
    piml_render_block_44k1_group_143f35(exec,v,sine);
    if (--loop_143f33 != 0u) goto L_143f35;
L_143f37: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t piml_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_203dc8: /* move.l a2, -(a7) */
    /* local register save */
C_203dca: /* move.l d[2], -(a7) */
    /* local register save */
C_203dcc: /* movea.l $c(a7), a2 */
    /* resolved argument pointer */
C_203dd0: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_203dd4: /* clr.l d[0] */
    d[0] = 0u;
C_203dd6: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_203dd8: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_203dda: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203ddc: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_203dde: /* moveq #$1, d[2] */
    d[2] = 0x00000001u;
C_203de0: /* and.l d[2], d[1] */
    lhs = d[1]; rhs = d[2]; d[1] = lhs & rhs;
C_203de2: /* addi.l #$a, d[1] */
    lhs = d[1]; rhs = 0xau; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_203de8: /* move.b #$7f, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x7fu) & 0xffu);
C_203dec: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_203dee: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_203df4: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_203df8: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203dfa: /* addi.l #$fffffc18, d[0] */
    lhs = d[0]; rhs = 0xfffffc18u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203e00: /* move.l d[0], $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_203e04: /* clr.l d[1] */
    d[1] = 0u;
C_203e06: /* move.w $2(a1), d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_203e0a: /* move.l d[1], d[0] */
    d[0] = d[1];
C_203e0c: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_203e0e: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_203e10: /* movea.l d[0], a0 */
    a0 = d[0];
C_203e12: /* adda.l #$24c794, a0 */
    /* resolved pointer base */
C_203e18: /* move.l $460(a0), $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(*(control_24bb90 + ((3076u + ((1120u) + a0)) / 4u)));
C_203e1e: /* moveq #$9, d[0] */
    d[0] = 0x00000009u;
C_203e20: /* lsl.l d[0], d[1] */
    d[1] = (((d[0]) & 63u) < 32u ? d[1] << ((d[0]) & 63u) : 0u);
C_203e22: /* move.l d[1], $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(d[1]);
C_203e26: /* move.w $4(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_203e2a: /* mulu.w #$190, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x190u);
C_203e2e: /* addi.l #$f4240, d[0] */
    lhs = d[0]; rhs = 0xf4240u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203e34: /* move.l d[0], $c(a2) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_203e38: /* move.l d[0], $1c(a2) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_203e3c: /* clr.l d[0] */
    d[0] = 0u;
C_203e3e: /* move.w $6(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_203e42: /* cmpi.l #$1fff, d[0] */
    lhs = d[0]; rhs = 0x1fffu;
C_203e48: /* bhi.b $203e5e */
    if (lhs > rhs) goto C_203e5e;
C_203e4a: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_203e4c: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203e4e: /* move.l d[0], $14(a2) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_203e52: /* move.l $24cc44.l, d[2] */
    d[2] = *(control_24bb90 + ((4276u + (0u)) / 4u));
C_203e58: /* move.l d[2], $18(a2) */
    *(y + (24u) / 4u) = (int32_t)(d[2]);
C_203e5c: /* bra.b $203e80 */
    goto C_203e80;
C_203e5e: /* move.l #$400200, d[1] */
    d[1] = 0x400200u;
C_203e64: /* move.l d[1], $14(a2) */
    *(y + (20u) / 4u) = (int32_t)(d[1]);
C_203e68: /* addi.l #$ffffe000, d[0] */
    lhs = d[0]; rhs = 0xffffe000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203e6e: /* asr.l #$7, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_203e70: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_203e72: /* movea.l d[0], a0 */
    a0 = d[0];
C_203e74: /* adda.l #$24c794, a0 */
    /* resolved pointer base */
C_203e7a: /* move.l $4b0(a0), $18(a2) */
    *(y + (24u) / 4u) = (int32_t)(*(control_24bb90 + ((3076u + ((1200u) + a0)) / 4u)));
C_203e80: /* moveq #$8, d[0] */
    d[0] = 0x00000008u;
C_203e82: /* move.l (a7)+, d[2] */
    /* local register restore */
C_203e84: /* movea.l (a7)+, a2 */
    /* local register restore */
C_203e86: /* rts  */
    return d[0];
}

void piml_update_coeffs(const uint32_t *p, PimlVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = piml_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { piml_dsp_init(v, 0); }
}
void piml_voice_init(PimlVoice *v) {
    uint32_t i;
    for (i = 0u; i < 64u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;
    for (i=0u;i<64u;i++) v->x_private[i] = 0; for (i=0u;i<1536u;i++) v->external[i] = 0; v->virtual_voice = 0u; v->scratch_y20_history = 0;

}
void piml_voice_trigger(PimlVoice *v) {
    piml_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t piml_resample_run(PimlVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE PimlVoice
#define MD_RENDER48 piml_render_block_48k_float
#define MD_RENDER44 piml_render_block_44k1
#define MD_RESAMPLE_RUN piml_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE PimlVoice
#define MD_SLOT_INIT piml_slot_init
#define MD_SLOT_PROCESS piml_slot_process
#define MD_VOICE_INIT piml_voice_init
#define MD_COEFFICIENTS piml_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER piml_update_coeffs
#define MD_VOICE_TRIGGER piml_voice_trigger
#define MD_RENDER44 piml_render_block_44k1
#define MD_RENDER48 piml_render_block_48k_float
#include "../engine_slot.inc"
#endif
