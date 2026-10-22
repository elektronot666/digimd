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
    int32_t y[26];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmsdVoice;
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

static uint32_t md_read_x(EfmsdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmsdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmsdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2074u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmsdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2074u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmsdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmsdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void efmsd_dsp_init(EfmsdVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102f68: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f68u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f69: /* move    #>$789abc,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((0x789abcu) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f69u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f6b: /* move    a1,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f6bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f6c: /* move    a1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f6cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f6d: /* move    b1,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f6du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f6e: /* rts */
    return;
}

void efmsd_dsp_trigger(EfmsdVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102f6f: /* move    #>$50c335,b */
    accum[1] = sx24((int32_t)(0x50c335u)); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102f6fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f71: /* move    y:(addresses[6]+$10),words[1] */
    words[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f71u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f72: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f72u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f73: /* move    #>$1f4,a */
    accum[0] = sx24((int32_t)(0x1f4u)); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102f73u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f75: /* move    #>$7fffff,words[1] */
    words[1] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f75u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f77: /* move    a1,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f77u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f78: /* move    a0,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f78u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f79: /* move    a1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f79u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f7a: /* move    a0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f7au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f7b: /* move    a0,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f7bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f7c: /* move    a0,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f7cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f7d: /* move    a0,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f7du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f7e: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f7eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f7f: /* move    a0,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f7fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f80: /* move    a0,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f80u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f81: /* move    words[1],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102f81u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f82: /* move    b,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x102f82u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f83: /* move    words[1],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102f83u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f84: /* rts */
    return;
}

void efmsd_render_block_44k1(EfmsdVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
    uint32_t clear_scratch;
    uint32_t loop_102fb3 = 0;
    uint32_t loop_102fcf = 0;
    uint32_t loop_102fde = 0;
    uint32_t loop_103029 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_102f85: /* move    y:(addresses[6]+$1),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 1u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f85u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f86: /* move    y:(addresses[6]+$2),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 2u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f86u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f87: /* move    y:(addresses[6]+$3),words[1] */
    words[1] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f87u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f88: /* move    a1,x:$1 */
    *(md_shared_scratch_x + 1u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f88u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f89: /* move    b1,x:$2 */
    *(md_shared_scratch_x + 2u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f89u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f8a: /* move    words[1],x:$3 */
    *(md_shared_scratch_x + 3u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102f8au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f8b: /* move    y:(addresses[6]+$4),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 4u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f8bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f8c: /* move    y:(addresses[6]+$5),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 5u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f8cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f8d: /* move    y:(addresses[6]+$6),words[1] */
    words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f8du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f8e: /* move    a1,x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f8eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f8f: /* move    b1,x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f8fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f90: /* move    words[1],x:$6 */
    *(md_shared_scratch_x + 6u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102f90u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f91: /* move    y:(addresses[6]+$7),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 7u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f91u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f92: /* move    y:(addresses[6]+$8),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 8u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f92u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f93: /* move    a1,x:$7 */
    *(md_shared_scratch_x + 7u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f93u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f94: /* move    b1,x:$8 */
    *(md_shared_scratch_x + 8u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f94u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f95: /* move    y:(addresses[6]+$b),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 11u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102f95u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f96: /* move    y:(addresses[6]+$c),a0 */
    lows[0] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f96u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f97: /* move    y:(addresses[6]+$d),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 13u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102f97u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f98: /* move    y:(addresses[6]+$e),b0 */
    lows[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f98u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f99: /* move    a,l:<$b */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[11] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[11] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f99u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f9a: /* move    b,l:<$d */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f9au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f9b: /* move    y:(addresses[6]+$f),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 15u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f9bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f9c: /* move    y:(addresses[6]+$10),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 16u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f9cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f9d: /* move    a1,x:$f */
    *(md_shared_scratch_x + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f9du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f9e: /* move    b1,x:$10 */
    *(md_shared_scratch_x + 16u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f9eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f9f: /* move    y:(addresses[6]+$11),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 17u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f9fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa0: /* move    y:(addresses[6]+$12),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 18u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102fa0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa1: /* move    y:(addresses[6]+$13),b0 */
    lows[1] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fa1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa2: /* move    a1,x:$11 */
    *(md_shared_scratch_x + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102fa2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa3: /* move    b,l:<$12 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[18] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[18] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102fa3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa4: /* move    y:(addresses[6]+$14),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 20u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102fa4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa5: /* move    y:(addresses[6]+$15),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102fa5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa6: /* move    a1,x:$14 */
    *(md_shared_scratch_x + 20u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102fa6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa7: /* move    b1,x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102fa7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa8: /* move    y:(addresses[6]+$16),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 22u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102fa8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fa9: /* move    y:(addresses[6]+$18),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 24u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102fa9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102faa: /* move    a1,x:$16 */
    *(md_shared_scratch_x + 22u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102faau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fab: /* move    b1,x:$18 */
    *(md_shared_scratch_x + 24u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102fabu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fac: /* move    #$1a,addresses[2] */
    addresses[2] = (0x1au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102facu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fad: /* move    x:$14,a */
    accum[0] = sx24((int32_t)(u24(*(md_shared_scratch_x + 20u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102fadu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fae: /* move    x:$15,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102faeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102faf: /* add     words[1],a */
    add56(accum[0], lows[0], sx24((int32_t)(words[1])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fafu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb0: /* move    words[1],words[3] */
    words[3] = (words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fb0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb1: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fb1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb2: /* move    words[3],a */
    accum[0] = sx24((int32_t)(words[3])); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102fb2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb3: /* do      #<$1f,>$102fb9 */
    loop_102fb3 = 0x1fu; if (loop_102fb3 == 0u) goto L_102fb9;
    MD_TRANSLATION_TRACE(0x102fb3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb5: /* add     words[1],a            words[1],x:(addresses[2])+ */
    parallel_words[0] = words[1]; add56(accum[0], lows[0], sx24((int32_t)(words[1])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&addresses[2], addresses[10], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fb5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb6: /* move    words[1],words[3] */
    words[3] = (words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fb6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb7: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fb7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fb8: /* move    words[3],a */
    accum[0] = sx24((int32_t)(words[3])); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102fb8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102fb3 != 0u) goto L_102fb5;
L_102fb9: /* move    words[1],x:(addresses[2]) */
    md_write_x(v, md_shared_scratch_x, addresses[2], sine, words[1]);
    MD_TRANSLATION_TRACE(0x102fb9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fba: /* move    a1,x:$14 */
    *(md_shared_scratch_x + 20u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102fbau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fbb: /* move    words[1],x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102fbbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fbc: /* move    #$1a,addresses[5] */
    addresses[5] = (0x1au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fbcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fbd: /* move    #$1a,addresses[4] */
    addresses[4] = (0x1au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fbdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fbe: /* move    #$16,addresses[2] */
    addresses[2] = (0x16u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fbeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fbf: /* move    y:(addresses[6]+$9),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 9u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102fbfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc0: /* asr     #$1,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = accum[2] >> 1u; lows[1] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fc0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc1: /* move    #$40,words[0] */
    words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fc1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc2: /* add     words[0],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fc2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc3: /* move    a1,words[2] */
    words[2] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fc3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc4: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fc4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc5: /* move    x:(addresses[5])+,words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fc5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc6: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fc6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc7: /* move    x:(addresses[2]),words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fc7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc8: /* move    y:(addresses[6]+$17),words[3] */
    words[3] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fc8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fc9: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fc9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fca: /* mac     words[3],words[2],a         x:(addresses[5])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fcau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fcb: /* mpy     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fcbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fcc: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fcd: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fcdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fce: /* mac     words[3],words[2],b         x:(addresses[5])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fcf: /* do      #<$f,>$102fd9 */
    loop_102fcf = 0xfu; if (loop_102fcf == 0u) goto L_102fd9;
    MD_TRANSLATION_TRACE(0x102fcfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd1: /* mpy     words[3],words[1],a         a,x:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[0])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fd1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd2: /* move    x:(addresses[2]),words[0]       b,words[3] */
    parallel_words[0] = u24(lim24(accum[1])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fd2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd3: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fd3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd4: /* mac     words[3],words[2],a         x:(addresses[5])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fd4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd5: /* mpy     words[3],words[1],b         b,x:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[1])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fd5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd6: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fd6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd7: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x102fd7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fd8: /* mac     words[3],words[2],b         x:(addresses[5])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fd8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102fcf != 0u) goto L_102fd1;
L_102fd9: /* move    a,x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102fd9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fda: /* move    b,x:(addresses[4]) */
    md_write_x(v, md_shared_scratch_x, addresses[4], sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102fdau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fdb: /* move    b,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x102fdbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fdc: /* move    addresses[7],addresses[1] */
    addresses[1] = (addresses[7]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fdcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fdd: /* move    #$1a,addresses[2] */
    addresses[2] = (0x1au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fde: /* do      #<$20,>$103016 */
    loop_102fde = 0x20u; if (loop_102fde == 0u) goto L_103016;
    MD_TRANSLATION_TRACE(0x102fdeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe0: /* move    x:$3,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fe0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe1: /* move    x:$1,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fe1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe2: /* mpy     words[3],words[2],a         l:<$12,x */
    parallel_words[0] = u24(md_shared_scratch_x[18]); parallel_words[3] = u24(md_shared_scratch_y[18]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102fe2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe3: /* asr     #$5,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 5u; lows[0] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fe3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe4: /* move    l:<$b,b */
    parallel_words[0] = u24(md_shared_scratch_x[11]); parallel_words[3] = u24(md_shared_scratch_y[11]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102fe4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe5: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fe5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe6: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102fe6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe8: /* move    b1,addresses[0] */
    addresses[0] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fe8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fe9: /* sub     x,b */
    sub56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102fe9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fea: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102feau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102feb: /* move    x:$6,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102febu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fec: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fee: /* move    b10,l:<$b */
    parallel_words[0] = (uint32_t)(accum[1]); parallel_words[3] = lows[1]; md_shared_scratch_x[11] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[11] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102feeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fef: /* mpy     words[2],words[0],a         x:$f,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 15u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fefu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff0: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ff0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff1: /* move    x:$5,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ff1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff2: /* move    a,l:<$12 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[18] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[18] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102ff2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff3: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ff3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff4: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ff4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff5: /* move    b1,x:$f */
    *(md_shared_scratch_x + 15u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102ff5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff6: /* move    a1,words[0] */
    words[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ff6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff7: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ff7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff8: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ff8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ff9: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ff9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ffa: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ffau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ffb: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ffbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ffc: /* add     words[3],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ffcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ffd: /* move    l:<$d,a */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102ffdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ffe: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ffeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102fff: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102fffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103000: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103000u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103001: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x103001u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103003: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103003u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103005: /* move    a10,l:<$d */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103005u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103006: /* move    x:$10,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103006u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103007: /* move    x:$2,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103007u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103008: /* mpy     words[3],words[2],b         x:$8,words[3] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 8u)); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103008u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103009: /* mpyr    words[2],words[0],a         x:$11,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 17u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103009u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10300a: /* move    b1,x:$10 */
    *(md_shared_scratch_x + 16u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10300au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10300b: /* move    x:(addresses[2])+,words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[2], addresses[10], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10300bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10300c: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10300cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10300d: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10300du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10300e: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10300eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10300f: /* move    b1,x:$11 */
    *(md_shared_scratch_x + 17u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10300fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103010: /* move    a1,words[3] */
    words[3] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103010u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103011: /* move    x:$7,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103011u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103012: /* mpyr    words[3],words[2],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103012u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103013: /* add     words[1],a */
    add56(accum[0], lows[0], sx24((int32_t)(words[1])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103013u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103014: /* nop */

    MD_TRANSLATION_TRACE(0x103014u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103015: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x103015u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102fde != 0u) goto L_102fe0;
L_103016: /* move    addresses[1],addresses[7] */
    addresses[7] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103016u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103017: /* move    addresses[1],addresses[4] */
    addresses[4] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103017u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103018: /* move    #$18,addresses[2] */
    addresses[2] = (0x18u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103018u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103019: /* move    y:(addresses[6]+$a),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 10u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103019u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10301a: /* asr     #$1,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = accum[2] >> 1u; lows[1] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10301au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10301b: /* move    #$40,words[0] */
    words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10301bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10301c: /* add     words[0],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10301cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10301d: /* move    a1,words[2] */
    words[2] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10301du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10301e: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10301eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10301f: /* move    y:(addresses[7])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10301fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103020: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103020u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103021: /* move    x:(addresses[2]),words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103021u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103022: /* move    y:(addresses[6]+$19),words[3] */
    words[3] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103022u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103023: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103023u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103024: /* mac     words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103024u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103025: /* mpy     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103025u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103026: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103026u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103027: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103027u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103028: /* mac     words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103028u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103029: /* do      #<$f,>$103033 */
    loop_103029 = 0xfu; if (loop_103029 == 0u) goto L_103033;
    MD_TRANSLATION_TRACE(0x103029u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10302b: /* mpy     words[3],words[1],a         a,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[0])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10302bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10302c: /* move    x:(addresses[2]),words[0]       b,words[3] */
    parallel_words[0] = u24(lim24(accum[1])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10302cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10302d: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10302du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10302e: /* mac     words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10302eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10302f: /* mpy     words[3],words[1],b         b,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[1])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10302fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103030: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103030u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103031: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103031u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103032: /* mac     words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103032u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_103029 != 0u) goto L_10302b;
L_103033: /* move    a,y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x103033u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103034: /* move    b,y:(addresses[4]) */
    md_write_y(v, md_shared_scratch_y, addresses[4], sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x103034u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103035: /* move    b,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x103035u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103036: /* move    l:<$b,a */
    parallel_words[0] = u24(md_shared_scratch_x[11]); parallel_words[3] = u24(md_shared_scratch_y[11]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103036u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103037: /* move    l:<$d,b */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103037u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103038: /* move    a1,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103038u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103039: /* move    a0,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103039u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10303a: /* move    b1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10303au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10303b: /* move    b0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x10303bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10303c: /* move    x:$f,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 15u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10303cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10303d: /* move    x:$10,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 16u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10303du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10303e: /* move    a1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10303eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10303f: /* move    b1,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10303fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103040: /* move    x:$11,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 17u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103040u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103041: /* move    l:<$12,b */
    parallel_words[0] = u24(md_shared_scratch_x[18]); parallel_words[3] = u24(md_shared_scratch_y[18]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103041u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103042: /* move    a1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103042u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103043: /* move    b1,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103043u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103044: /* move    b0,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x103044u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103045: /* move    x:$14,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 20u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103045u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103046: /* move    x:$15,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103046u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103047: /* move    a1,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103047u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103048: /* move    b1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103048u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103049: /* move    x:$16,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 22u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103049u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10304a: /* move    x:$18,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 24u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10304au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10304b: /* move    a1,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10304bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10304c: /* move    b1,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10304cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10304d: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t efmsd_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2021fc: /* move.l a2, -(a7) */
    /* local register save */
C_2021fe: /* move.l d[2], -(a7) */
    /* local register save */
C_202200: /* movea.l $c(a7), a1 */
    /* resolved argument pointer */
C_202204: /* movea.l $10(a7), a2 */
    /* resolved argument pointer */
C_202208: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20220a: /* mulu.w #$5, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x5u);
C_20220e: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_202210: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202216: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202218: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_20221a: /* addq.l #$3, d[1] */
    d[1] = d[1] + 0x3u;
C_20221c: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_20221e: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202220: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_202226: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_20222a: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20222c: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_202230: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202234: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_202238: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_20223a: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_20223c: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20223e: /* movea.l d[0], a0 */
    a0 = d[0];
C_202240: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202246: /* move.l $24c(a0), $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20224c: /* lea.l $c(a1), a0 */
    a0 = 12u;
C_202250: /* clr.l d[0] */
    d[0] = 0u;
C_202252: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_202256: /* move.b #$9, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x9u) & 0xffu);
C_20225a: /* lsl.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] << ((d[2]) & 63u) : 0u);
C_20225c: /* move.l d[0], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[0]);
C_20225e: /* cmpi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu;
C_202264: /* bgt.b $20226c */
    if ((int32_t)lhs > (int32_t)rhs) goto C_20226c;
C_202266: /* move.l #$10000, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x10000u);
C_20226c: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_202270: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_202272: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202274: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_20227a: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20227e: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_202282: /* mulu.w #$52, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x52u);
C_202286: /* moveq #$c, d[2] */
    d[2] = 0x0000000cu;
C_202288: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_20228a: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20228c: /* movea.l d[0], a0 */
    a0 = d[0];
C_20228e: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202294: /* move.l $24c(a0), $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20229a: /* move.l #$7a1200, d[0] */
    d[0] = 0x7a1200u;
C_2022a0: /* move.l d[0], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_2022a4: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2022a8: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_2022aa: /* andi.l #$ffff00, d[0] */
    lhs = d[0]; rhs = 0xffff00u; d[0] = lhs & rhs;
C_2022b0: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_2022b4: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_2022b8: /* mulu.w #$32, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x32u);
C_2022bc: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_2022be: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2022c0: /* movea.l d[0], a0 */
    a0 = d[0];
C_2022c2: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2022c8: /* move.l $24c(a0), $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_2022ce: /* move.l #$44aa20, d[1] */
    d[1] = 0x44aa20u;
C_2022d4: /* move.l d[1], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[1]);
C_2022d8: /* clr.l d[0] */
    d[0] = 0u;
C_2022da: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_2022de: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_2022e0: /* move.l #$7fffff, d[2] */
    d[2] = 0x7fffffu;
C_2022e6: /* sub.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_2022e8: /* move.l d[2], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[2]);
C_2022ec: /* moveq #$b, d[0] */
    d[0] = 0x0000000bu;
C_2022ee: /* move.l (a7)+, d[2] */
    /* local register restore */
C_2022f0: /* movea.l (a7)+, a2 */
    /* local register restore */
C_2022f2: /* rts  */
    return d[0];
}

void efmsd_update_coeffs(const uint32_t *p, EfmsdVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmsd_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmsd_dsp_init(v, 0); }
}
void efmsd_voice_init(EfmsdVoice *v) {
    uint32_t i;
    for (i = 0u; i < 26u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmsd_voice_trigger(EfmsdVoice *v) {
    efmsd_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmsd_resample_run(EfmsdVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmsdVoice
#define MD_RENDER48 efmsd_render_block_48k_float
#define MD_RENDER44 efmsd_render_block_44k1
#define MD_RESAMPLE_RUN efmsd_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmsdVoice
#define MD_SLOT_INIT efmsd_slot_init
#define MD_SLOT_PROCESS efmsd_slot_process
#define MD_VOICE_INIT efmsd_voice_init
#define MD_COEFFICIENTS efmsd_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmsd_update_coeffs
#define MD_VOICE_TRIGGER efmsd_voice_trigger
#define MD_RENDER44 efmsd_render_block_44k1
#define MD_RENDER48 efmsd_render_block_48k_float
#include "../engine_slot.inc"
#endif
