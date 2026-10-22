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
} PirsVoice;
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

static uint32_t md_read_source(PirsVoice *v, uint32_t address) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) { value = md_source_words[address - 0x20000u]; }
    else if (address >= 0x200000u && address < 0xffff80u) { value = 0u; }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
extern const uint32_t source_142b49[512];
extern const uint32_t source_144ab9[505];
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

static uint32_t md_read_x(PirsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }
    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { value = u24(v->x_private[address - 0x800u - v->virtual_voice * 64u]); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1321801u && address < 1322313u) { value = *(source_142b49 + (address - 1321801u)); }
    else if (address >= 1329849u && address < 1330354u) { value = *(source_144ab9 + (address - 1329849u)); }
    else if (address >= 1321289u && address < 1321801u) { value = *(source_142949 + (address - 1321289u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(PirsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }
    else if (address >= 0x800u + v->virtual_voice * 64u && address < 0x840u + v->virtual_voice * 64u) { v->x_private[address - 0x800u - v->virtual_voice * 64u] = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(PirsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { value = u24(v->external[address - 0x135600u - v->virtual_voice * 1536u]); }
    else if (address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u) { value = u24(*(v->y + (address - 0x800u - v->virtual_voice*64u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1321801u && address < 1322313u) { value = *(source_142b49 + (address - 1321801u)); }
    else if (address >= 1329849u && address < 1330354u) { value = *(source_144ab9 + (address - 1329849u)); }
    else if (address >= 1321289u && address < 1321801u) { value = *(source_142949 + (address - 1321289u)); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(PirsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x135600u + v->virtual_voice * 1536u && address < 0x135c00u + v->virtual_voice * 1536u) { v->external[address - 0x135600u - v->virtual_voice * 1536u] = sx24((int32_t)value); }
    else if (address >= 0x800u + v->virtual_voice*64u && address < 0x840u + v->virtual_voice*64u) { *(v->y + (address - 0x800u - v->virtual_voice*64u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(PirsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(PirsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(PirsVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
#ifndef PIRS_DSP_INIT_EXEC_DEFINED
#define PIRS_DSP_INIT_EXEC_DEFINED
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
} Pirs_Dsp_InitExec;
#endif
static void pirs_dsp_init_group_144ab9(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144ab9: /* lua     (exec->addresses[6]+$2a),exec->addresses[3] */
    exec->addresses[3] = (md_index(exec->addresses[6],exec->addresses[14],0x2au)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ab9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144aba: /* lua     (exec->addresses[6]+$1d),exec->addresses[4] */
    exec->addresses[4] = (md_index(exec->addresses[6],exec->addresses[14],0x1du)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144abau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144abb: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144abbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144abc: /* move    exec->words[0],exec->words[1] */
    exec->words[1] = (exec->words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144abcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144abd: /* move    exec->words[0],y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144abdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_init_group_144ac0(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144ac0: /* move    x,l:(exec->addresses[3])+ */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = (uint32_t)(sx24((int32_t)exec->words[1])); exec->parallel_words[3] = exec->words[0]; md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x144ac0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_init_group_144ac2(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144ac2: /* move    exec->words[0],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[0]);
    MD_TRANSLATION_TRACE(0x144ac2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_init_group_144ac3(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144ac3: /* move    #>$135600,a */
    exec->accum[0] = sx24((int32_t)(0x135600u)); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144ac3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ac5: /* move    y:>$142,exec->words[0] */
    exec->words[0] = (v->virtual_voice) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ac5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ac7: /* mpyi    #>$300,exec->words[0],b */
    mpy_ss32(sx24((int32_t)(0x300u)), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144ac7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ac9: /* move    b0,exec->words[1] */
    exec->words[1] = (exec->lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ac9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144aca: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144acau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144acb: /* move    #>$7ffcb9,exec->words[0] */
    exec->words[0] = (0x7ffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144acbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144acd: /* move    exec->words[0],y:(exec->addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144acdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144acf: /* move    exec->words[0],y:(exec->addresses[6]+$6) */
    *(v->y + 6u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144acfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_init_group_144ad1(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144ad1: /* move    a,y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144ad1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ad3: /* add     #>$200,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x200u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144ad3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ad5: /* move    #>$10b,exec->words[0] */
    exec->words[0] = (0x10bu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ad5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ad7: /* move    exec->words[0],y:(exec->addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144ad7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ad9: /* move    a,y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144ad9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144adb: /* add     #>$200,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x200u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144adbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144add: /* move    #>$ff,exec->words[0] */
    exec->words[0] = (0xffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144addu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144adf: /* move    exec->words[0],y:(exec->addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144adfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_init_group_144ae1(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144ae1: /* move    a,y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144ae1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ae3: /* move    #>$b5,exec->words[0] */
    exec->words[0] = (0xb5u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ae3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ae5: /* move    exec->words[0],y:(exec->addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144ae5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ae7: /* move    #>$ccccd,exec->words[0] */
    exec->words[0] = (0xccccdu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ae7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ae9: /* move    exec->words[0],y:(exec->addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144ae9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144aeb: /* move    #>$a,exec->words[0] */
    exec->words[0] = (0xau) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144aebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144aed: /* move    exec->words[0],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144aedu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144aef: /* move    #>$1,exec->words[0] */
    exec->words[0] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144aefu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_init_group_144af1(Pirs_Dsp_InitExec *exec, PirsVoice *v, const int32_t *sine) {
L_144af1: /* move    exec->words[0],y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144af1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void pirs_dsp_init(PirsVoice *v, const int32_t *sine) {
    Pirs_Dsp_InitExec execution;
    Pirs_Dsp_InitExec *exec;
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
L_144ab9:
    pirs_dsp_init_group_144ab9(exec,v,sine);
L_144abf: /* rep     #<$10 */
    for (exec->repeat = 0x10u; exec->repeat > 0u; exec->repeat--) { exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = (uint32_t)(sx24((int32_t)exec->words[1])); exec->parallel_words[3] = exec->words[0]; md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]); } goto L_144ac1;
    MD_TRANSLATION_TRACE(0x144abfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ac0:
    pirs_dsp_init_group_144ac0(exec,v,sine);
L_144ac1: /* rep     #<$5 */
    for (exec->repeat = 0x5u; exec->repeat > 0u; exec->repeat--) { md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[0]); } goto L_144ac3;
    MD_TRANSLATION_TRACE(0x144ac1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ac2:
    pirs_dsp_init_group_144ac2(exec,v,sine);
L_144ac3:
    pirs_dsp_init_group_144ac3(exec,v,sine);
L_144ad1:
    pirs_dsp_init_group_144ad1(exec,v,sine);
L_144ae1:
    pirs_dsp_init_group_144ae1(exec,v,sine);
L_144af1:
    pirs_dsp_init_group_144af1(exec,v,sine);
L_144af3: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

#ifndef PIRS_DSP_TRIGGER_EXEC_DEFINED
#define PIRS_DSP_TRIGGER_EXEC_DEFINED
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
} Pirs_Dsp_TriggerExec;
#endif
static void pirs_dsp_trigger_group_144af4(Pirs_Dsp_TriggerExec *exec, PirsVoice *v, const int32_t *sine) {
L_144af4: /* move    #$0,exec->words[0] */
    exec->words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144af4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144af5: /* move    exec->words[0],y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144af5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144af6: /* move    exec->words[0],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144af6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144af7: /* move    #>$144cb2,exec->words[0] */
    exec->words[0] = (0x144cb2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144af7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144af9: /* move    exec->words[0],y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144af9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144afa: /* move    #>$7fffff,exec->words[0] */
    exec->words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144afau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144afc: /* move    exec->words[0],y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144afcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144afd: /* move    exec->words[0],y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144afdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_dsp_trigger_group_144afe(Pirs_Dsp_TriggerExec *exec, PirsVoice *v, const int32_t *sine) {
L_144afe: /* move    exec->words[0],y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144afeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144aff: /* move    exec->words[0],y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144affu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void pirs_dsp_trigger(PirsVoice *v, const int32_t *sine) {
    Pirs_Dsp_TriggerExec execution;
    Pirs_Dsp_TriggerExec *exec;
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
L_144af4:
    pirs_dsp_trigger_group_144af4(exec,v,sine);
L_144afe:
    pirs_dsp_trigger_group_144afe(exec,v,sine);
L_144b00: /* rts */
    return;}

#ifndef PIRS_RENDER_BLOCK_44K1_EXEC_DEFINED
#define PIRS_RENDER_BLOCK_44K1_EXEC_DEFINED
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
} Pirs_Render_Block_44K1Exec;
#endif
static void pirs_render_block_44k1_group_144b01(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b01: /* move    y:(exec->addresses[6]+$13),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b02: /* move    y:(exec->addresses[6]+$f),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b02u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b03: /* move    #>$7f7cee,exec->words[3] */
    exec->words[3] = (0x7f7ceeu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b05: /* mpy     exec->words[3],exec->words[1],b         #>$7c28f6,exec->words[2] */
    exec->parallel_words[0] = 0x7c28f6u; mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b07: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b08: /* move    b,y:(exec->addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x144b08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b09: /* move    a,y:(exec->addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144b09u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b0a: /* move    a,exec->words[0] */
    exec->words[0] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b0au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b0b(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b0b: /* move    b,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b0c: /* move    y:(exec->addresses[6]+$3),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b0d: /* move    y:(exec->addresses[6]+$5),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b0du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b0e: /* mpy     exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b0f: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b10: /* add     b,a */
    add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b11: /* add     #>$400000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x400000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b11u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b13: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b13u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b14(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b14: /* move    #$40,exec->words[2] */
    exec->words[2] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b15: /* mpy     exec->words[2],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b15u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b16: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b17: /* move    a,y:>$fe */
    *(md_shared_scratch_y + 254u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144b17u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b19: /* asr     #$16,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = exec->accum[2] >> 22u; exec->lows[0] = ((exec->lows[2] >> 22u) | ((uint32_t)exec->accum[2] << 2u)) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b19u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b1a: /* move    y:(exec->addresses[6]+$11),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 17u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144b1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b1b: /* move    y:(exec->addresses[6]+$10),b0 */
    exec->lows[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b1c: /* move    a0,exec->words[0] */
    exec->words[0] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b1d(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b1d: /* asl     a               a1,exec->words[1] */
    exec->parallel_words[0] = u24(exec->accum[0]); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b1e: /* move    #$0,exec->addresses[0] */
    exec->addresses[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b1f: /* move    a0,exec->words[2] */
    exec->words[2] = (exec->lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b20: /* tfr     b,a             a1,exec->words[3] */
    exec->parallel_words[0] = u24(exec->accum[0]); exec->accum[0] = exec->accum[1]; exec->lows[0] = exec->lows[1]; exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b21: /* add     x,b */
    add56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[1]), exec->words[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b24(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b24: /* add     y,a             a,l:(exec->addresses[0])+ */
    exec->long_address[0] = md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); add56(exec->accum[0], exec->lows[0], sx24((int32_t)exec->words[3]), exec->words[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x144b24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b25: /* add     y,b             b,l:(exec->addresses[0])+ */
    exec->long_address[0] = md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[1]))); exec->parallel_words[3] = (exec->accum[1] > 0x7fffff ? 0xffffffu : exec->accum[1] < -0x800000 ? 0u : exec->lows[1]); add56(exec->accum[1], exec->lows[1], sx24((int32_t)exec->words[3]), exec->words[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x144b25u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b26(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b26: /* move    #>$144efc,exec->words[0] */
    exec->words[0] = (0x144efcu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b28: /* cmp     exec->words[0],a */
    exec->accum[2] = sx24((int32_t)(exec->words[0])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x144b28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b29: /* tge     exec->words[0],a */
    if (!exec->flags[1]) { exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; }
    MD_TRANSLATION_TRACE(0x144b29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b2a: /* move    a0,y:(exec->addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(exec->lows[0]));
    MD_TRANSLATION_TRACE(0x144b2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b2b: /* move    a1,y:(exec->addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x144b2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b2c: /* move    #$0,exec->addresses[1] */
    exec->addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b2d: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b2e: /* move    #$3f,exec->addresses[5] */
    exec->addresses[5] = (0x3fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b2f(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b2f: /* move    x:(exec->addresses[1])+,exec->addresses[2] */
    exec->addresses[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b2fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b30: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b31: /* move    x:(exec->addresses[2])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b32: /* move    x:(exec->addresses[1])+,exec->addresses[0] */
    exec->addresses[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b32u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b35(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b35: /* mpysu   -exec->words[1],exec->words[2],a */
    mpy_su32(sx24((int32_t)(exec->words[1])), exec->words[2], &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b36: /* add     exec->words[1],a            x:(exec->addresses[2]),exec->words[0]       b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144b36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b37: /* add     exec->words[1],a            x:(exec->addresses[1])+,exec->addresses[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b37u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b38: /* macsu   exec->words[0],exec->words[2],a */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[2], &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b39: /* move    x:(exec->addresses[0])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b3a: /* mpysu   -exec->words[1],exec->words[2],b */
    mpy_su32(sx24((int32_t)(exec->words[1])), exec->words[2], &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b3b: /* add     exec->words[1],b            x:(exec->addresses[0]),exec->words[0]       a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144b3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b3c: /* add     exec->words[1],b            x:(exec->addresses[1])+,exec->addresses[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b3cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b3d(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b3d: /* macsu   exec->words[0],exec->words[2],b */
    mpy_su32(sx24((int32_t)(exec->words[0])), exec->words[2], &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b3e: /* move    x:(exec->addresses[2])+,exec->words[1]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b3f(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b3f: /* move    b,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x144b3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b40: /* move    #$40,exec->addresses[4] */
    exec->addresses[4] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b41: /* move    exec->addresses[4],exec->addresses[1] */
    exec->addresses[1] = (exec->addresses[4]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b41u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b42: /* move    y:(exec->addresses[6]+$3),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b42u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b43: /* move    y:(exec->addresses[4])+,exec->words[3] */
    exec->words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b43u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b44: /* mpy     exec->words[0],exec->words[3],b         y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b44u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b45: /* asl     #$4,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 4u) | (exec->lows[2] >> 20u)); exec->lows[1] = (exec->lows[2] << 4u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b45u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b46: /* mpy     exec->words[0],exec->words[3],a         y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b46u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b49(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b49: /* asl     #$4,a,a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 4u) | (exec->lows[2] >> 20u)); exec->lows[0] = (exec->lows[2] << 4u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b4a: /* mpy     exec->words[0],exec->words[3],b         b,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b4b: /* asl     #$4,b,b */
    exec->accum[2] = exec->accum[1]; exec->lows[2] = exec->lows[1]; exec->accum[1] = (int32_t)(((uint32_t)exec->accum[2] << 4u) | (exec->lows[2] >> 20u)); exec->lows[1] = (exec->lows[2] << 4u) & 0xffffffu; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b4c: /* mpy     exec->words[0],exec->words[3],a         a,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b4d(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b4d: /* move    #>$7f5c29,exec->words[1] */
    exec->words[1] = (0x7f5c29u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b4f: /* move    y:(exec->addresses[6]+$26),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 38u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b4fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b50: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b51: /* move    y:(exec->addresses[6]+$7),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b51u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b52: /* move    y:(exec->addresses[6]+$24),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b53: /* move    a,y:(exec->addresses[6]+$26) */
    *(v->y + 38u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144b53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b54: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b55: /* move    #>$47ae14,exec->words[1] */
    exec->words[1] = (0x47ae14u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b57(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b57: /* move    y:(exec->addresses[6]+$25),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 37u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b58: /* move    a,y:(exec->addresses[6]+$24) */
    *(v->y + 36u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144b58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b59: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b5a: /* move    a,y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144b5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b5b: /* move    #>$142b49,exec->addresses[0] */
    exec->addresses[0] = (0x142b49u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b5d: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b5e: /* move    #>$64,exec->words[1] */
    exec->words[1] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b5eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b60: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b60u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b61(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b61: /* move    #>$5a827a,exec->words[0] */
    exec->words[0] = (0x5a827au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b63: /* move    b,exec->addresses[16] */
    exec->addresses[16] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b63u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b64: /* move    y:(exec->addresses[6]+$26),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 38u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b65: /* move    #$40,a */
    exec->accum[0] = sx24((int32_t)(((0x40u) << 16))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144b65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b66: /* maci    #>$d9999a,exec->words[2],a */
    mpy_ss32(sx24((int32_t)(0xd9999au)), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b66u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b68: /* move    x:(exec->addresses[0]+exec->addresses[16]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_index(exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16])), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b68u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b69: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b6a: /* move    exec->words[1],x:>$fb */
    *(md_shared_scratch_x + 251u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x144b6au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b6c(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b6c: /* move    b,x:>$fc */
    *(md_shared_scratch_x + 252u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x144b6cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b6e: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b6eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b6f: /* move    exec->words[1],x:>$f9 */
    *(md_shared_scratch_x + 249u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x144b6fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b71: /* move    y:(exec->addresses[6]+$17),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b71u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b73: /* move    y:(exec->addresses[6]+$14),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b75: /* move    #>$1ff,exec->addresses[10] */
    exec->addresses[10] = (0x1ffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b77: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b78: /* move    #>$7ffcb9,exec->words[0] */
    exec->words[0] = (0x7ffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b78u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b7a(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b7a: /* move    #$40,exec->addresses[0] */
    exec->addresses[0] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b7b: /* move    #$60,exec->addresses[1] */
    exec->addresses[1] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b7bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b7c: /* move    (exec->addresses[2])+exec->addresses[18] */
    (void)md_agu(&exec->addresses[2],exec->addresses[10],sx24((int32_t)exec->addresses[18]),0u);
    MD_TRANSLATION_TRACE(0x144b7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b7d: /* move    y:(exec->addresses[6]+$12),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 18u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144b7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b7e: /* move    x:(exec->addresses[0])+,exec->words[2] */
    exec->words[2] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b7eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b7f: /* move    y:(exec->addresses[6]+$1d),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 29u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144b7fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b80: /* move    x:(exec->addresses[2])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b83(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b83: /* mac     exec->words[2],exec->words[0],a         a,x:(exec->addresses[1])+       a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b83u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b84: /* mac     -exec->words[2],exec->words[0],a        x:(exec->addresses[0])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b85: /* mac     exec->words[3],exec->words[1],b         b,x:(exec->addresses[3])+       b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b85u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b86: /* mac     -exec->words[3],exec->words[1],b        x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b87(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b87: /* move    y:(exec->addresses[6]+$15),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b87u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b89: /* move    y:(exec->addresses[6]+$18),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b8b: /* move    b,y:(exec->addresses[6]+$1d) */
    *(v->y + 29u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x144b8bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b8c: /* move    y:(exec->addresses[6]+$1e),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 30u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144b8cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b8d: /* move    (exec->addresses[2])+exec->addresses[18] */
    (void)md_agu(&exec->addresses[2],exec->addresses[10],sx24((int32_t)exec->addresses[18]),0u);
    MD_TRANSLATION_TRACE(0x144b8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b8e: /* move    #$a0,exec->addresses[3] */
    exec->addresses[3] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b8f: /* move    a,y:(exec->addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144b8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b90: /* move    x:(exec->addresses[2])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b93(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b93: /* mac     exec->words[3],exec->words[1],b         b,x:(exec->addresses[3])+       b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b94: /* mac     -exec->words[3],exec->words[1],b        x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b94u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b95(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b95: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b95u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b96: /* move    b,y:(exec->addresses[6]+$1e) */
    *(v->y + 30u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x144b96u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b97: /* clr     a */
    exec->accum[0] = 0; exec->lows[0] = 0u; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b97u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b98: /* clr     b */
    exec->accum[1] = 0; exec->lows[1] = 0u; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144b98u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b99: /* move    y:(exec->addresses[6]+$22),b1 */
    exec->accum[1] = (int32_t)(((uint32_t)exec->accum[1] & 0xff000000u) | ((u24(*(v->y + 34u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x144b99u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b9a: /* move    y:(exec->addresses[6]+$23),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 35u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b9au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144b9d(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144b9d: /* add     exec->words[1],b            exec->words[1],a1 */
    exec->parallel_words[0] = exec->words[1]; add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[0] = (int32_t)(((uint32_t)exec->accum[0] & 0xff000000u) | ((exec->parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x144b9du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b9e: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x144b9eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b9f: /* move    b1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144b9fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba0: /* add     exec->words[1],a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[1])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144ba0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba1: /* move    exec->words[1],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[1]);
    MD_TRANSLATION_TRACE(0x144ba1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba2: /* move    a1,exec->words[1] */
    exec->words[1] = (u24(exec->accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ba2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144ba3(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144ba3: /* move    exec->words[1],y:(exec->addresses[6]+$22) */
    *(v->y + 34u) = sx24((int32_t)(exec->words[1]));
    MD_TRANSLATION_TRACE(0x144ba3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba4: /* move    b1,y:(exec->addresses[6]+$23) */
    *(v->y + 35u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x144ba4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba5: /* move    b1,y:>$fa */
    *(md_shared_scratch_y + 250u) = sx24((int32_t)(u24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x144ba5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba7: /* move    x:>$f9,exec->words[2] */
    exec->words[2] = (u24(*(md_shared_scratch_x + 249u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ba7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144ba9: /* move    y:(exec->addresses[6]+$16),exec->addresses[2] */
    exec->addresses[2] = (u24(*(v->y + 22u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144ba9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bab: /* move    y:(exec->addresses[6]+$19),exec->addresses[18] */
    exec->addresses[18] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144babu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bad: /* move    y:(exec->addresses[6]+$1f),a */
    exec->accum[0] = sx24((int32_t)(u24(*(v->y + 31u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144badu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bae: /* move    (exec->addresses[2])+exec->addresses[18] */
    (void)md_agu(&exec->addresses[2],exec->addresses[10],sx24((int32_t)exec->addresses[18]),0u);
    MD_TRANSLATION_TRACE(0x144baeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144baf(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144baf: /* move    #$c0,exec->addresses[3] */
    exec->addresses[3] = (0xc0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bafu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb0: /* move    x:(exec->addresses[2])+,exec->words[3] */
    exec->words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bb0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb1: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bb1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb2: /* move    y:(exec->addresses[6]+$27),b */
    exec->accum[1] = sx24((int32_t)(u24(*(v->y + 39u)))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144bb2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb3: /* move    #>$d9999a,exec->words[1] */
    exec->words[1] = (0xd9999au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bb3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bb7(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bb7: /* mac     exec->words[3],exec->words[2],a         a,x:(exec->addresses[3])+       a,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bb7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb8: /* mac     -exec->words[3],exec->words[2],a        y:(exec->addresses[4]),exec->words[0] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, exec->addresses[4], sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bb8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb9: /* add     exec->words[0],b            b,exec->words[0]            b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = u24(lim24(exec->accum[1])); add56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144bb9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bba: /* mac     exec->words[1],exec->words[0],b         x:(exec->addresses[2])+,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bbau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bbb(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bbb: /* move    a,y:(exec->addresses[6]+$1f) */
    *(v->y + 31u) = sx24((int32_t)(u24(lim24(exec->accum[0]))));
    MD_TRANSLATION_TRACE(0x144bbbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bbc: /* move    b,y:(exec->addresses[6]+$27) */
    *(v->y + 39u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x144bbcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bbd: /* move    #>$ffffff,exec->addresses[10] */
    exec->addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bbdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bbf: /* move    y:>$fa,a */
    exec->accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_y + 250u)))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144bbfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bc1: /* abs     a */
    if (exec->accum[0] < 0) { exec->accum[0] = (int32_t)(0u - (uint32_t)exec->accum[0] - (uint32_t)(exec->lows[0] != 0u)); exec->lows[0] = (0u - exec->lows[0]) & 0xffffffu; } exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144bc1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bc2: /* move    y:(exec->addresses[6]+$24),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 36u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bc2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bc3: /* cmp     exec->words[1],a */
    exec->accum[2] = sx24((int32_t)(exec->words[1])); exec->lows[2] = 0u; exec->flags[0] = (exec->accum[0] == exec->accum[2] && exec->lows[0] == exec->lows[2]); exec->flags[1] = (exec->accum[0] < exec->accum[2] || (exec->accum[0] == exec->accum[2] && exec->lows[0] < exec->lows[2])); exec->flags[2] = (!exec->flags[0] && !exec->flags[1]); exec->flags[3] = exec->flags[1];
    MD_TRANSLATION_TRACE(0x144bc3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bc6(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bc6: /* move    #>$7fffff,exec->words[0] */
    exec->words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bc6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bc8: /* move    exec->words[0],y:(exec->addresses[6]+$25) */
    *(v->y + 37u) = sx24((int32_t)(exec->words[0]));
    MD_TRANSLATION_TRACE(0x144bc8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bc9(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bc9: /* mpy     exec->words[1],exec->words[0],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144bc9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bca: /* move    y:(exec->addresses[6]+$6),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bcau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bcb: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bcbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bcc: /* mpy     exec->words[1],exec->words[2],a */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144bccu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bcd: /* move    #$40,exec->addresses[2] */
    exec->addresses[2] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bcdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bce: /* move    a,exec->words[2] */
    exec->words[2] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bceu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bcf: /* mpyi    #>$266666,exec->words[2],a */
    mpy_ss32(sx24((int32_t)(0x266666u)), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144bcfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bd1: /* move    #$0,exec->addresses[4] */
    exec->addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bd1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bd2(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bd2: /* move    exec->addresses[2],exec->addresses[0] */
    exec->addresses[0] = (exec->addresses[2]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bd2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bd3: /* move    #$60,exec->addresses[3] */
    exec->addresses[3] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bd3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bd4: /* move    a,exec->words[1] */
    exec->words[1] = (u24(lim24(exec->accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bd4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bd5: /* move    x:(exec->addresses[0])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144bd5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bd6: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bd6u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bd9(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bd9: /* mac     exec->words[1],exec->words[2],a         x:(exec->addresses[3]),b */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[3], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144bd9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bda: /* mac     exec->words[1],exec->words[2],b         x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bdau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bdb: /* tfr     exec->words[0],a            a,x:(exec->addresses[2])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x144bdbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bdc: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x144bdcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bdd(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bdd: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bddu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bde: /* move    #$a0,exec->addresses[1] */
    exec->addresses[1] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bdeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bdf: /* move    #$c0,exec->addresses[2] */
    exec->addresses[2] = (0xc0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bdfu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144be0: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144be0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144be1: /* move    exec->addresses[7],y:>$ff */
    *(md_shared_scratch_y + 255u) = sx24((int32_t)(exec->addresses[7]));
    MD_TRANSLATION_TRACE(0x144be1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144be3: /* move    #>$1ff,exec->addresses[15] */
    exec->addresses[15] = (0x1ffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144be3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144be5: /* move    y:(exec->addresses[6]+$16),exec->addresses[7] */
    exec->addresses[7] = (u24(*(v->y + 22u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144be5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144be7: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144be7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144be8(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144be8: /* move    #$20,exec->addresses[4] */
    exec->addresses[4] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144be8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144be9: /* move    #>$7eb852,exec->words[3] */
    exec->words[3] = (0x7eb852u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144be9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144beb: /* move    x:>$fb,exec->words[1] */
    exec->words[1] = (u24(*(md_shared_scratch_x + 251u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bebu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bed: /* move    x:>$fc,exec->words[2] */
    exec->words[2] = (u24(*(md_shared_scratch_x + 252u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bedu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bef: /* move    x:(exec->addresses[0]),exec->words[0] */
    exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144befu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf0: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[1]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bf0u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf1: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bf1u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf2: /* mpy     exec->words[0],exec->words[3],a         x:(exec->addresses[3])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bf2u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bf3(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bf3: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[2]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bf3u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf4: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bf4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bf7(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bf7: /* mpy     exec->words[0],exec->words[3],a         x:(exec->addresses[1])+,exec->words[0]      a,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144bf7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf8: /* mac     -exec->words[2],exec->words[0],a        x:(exec->addresses[2])+,exec->words[0]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144bf8u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf9: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[0]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bf9u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bfa: /* mpy     exec->words[0],exec->words[3],b         x:(exec->addresses[1]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bfau,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bfb: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0]),exec->words[0]       a,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[0], sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144bfbu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bfc: /* mpy     exec->words[0],exec->words[3],a         x:(exec->addresses[3])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[0])), sx24((int32_t)(exec->words[3])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bfcu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bfd: /* mac     exec->words[1],exec->words[0],a         x:(exec->addresses[2]),exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[2], sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bfdu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bfe: /* mac     exec->words[2],exec->words[0],b         x:(exec->addresses[0])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bfeu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144bff(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144bff: /* move    y:(exec->addresses[6]+$14),exec->addresses[0] */
    exec->addresses[0] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144bffu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c01: /* move    y:(exec->addresses[6]+$15),exec->addresses[1] */
    exec->addresses[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c01u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c03: /* move    #$0,exec->addresses[5] */
    exec->addresses[5] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c03u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c04: /* move    exec->addresses[15],exec->addresses[8] */
    exec->addresses[8] = (exec->addresses[15]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c04u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c05: /* move    exec->addresses[15],exec->addresses[9] */
    exec->addresses[9] = (exec->addresses[15]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c05u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c06: /* move    y:(exec->addresses[5])+,b */
    exec->accum[1] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine))); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144c06u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c07: /* move    y:(exec->addresses[5])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144c07u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c0a(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c0a: /* move    b,x:(exec->addresses[1])+       y:(exec->addresses[5])+,b */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[1]))); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144c0au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c0b: /* move    a,x:(exec->addresses[1])+       y:(exec->addresses[5])+,a */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[0]))); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144c0bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c0e(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c0e: /* move    b,x:(exec->addresses[0])+       y:(exec->addresses[5])+,b */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[1]))); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x144c0eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c0f: /* move    a,x:(exec->addresses[0])+       y:(exec->addresses[5])+,a */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, u24(lim24(exec->accum[0]))); exec->accum[0] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144c0fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c10(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c10: /* move    exec->addresses[0],y:(exec->addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(exec->addresses[0]));
    MD_TRANSLATION_TRACE(0x144c10u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c12: /* move    exec->addresses[1],y:(exec->addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(exec->addresses[1]));
    MD_TRANSLATION_TRACE(0x144c12u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c14: /* move    exec->addresses[7],y:(exec->addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(exec->addresses[7]));
    MD_TRANSLATION_TRACE(0x144c14u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c16: /* move    #>$ffffff,exec->addresses[8] */
    exec->addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c16u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c18: /* move    y:>$ff,exec->addresses[7] */
    exec->addresses[7] = (u24(*(md_shared_scratch_y + 255u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c18u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c1a: /* move    exec->addresses[8],exec->addresses[9] */
    exec->addresses[9] = (exec->addresses[8]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c1au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c1b: /* move    #$1f,exec->addresses[15] */
    exec->addresses[15] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c1bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c1c: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c1cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c1d(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c1d: /* move    #$a0,exec->addresses[1] */
    exec->addresses[1] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c1du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c1e: /* move    #$c0,exec->addresses[2] */
    exec->addresses[2] = (0xc0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c1eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c1f: /* move    #$80,exec->addresses[3] */
    exec->addresses[3] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c1fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c20: /* move    y:(exec->addresses[6]+$20),exec->words[3] */
    exec->words[3] = (u24(*(v->y + 32u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c20u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c21: /* move    y:(exec->addresses[6]+$21),exec->words[2] */
    exec->words[2] = (u24(*(v->y + 33u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c21u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c22: /* move    y:(exec->addresses[6]+$1c),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c23: /* tfr     exec->words[3],b            x:(exec->addresses[2])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); exec->accum[1] = sx24((int32_t)(exec->words[3])); exec->lows[1] = 0u; exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c23u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c26(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c26: /* move    x:(exec->addresses[1])+,a */
    exec->accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine))); exec->lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x144c26u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c27: /* add     exec->words[0],a            x:(exec->addresses[3])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine); add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c27u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c28: /* add     exec->words[0],a            exec->words[3],x:(exec->addresses[0])+ */
    exec->parallel_words[0] = exec->words[3]; add56(exec->accum[0], exec->lows[0], sx24((int32_t)(exec->words[0])), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x144c28u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c29: /* sub     exec->words[2],b            b,exec->words[3] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); sub56(exec->accum[1], exec->lows[1], sx24((int32_t)(exec->words[2])), 0u, &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c29u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c2a: /* add     a,b             a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c2au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c2b: /* mac     -exec->words[3],exec->words[1],b        x:(exec->addresses[2])+,exec->words[0] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; add56(exec->accum[1], exec->lows[1], exec->accum[2], exec->lows[2], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c2bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c2c(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c2c: /* move    exec->words[2],y:(exec->addresses[6]+$21) */
    *(v->y + 33u) = sx24((int32_t)(exec->words[2]));
    MD_TRANSLATION_TRACE(0x144c2cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c2d: /* move    b,y:(exec->addresses[6]+$20) */
    *(v->y + 32u) = sx24((int32_t)(u24(lim24(exec->accum[1]))));
    MD_TRANSLATION_TRACE(0x144c2du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c2e: /* move    #>$144c9a,exec->addresses[0] */
    exec->addresses[0] = (0x144c9au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c2eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c30: /* move    #$b4,exec->addresses[5] */
    exec->addresses[5] = (0xb4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c30u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c31: /* move    y:>$fe,exec->words[0] */
    exec->words[0] = (u24(*(md_shared_scratch_y + 254u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c31u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c33: /* move    x:(exec->addresses[0])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c34: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c34u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c35: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c35u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c36(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c36: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c36u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c37: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c37u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c38: /* move    y:(exec->addresses[6]+$1),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c38u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c39: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c39u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c3a: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c3au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c3b: /* mpy     exec->words[1],exec->words[0],a         x:(exec->addresses[0])+,exec->words[1]      a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c3bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c3c: /* mpy     exec->words[1],exec->words[0],b         x:(exec->addresses[0])+,exec->words[1]      b,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c3cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c3d: /* move    #$b4,exec->addresses[4] */
    exec->addresses[4] = (0xb4u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c3du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c3e(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c3e: /* move    a,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x144c3eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c3f: /* move    b,y:(exec->addresses[5])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x144c3fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c40: /* move    #$a0,exec->addresses[5] */
    exec->addresses[5] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c40u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c41: /* move    y:(exec->addresses[4])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c41u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c42: /* mpy     -exec->words[2],exec->words[2],b        #>$7fffff,exec->words[0] */
    exec->parallel_words[0] = 0x7fffffu; mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c42u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c44: /* tfr     exec->words[0],a            #>$155555,exec->words[1] */
    exec->parallel_words[0] = 0x155555u; exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; exec->words[1] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c44u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c48(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c48: /* add     b,a             b,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c48u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c49: /* mpy     exec->words[1],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144c49u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c4a: /* move    b,exec->words[3] */
    exec->words[3] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c4au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c4b: /* mac     exec->words[3],exec->words[2],a         y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c4bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c4c: /* mpy     -exec->words[2],exec->words[2],b */
    mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144c4cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c4d: /* tfr     exec->words[0],a            a,y:(exec->addresses[5])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->accum[0] = sx24((int32_t)(exec->words[0])); exec->lows[0] = 0u; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x144c4du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c4e(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c4e: /* move    #>$144caa,exec->addresses[5] */
    exec->addresses[5] = (0x144caau) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c4eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c50: /* move    #$0,exec->addresses[3] */
    exec->addresses[3] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c50u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c51: /* move    #$64,exec->addresses[4] */
    exec->addresses[4] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c51u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c52: /* move    #$2,exec->addresses[20] */
    exec->addresses[20] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c52u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c53: /* move    y:(exec->addresses[5])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c53u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c54: /* move    exec->words[0],x:(exec->addresses[3])+      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[0]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c54u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c55: /* move    exec->words[2],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x144c55u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c56: /* move    y:(exec->addresses[5])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c56u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c57(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c57: /* move    exec->words[0],x:(exec->addresses[3])+      y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[0]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c57u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c58: /* move    exec->words[2],x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[2]);
    MD_TRANSLATION_TRACE(0x144c58u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c59: /* move    y:(exec->addresses[6]+$8),exec->words[1] */
    exec->words[1] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c59u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c5a: /* move    y:(exec->addresses[5])+,exec->words[0] */
    exec->words[0] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c5au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c5b: /* mpy     exec->words[1],exec->words[0],a         y:(exec->addresses[5])+,exec->words[0] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c5bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c5c: /* mpy     exec->words[1],exec->words[0],b         y:(exec->addresses[5])+,exec->words[0] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c5cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c5d: /* mpy     exec->words[1],exec->words[0],a         a,x:(exec->addresses[3])+       y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c5du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c5e: /* mpy     exec->words[1],exec->words[2],b         b,x:(exec->addresses[3])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x144c5eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c5f(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c5f: /* move    a,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x144c5fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c60: /* move    b,x:(exec->addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, u24(lim24(exec->accum[1])));
    MD_TRANSLATION_TRACE(0x144c60u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c61: /* move    #>$144ca2,exec->addresses[1] */
    exec->addresses[1] = (0x144ca2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c61u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c63: /* move    #$0,exec->addresses[2] */
    exec->addresses[2] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c63u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c64: /* move    #$a0,exec->addresses[5] */
    exec->addresses[5] = (0xa0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c64u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c65: /* move    #>$142949,exec->addresses[0] */
    exec->addresses[0] = (0x142949u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c65u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c67: /* move    y:(exec->addresses[6]+$2),exec->words[0] */
    exec->words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c67u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c68: /* move    x:(exec->addresses[1])+,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c68u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c69(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c69: /* mpy     exec->words[1],exec->words[0],b */
    mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144c69u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c6a: /* move    y:(exec->addresses[5])+,exec->words[2] */
    exec->words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c6au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c6b: /* move    b,exec->addresses[16] */
    exec->addresses[16] = (u24(lim24(exec->accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c6bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c6e(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c6e: /* move    x:(exec->addresses[0]+exec->addresses[16]),exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_index(exec->addresses[0], exec->addresses[8], sx24((int32_t)exec->addresses[16])), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c6eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c6f: /* mpy     exec->words[1],exec->words[2],a         exec->words[1],exec->words[3] */
    exec->parallel_words[0] = exec->words[1]; mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[3] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c6fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c70: /* asl     a */
    exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144c70u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c71: /* add     #>$800000,a */
    add56(exec->accum[0], exec->lows[0], sx24((int32_t)(0x800000u)), 0u, &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144c71u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c73: /* mpy     -exec->words[3],exec->words[1],b */
    mpy_ss32(sx24((int32_t)(exec->words[3])), sx24((int32_t)(exec->words[1])), &exec->accum[2], &exec->lows[2]); exec->accum[2] = (int32_t)(0u - (uint32_t)exec->accum[2] - (uint32_t)(exec->lows[2] != 0u)); exec->lows[2] = (0u - exec->lows[2]) & 0xffffffu; exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]);
    MD_TRANSLATION_TRACE(0x144c73u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c74: /* add     b,a             x:(exec->addresses[1])+,exec->words[1]      a,exec->words[3] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine); exec->parallel_words[1] = u24(lim24(exec->accum[0])); add56(exec->accum[0], exec->lows[0], exec->accum[1], exec->lows[1], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[3] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c74u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c75: /* mpy     exec->words[1],exec->words[0],b         b,y:(exec->addresses[4])+ */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); exec->accum[1] = exec->accum[2]; exec->lows[1] = exec->lows[2]; exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x144c75u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c76: /* move    x:(exec->addresses[2])+,exec->words[1]      a,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[2], exec->addresses[10], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c76u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c77(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c77: /* mpy     exec->words[1],exec->words[2],a         b,exec->addresses[16] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); exec->accum[0] = exec->accum[2]; exec->lows[0] = exec->lows[2]; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->addresses[16] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c77u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c78: /* asl     a               y:(exec->addresses[5])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[5], exec->addresses[13], 1, 0u), sine); exec->accum[2] = exec->accum[0]; exec->lows[2] = exec->lows[0]; exec->accum[0] = (int32_t)(((uint32_t)exec->accum[2] << 1u) | (exec->lows[2] >> 23u)); exec->lows[0] = (exec->lows[2] << 1u) & 0xffffffu; exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c78u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c79: /* move    exec->words[3],y:(exec->addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine, exec->words[3]);
    MD_TRANSLATION_TRACE(0x144c79u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c7a: /* move    a,y:(exec->addresses[4])+exec->addresses[20] */
    md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], sx24((int32_t)exec->addresses[20]), 0u), sine, u24(lim24(exec->accum[0])));
    MD_TRANSLATION_TRACE(0x144c7au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c7b(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c7b: /* move    #$60,exec->addresses[0] */
    exec->addresses[0] = (0x60u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c7bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c7c: /* move    #$3f,exec->addresses[1] */
    exec->addresses[1] = (0x3fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c7cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c7d: /* move    #$64,exec->addresses[4] */
    exec->addresses[4] = (0x64u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c7du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c7e: /* lua     (exec->addresses[6]+$2a),exec->addresses[3] */
    exec->addresses[3] = (md_index(exec->addresses[6],exec->addresses[14],0x2au)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c7eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c7f: /* move    exec->addresses[15],exec->addresses[8] */
    exec->addresses[8] = (exec->addresses[15]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c7fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c80: /* move    #$80,exec->addresses[2] */
    exec->addresses[2] = (0x80u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c80u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c81: /* move    #$1,exec->addresses[10] */
    exec->addresses[10] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c82: /* move    #$2,exec->addresses[12] */
    exec->addresses[12] = (0x2u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c82u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c83(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c83: /* move    #$6,exec->addresses[20] */
    exec->addresses[20] = (0x6u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c83u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c84: /* move    (exec->addresses[7])- */
    (void)md_agu(&exec->addresses[7],exec->addresses[15],-1,0u);
    MD_TRANSLATION_TRACE(0x144c84u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c85: /* move    x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); exec->words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine)) & 0xffffffu; exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c85u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c88(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c88: /* move    l:(exec->addresses[3])+,a */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->long_address[0], sine); exec->parallel_words[3] = md_read_y_raw(v, md_shared_scratch_y, exec->long_address[0], sine); exec->accum[0] = sx24((int32_t)exec->parallel_words[0]); exec->lows[0] = exec->parallel_words[3];
    MD_TRANSLATION_TRACE(0x144c88u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c89: /* move    x:(exec->addresses[3])-,exec->words[1] */
    exec->words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c89u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c8c(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c8c: /* mac     exec->words[1],exec->words[2],a         a,exec->words[1]            y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[0])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[1] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c8cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c8d: /* mac     exec->words[1],exec->words[2],a         b,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(exec->words[1])), sx24((int32_t)(exec->words[2])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, exec->parallel_words[0]); exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c8du,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c8e: /* mac     exec->words[2],exec->words[0],a         x:(exec->addresses[1]),b        b,y:(exec->addresses[7])+ */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, exec->addresses[1], sine); exec->parallel_words[1] = u24(lim24(exec->accum[1])); mpy_ss32(sx24((int32_t)(exec->words[2])), sx24((int32_t)(exec->words[0])), &exec->accum[2], &exec->lows[2]); add56(exec->accum[0], exec->lows[0], exec->accum[2], exec->lows[2], &exec->accum[0], &exec->lows[0]); exec->flags[3] = (exec->accum[0] < 0); exec->flags[0] = (exec->accum[0] == 0 && exec->lows[0] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->accum[1] = sx24((int32_t)(exec->parallel_words[0])); exec->lows[1] = 0u; md_write_y(v, md_shared_scratch_y, md_agu(&exec->addresses[7], exec->addresses[15], 1, 0u), sine, exec->parallel_words[1]);
    MD_TRANSLATION_TRACE(0x144c8eu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c8f: /* add     a,b             x:(exec->addresses[0])+,exec->words[0]      y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&exec->addresses[0], exec->addresses[8], 1, 0u), sine); exec->parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); add56(exec->accum[1], exec->lows[1], exec->accum[0], exec->lows[0], &exec->accum[1], &exec->lows[1]); exec->flags[3] = (exec->accum[1] < 0); exec->flags[0] = (exec->accum[1] == 0 && exec->lows[1] == 0u); exec->flags[1] = exec->flags[3]; exec->flags[2] = (!exec->flags[0] && !exec->flags[3]); exec->words[0] = (exec->parallel_words[0]) & 0xffffffu; exec->words[2] = (exec->parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c8fu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c90(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c90: /* move    (exec->addresses[4])+exec->addresses[20] */
    (void)md_agu(&exec->addresses[4],exec->addresses[12],sx24((int32_t)exec->addresses[20]),0u);
    MD_TRANSLATION_TRACE(0x144c90u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c91: /* move    b,x:(exec->addresses[1])+       y:(exec->addresses[4])+,exec->words[2] */
    exec->parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&exec->addresses[4], exec->addresses[12], 1, 0u), sine); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[1], exec->addresses[9], 1, 0u), sine, u24(lim24(exec->accum[1]))); exec->words[2] = (exec->parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c92: /* move    a,l:(exec->addresses[3])+ */
    exec->long_address[0] = md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u); exec->parallel_words[0] = (uint32_t)(u24(lim24(exec->accum[0]))); exec->parallel_words[3] = (exec->accum[0] > 0x7fffff ? 0xffffffu : exec->accum[0] < -0x800000 ? 0u : exec->lows[0]); md_write_x(v, md_shared_scratch_x, exec->long_address[0], sine, exec->parallel_words[0]); md_write_y(v, md_shared_scratch_y, exec->long_address[0], sine, exec->parallel_words[3]);
    MD_TRANSLATION_TRACE(0x144c92u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c93: /* move    exec->words[1],x:(exec->addresses[3])+      b,y:(exec->addresses[7]) */
    exec->parallel_words[0] = u24(lim24(exec->accum[1])); md_write_x(v, md_shared_scratch_x, md_agu(&exec->addresses[3], exec->addresses[11], 1, 0u), sine, exec->words[1]); md_write_y(v, md_shared_scratch_y, exec->addresses[7], sine, exec->parallel_words[0]);
    MD_TRANSLATION_TRACE(0x144c93u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c94: /* move    #$3f,exec->addresses[1] */
    exec->addresses[1] = (0x3fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c94u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
static void pirs_render_block_44k1_group_144c95(Pirs_Render_Block_44K1Exec *exec, PirsVoice *v, const int32_t *sine) {
L_144c95: /* move    #>$ffffff,exec->addresses[12] */
    exec->addresses[12] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c95u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c97: /* move    exec->addresses[12],exec->addresses[10] */
    exec->addresses[10] = (exec->addresses[12]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c97u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c98: /* move    exec->addresses[12],exec->addresses[8] */
    exec->addresses[8] = (exec->addresses[12]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x144c98u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
}
void pirs_render_block_44k1(PirsVoice *v, const int32_t *sine) {
    Pirs_Render_Block_44K1Exec execution;
    Pirs_Render_Block_44K1Exec *exec;
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
    uint32_t loop_144b22 = 0;
    uint32_t loop_144b33 = 0;
    uint32_t loop_144b47 = 0;
    uint32_t loop_144b81 = 0;
    uint32_t loop_144b91 = 0;
    uint32_t loop_144b9b = 0;
    uint32_t loop_144bb5 = 0;
    uint32_t loop_144bd7 = 0;
    uint32_t loop_144bf5 = 0;
    uint32_t loop_144c08 = 0;
    uint32_t loop_144c0c = 0;
    uint32_t loop_144c24 = 0;
    uint32_t loop_144c46 = 0;
    uint32_t loop_144c6c = 0;
    uint32_t loop_144c86 = 0;
    uint32_t loop_144c8a = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_144b01:
    pirs_render_block_44k1_group_144b01(exec,v,sine);
L_144b0b:
    pirs_render_block_44k1_group_144b0b(exec,v,sine);
L_144b14:
    pirs_render_block_44k1_group_144b14(exec,v,sine);
L_144b1d:
    pirs_render_block_44k1_group_144b1d(exec,v,sine);
L_144b22: /* do      #<$10,>$144b26 */
    loop_144b22 = 0x10u; if (loop_144b22 == 0u) goto L_144b26;
    MD_TRANSLATION_TRACE(0x144b22u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b24:
    pirs_render_block_44k1_group_144b24(exec,v,sine);
    if (--loop_144b22 != 0u) goto L_144b24;
L_144b26:
    pirs_render_block_44k1_group_144b26(exec,v,sine);
L_144b2f:
    pirs_render_block_44k1_group_144b2f(exec,v,sine);
L_144b33: /* do      #<$10,>$144b3f */
    loop_144b33 = 0x10u; if (loop_144b33 == 0u) goto L_144b3f;
    MD_TRANSLATION_TRACE(0x144b33u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b35:
    pirs_render_block_44k1_group_144b35(exec,v,sine);
L_144b3d:
    pirs_render_block_44k1_group_144b3d(exec,v,sine);
    if (--loop_144b33 != 0u) goto L_144b35;
L_144b3f:
    pirs_render_block_44k1_group_144b3f(exec,v,sine);
L_144b47: /* do      #<$10,>$144b4d */
    loop_144b47 = 0x10u; if (loop_144b47 == 0u) goto L_144b4d;
    MD_TRANSLATION_TRACE(0x144b47u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b49:
    pirs_render_block_44k1_group_144b49(exec,v,sine);
    if (--loop_144b47 != 0u) goto L_144b49;
L_144b4d:
    pirs_render_block_44k1_group_144b4d(exec,v,sine);
L_144b57:
    pirs_render_block_44k1_group_144b57(exec,v,sine);
L_144b61:
    pirs_render_block_44k1_group_144b61(exec,v,sine);
L_144b6c:
    pirs_render_block_44k1_group_144b6c(exec,v,sine);
L_144b7a:
    pirs_render_block_44k1_group_144b7a(exec,v,sine);
L_144b81: /* do      #<$20,>$144b87 */
    loop_144b81 = 0x20u; if (loop_144b81 == 0u) goto L_144b87;
    MD_TRANSLATION_TRACE(0x144b81u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b83:
    pirs_render_block_44k1_group_144b83(exec,v,sine);
    if (--loop_144b81 != 0u) goto L_144b83;
L_144b87:
    pirs_render_block_44k1_group_144b87(exec,v,sine);
L_144b91: /* do      #<$20,>$144b95 */
    loop_144b91 = 0x20u; if (loop_144b91 == 0u) goto L_144b95;
    MD_TRANSLATION_TRACE(0x144b91u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b93:
    pirs_render_block_44k1_group_144b93(exec,v,sine);
    if (--loop_144b91 != 0u) goto L_144b93;
L_144b95:
    pirs_render_block_44k1_group_144b95(exec,v,sine);
L_144b9b: /* do      #<$10,>$144ba3 */
    loop_144b9b = 0x10u; if (loop_144b9b == 0u) goto L_144ba3;
    MD_TRANSLATION_TRACE(0x144b9bu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144b9d:
    pirs_render_block_44k1_group_144b9d(exec,v,sine);
    if (--loop_144b9b != 0u) goto L_144b9d;
L_144ba3:
    pirs_render_block_44k1_group_144ba3(exec,v,sine);
L_144baf:
    pirs_render_block_44k1_group_144baf(exec,v,sine);
L_144bb5: /* do      #<$20,>$144bbb */
    loop_144bb5 = 0x20u; if (loop_144bb5 == 0u) goto L_144bbb;
    MD_TRANSLATION_TRACE(0x144bb5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bb7:
    pirs_render_block_44k1_group_144bb7(exec,v,sine);
    if (--loop_144bb5 != 0u) goto L_144bb7;
L_144bbb:
    pirs_render_block_44k1_group_144bbb(exec,v,sine);
L_144bc4: /* blt     func_144bc9 */
    if (exec->flags[1]) goto L_144bc9;
    MD_TRANSLATION_TRACE(0x144bc4u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bc6:
    pirs_render_block_44k1_group_144bc6(exec,v,sine);
L_144bc9:
    pirs_render_block_44k1_group_144bc9(exec,v,sine);
L_144bd2:
    pirs_render_block_44k1_group_144bd2(exec,v,sine);
L_144bd7: /* do      #<$20,>$144bdd */
    loop_144bd7 = 0x20u; if (loop_144bd7 == 0u) goto L_144bdd;
    MD_TRANSLATION_TRACE(0x144bd7u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bd9:
    pirs_render_block_44k1_group_144bd9(exec,v,sine);
    if (--loop_144bd7 != 0u) goto L_144bd9;
L_144bdd:
    pirs_render_block_44k1_group_144bdd(exec,v,sine);
L_144be8:
    pirs_render_block_44k1_group_144be8(exec,v,sine);
L_144bf3:
    pirs_render_block_44k1_group_144bf3(exec,v,sine);
L_144bf5: /* do      #<$20,>$144bff */
    loop_144bf5 = 0x20u; if (loop_144bf5 == 0u) goto L_144bff;
    MD_TRANSLATION_TRACE(0x144bf5u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144bf7:
    pirs_render_block_44k1_group_144bf7(exec,v,sine);
    if (--loop_144bf5 != 0u) goto L_144bf7;
L_144bff:
    pirs_render_block_44k1_group_144bff(exec,v,sine);
L_144c08: /* do      #<$10,>$144c0c */
    loop_144c08 = 0x10u; if (loop_144c08 == 0u) goto L_144c0c;
    MD_TRANSLATION_TRACE(0x144c08u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c0a:
    pirs_render_block_44k1_group_144c0a(exec,v,sine);
    if (--loop_144c08 != 0u) goto L_144c0a;
L_144c0c: /* do      #<$10,>$144c10 */
    loop_144c0c = 0x10u; if (loop_144c0c == 0u) goto L_144c10;
    MD_TRANSLATION_TRACE(0x144c0cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c0e:
    pirs_render_block_44k1_group_144c0e(exec,v,sine);
    if (--loop_144c0c != 0u) goto L_144c0e;
L_144c10:
    pirs_render_block_44k1_group_144c10(exec,v,sine);
L_144c1d:
    pirs_render_block_44k1_group_144c1d(exec,v,sine);
L_144c24: /* do      #<$20,>$144c2c */
    loop_144c24 = 0x20u; if (loop_144c24 == 0u) goto L_144c2c;
    MD_TRANSLATION_TRACE(0x144c24u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c26:
    pirs_render_block_44k1_group_144c26(exec,v,sine);
    if (--loop_144c24 != 0u) goto L_144c26;
L_144c2c:
    pirs_render_block_44k1_group_144c2c(exec,v,sine);
L_144c36:
    pirs_render_block_44k1_group_144c36(exec,v,sine);
L_144c3e:
    pirs_render_block_44k1_group_144c3e(exec,v,sine);
L_144c46: /* do      #<$8,>$144c4e */
    loop_144c46 = 0x8u; if (loop_144c46 == 0u) goto L_144c4e;
    MD_TRANSLATION_TRACE(0x144c46u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c48:
    pirs_render_block_44k1_group_144c48(exec,v,sine);
    if (--loop_144c46 != 0u) goto L_144c48;
L_144c4e:
    pirs_render_block_44k1_group_144c4e(exec,v,sine);
L_144c57:
    pirs_render_block_44k1_group_144c57(exec,v,sine);
L_144c5f:
    pirs_render_block_44k1_group_144c5f(exec,v,sine);
L_144c69:
    pirs_render_block_44k1_group_144c69(exec,v,sine);
L_144c6c: /* do      #<$8,>$144c7b */
    loop_144c6c = 0x8u; if (loop_144c6c == 0u) goto L_144c7b;
    MD_TRANSLATION_TRACE(0x144c6cu,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c6e:
    pirs_render_block_44k1_group_144c6e(exec,v,sine);
L_144c77:
    pirs_render_block_44k1_group_144c77(exec,v,sine);
    if (--loop_144c6c != 0u) goto L_144c6e;
L_144c7b:
    pirs_render_block_44k1_group_144c7b(exec,v,sine);
L_144c83:
    pirs_render_block_44k1_group_144c83(exec,v,sine);
L_144c86: /* do      #<$8,>$144c95 */
    loop_144c86 = 0x8u; if (loop_144c86 == 0u) goto L_144c95;
    MD_TRANSLATION_TRACE(0x144c86u,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c88:
    pirs_render_block_44k1_group_144c88(exec,v,sine);
L_144c8a: /* do      #<$20,>$144c90 */
    loop_144c8a = 0x20u; if (loop_144c8a == 0u) goto L_144c90;
    MD_TRANSLATION_TRACE(0x144c8au,exec->accum[0],exec->lows[0],exec->accum[1],exec->lows[1],exec->words[0],exec->words[1],exec->words[2],exec->words[3],exec->addresses[2],exec->addresses[4],exec->addresses[7]);
L_144c8c:
    pirs_render_block_44k1_group_144c8c(exec,v,sine);
    if (--loop_144c8a != 0u) goto L_144c8c;
L_144c90:
    pirs_render_block_44k1_group_144c90(exec,v,sine);
    if (--loop_144c86 != 0u) goto L_144c88;
L_144c95:
    pirs_render_block_44k1_group_144c95(exec,v,sine);
L_144c99: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;}

uint32_t pirs_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_203f34: /* move.l a2, -(a7) */
    /* local register save */
C_203f36: /* move.l d[2], -(a7) */
    /* local register save */
C_203f38: /* movea.l $c(a7), a2 */
    /* resolved argument pointer */
C_203f3c: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_203f40: /* clr.l d[0] */
    d[0] = 0u;
C_203f42: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_203f44: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_203f46: /* move.l d[0], d[1] */
    d[1] = d[0];
C_203f48: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_203f4a: /* moveq #$1, d[2] */
    d[2] = 0x00000001u;
C_203f4c: /* and.l d[2], d[1] */
    lhs = d[1]; rhs = d[2]; d[1] = lhs & rhs;
C_203f4e: /* addi.l #$a, d[1] */
    lhs = d[1]; rhs = 0xau; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_203f54: /* move.b #$7f, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x7fu) & 0xffu);
C_203f58: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_203f5a: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_203f60: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_203f64: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203f66: /* addi.l #$fffffc18, d[0] */
    lhs = d[0]; rhs = 0xfffffc18u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203f6c: /* move.l d[0], $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_203f70: /* move.w $2(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_203f74: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_203f76: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_203f78: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_203f7e: /* move.l d[0], $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_203f82: /* clr.l d[0] */
    d[0] = 0u;
C_203f84: /* move.w $4(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_203f88: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_203f8a: /* addi.l #$80000, d[0] */
    lhs = d[0]; rhs = 0x80000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_203f90: /* move.l d[0], $c(a2) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_203f94: /* clr.l $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(0u);
C_203f98: /* clr.l $14(a2) */
    *(y + (20u) / 4u) = (int32_t)(0u);
C_203f9c: /* move.w $8(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_203fa0: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_203fa2: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_203fa4: /* move.l d[0], $18(a2) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_203fa8: /* clr.l d[0] */
    d[0] = 0u;
C_203faa: /* move.b $a(a1), d[0] */
    d[0] = (d[0] & 0xffffff00u) | ((((*(p + (10u) / 2u) >> (((10u) & 1u) ? 0u : 8u)) & 0xffu)) & 0xffu);
C_203fae: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_203fb0: /* movea.l d[0], a0 */
    a0 = d[0];
C_203fb2: /* adda.l #$24c794, a0 */
    /* resolved pointer base */
C_203fb8: /* move.l $490(a0), $1c(a2) */
    *(y + (28u) / 4u) = (int32_t)(*(control_24bb90 + ((3076u + ((1168u) + a0)) / 4u)));
C_203fbe: /* move.w $6(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_203fc2: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_203fc4: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_203fc6: /* move.l d[0], $20(a2) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_203fca: /* moveq #$9, d[0] */
    d[0] = 0x00000009u;
C_203fcc: /* move.l (a7)+, d[2] */
    /* local register restore */
C_203fce: /* movea.l (a7)+, a2 */
    /* local register restore */
C_203fd0: /* rts  */
    return d[0];
}

void pirs_update_coeffs(const uint32_t *p, PirsVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = pirs_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { pirs_dsp_init(v, 0); }
}
void pirs_voice_init(PirsVoice *v) {
    uint32_t i;
    for (i = 0u; i < 64u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;
    for (i=0u;i<64u;i++) v->x_private[i] = 0; for (i=0u;i<1536u;i++) v->external[i] = 0; v->virtual_voice = 0u; v->scratch_y20_history = 0;

}
void pirs_voice_trigger(PirsVoice *v) {
    pirs_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t pirs_resample_run(PirsVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE PirsVoice
#define MD_RENDER48 pirs_render_block_48k_float
#define MD_RENDER44 pirs_render_block_44k1
#define MD_RESAMPLE_RUN pirs_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE PirsVoice
#define MD_SLOT_INIT pirs_slot_init
#define MD_SLOT_PROCESS pirs_slot_process
#define MD_VOICE_INIT pirs_voice_init
#define MD_COEFFICIENTS pirs_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER pirs_update_coeffs
#define MD_VOICE_TRIGGER pirs_voice_trigger
#define MD_RENDER44 pirs_render_block_44k1
#define MD_RENDER48 pirs_render_block_48k_float
#include "../engine_slot.inc"
#endif
