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
    int32_t y[18];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmbdVoice;
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

static uint32_t md_read_x(EfmbdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmbdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmbdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2066u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmbdVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2066u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmbdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmbdVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void efmbd_dsp_init(EfmbdVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102eef: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102eefu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef0: /* nop */

    MD_TRANSLATION_TRACE(0x102ef0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef1: /* move    a1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102ef1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef2: /* rts */
    return;
}

void efmbd_dsp_trigger(EfmbdVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102ef3: /* move    #>$50c335,b */
    accum[1] = sx24((int32_t)(0x50c335u)); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102ef3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef5: /* move    y:(addresses[6]+$f),words[1] */
    words[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ef5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef6: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ef6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef7: /* move    #>$fa0,a */
    accum[0] = sx24((int32_t)(0xfa0u)); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102ef7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ef9: /* move    y:(addresses[6]+$7),words[1] */
    words[1] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ef9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102efa: /* move    #>$7fffff,words[3] */
    words[3] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102efau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102efc: /* move    a1,y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102efcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102efd: /* move    a0,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102efdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102efe: /* move    a1,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102efeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102eff: /* move    a0,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102effu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f00: /* move    a0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f00u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f01: /* move    a0,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f01u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f02: /* move    words[1],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102f02u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f03: /* move    words[3],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102f03u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f04: /* move    b,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x102f04u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f05: /* rts */
    return;
}

void efmbd_render_block_44k1(EfmbdVoice *v, const int32_t *sine) {
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
    uint32_t loop_102f23 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_102f06: /* move    y:(addresses[6]+$1),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 1u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f06u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f07: /* move    y:(addresses[6]+$2),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 2u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f07u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f08: /* move    a1,x:$1 */
    *(md_shared_scratch_x + 1u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f08u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f09: /* move    b1,x:$2 */
    *(md_shared_scratch_x + 2u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f09u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f0a: /* move    y:(addresses[6]+$8),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 8u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f0au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f0b: /* move    y:(addresses[6]+$3),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 3u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f0bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f0c: /* move    a1,x:$8 */
    *(md_shared_scratch_x + 8u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f0cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f0d: /* move    b1,x:$3 */
    *(md_shared_scratch_x + 3u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f0du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f0e: /* move    y:(addresses[6]+$4),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 4u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f0eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f0f: /* move    y:(addresses[6]+$5),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 5u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f0fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f10: /* move    y:(addresses[6]+$6),words[1] */
    words[1] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f10u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f11: /* move    a1,x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f11u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f12: /* move    b1,x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f12u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f13: /* move    words[1],x:$6 */
    *(md_shared_scratch_x + 6u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102f13u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f14: /* move    y:(addresses[6]+$9),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 9u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102f14u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f15: /* move    y:(addresses[6]+$a),a0 */
    lows[0] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f15u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f16: /* move    y:(addresses[6]+$b),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 11u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102f16u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f17: /* move    y:(addresses[6]+$c),b0 */
    lows[1] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f17u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f18: /* move    a,l:<$9 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[9] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[9] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f18u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f19: /* move    b,l:<$b */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[11] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[11] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f19u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f1a: /* move    y:(addresses[6]+$d),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 13u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f1au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f1b: /* move    y:(addresses[6]+$e),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 14u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f1bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f1c: /* move    a1,x:$d */
    *(md_shared_scratch_x + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f1cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f1d: /* move    b1,x:$e */
    *(md_shared_scratch_x + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f1du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f1e: /* move    y:(addresses[6]+$f),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 15u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f1eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f1f: /* move    y:(addresses[6]+$10),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 16u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102f1fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f20: /* move    y:(addresses[6]+$11),b0 */
    lows[1] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f20u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f21: /* move    a1,x:$f */
    *(md_shared_scratch_x + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f21u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f22: /* move    b,l:<$10 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[16] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[16] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f22u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f23: /* do      #<$20,>$102f58 */
    loop_102f23 = 0x20u; if (loop_102f23 == 0u) goto L_102f58;
    MD_TRANSLATION_TRACE(0x102f23u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f25: /* move    x:$d,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f25u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f26: /* move    x:$8,words[0] */
    words[0] = (u24(*(md_shared_scratch_x + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f26u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f27: /* mpy     words[0],words[3],b         x:$1,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 1u)); mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f27u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f28: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f28u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f29: /* move    b1,x:$d */
    *(md_shared_scratch_x + 13u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f29u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f2a: /* asl     #$3,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f2au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f2b: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f2bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f2c: /* move    x:$3,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f2cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f2d: /* move    b1,words[3] */
    words[3] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f2du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f2e: /* mpy     words[3],words[2],a         l:<$10,x */
    parallel_words[0] = u24(md_shared_scratch_x[16]); parallel_words[3] = u24(md_shared_scratch_y[16]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102f2eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f2f: /* asr     #$5,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 5u; lows[0] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f2fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f30: /* move    l:<$9,b */
    parallel_words[0] = u24(md_shared_scratch_x[9]); parallel_words[3] = u24(md_shared_scratch_y[9]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102f30u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f31: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f31u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f32: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102f32u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f34: /* move    b1,addresses[0] */
    addresses[0] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f34u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f35: /* sub     x,b */
    sub56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f35u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f36: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f36u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f37: /* move    x:$6,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f37u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f38: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f38u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f3a: /* move    b10,l:<$9 */
    parallel_words[0] = (uint32_t)(accum[1]); parallel_words[3] = lows[1]; md_shared_scratch_x[9] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[9] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f3au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f3b: /* mpy     words[2],words[0],a         x:$e,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 14u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f3bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f3c: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f3cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f3d: /* move    x:$5,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f3du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f3e: /* move    a,l:<$10 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[16] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[16] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f3eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f3f: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f3fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f40: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f40u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f41: /* move    b1,x:$e */
    *(md_shared_scratch_x + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f41u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f42: /* move    a1,words[0] */
    words[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f42u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f43: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f43u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f44: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f44u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f45: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f45u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f46: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f46u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f47: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f47u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f48: /* add     words[3],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f48u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f49: /* move    l:<$b,a */
    parallel_words[0] = u24(md_shared_scratch_x[11]); parallel_words[3] = u24(md_shared_scratch_y[11]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102f49u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f4a: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f4au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f4b: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f4bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f4c: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f4cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f4d: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102f4du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f4f: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f4fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f51: /* move    a10,l:<$b */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[11] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[11] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x102f51u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f52: /* move    x:$f,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f52u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f53: /* move    x:$2,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102f53u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f54: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f54u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f55: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102f55u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f56: /* move    b1,x:$f */
    *(md_shared_scratch_x + 15u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f56u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f57: /* move    a1,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(accum[0]));
    MD_TRANSLATION_TRACE(0x102f57u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102f23 != 0u) goto L_102f25;
L_102f58: /* move    l:<$9,a */
    parallel_words[0] = u24(md_shared_scratch_x[9]); parallel_words[3] = u24(md_shared_scratch_y[9]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102f58u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f59: /* move    l:<$b,b */
    parallel_words[0] = u24(md_shared_scratch_x[11]); parallel_words[3] = u24(md_shared_scratch_y[11]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102f59u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f5a: /* move    a1,y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f5au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f5b: /* move    a0,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x102f5bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f5c: /* move    b1,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f5cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f5d: /* move    b0,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x102f5du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f5e: /* move    x:$d,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 13u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f5eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f5f: /* move    x:$e,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 14u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f5fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f60: /* move    a1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f60u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f61: /* move    b1,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f61u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f62: /* move    x:$f,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 15u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102f62u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f63: /* move    l:<$10,b */
    parallel_words[0] = u24(md_shared_scratch_x[16]); parallel_words[3] = u24(md_shared_scratch_y[16]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x102f63u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f64: /* move    a1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x102f64u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f65: /* move    b1,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102f65u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f66: /* move    b0,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x102f66u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102f67: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t efmbd_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_20211c: /* move.l a2, -(a7) */
    /* local register save */
C_20211e: /* move.l d[2], -(a7) */
    /* local register save */
C_202120: /* movea.l $c(a7), a2 */
    /* resolved argument pointer */
C_202124: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_202128: /* clr.l d[0] */
    d[0] = 0u;
C_20212a: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20212c: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_20212e: /* addi.l #$20, d[0] */
    lhs = d[0]; rhs = 0x20u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202134: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202136: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_202138: /* addq.l #$1, d[1] */
    d[1] = d[1] + 0x1u;
C_20213a: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_20213c: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_20213e: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_202144: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_202148: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20214a: /* move.l d[0], $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_20214e: /* move.w $2(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202152: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_202156: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_202158: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_20215a: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20215c: /* movea.l d[0], a0 */
    a0 = d[0];
C_20215e: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202164: /* move.l $24c(a0), $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20216a: /* lea.l $c(a2), a0 */
    a0 = 12u;
C_20216e: /* clr.l d[0] */
    d[0] = 0u;
C_202170: /* move.w $a(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_202174: /* move.b #$9, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x9u) & 0xffu);
C_202178: /* lsl.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] << ((d[2]) & 63u) : 0u);
C_20217a: /* move.l d[0], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[0]);
C_20217c: /* cmpi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu;
C_202182: /* bgt.b $20218a */
    if ((int32_t)lhs > (int32_t)rhs) goto C_20218a;
C_202184: /* move.l #$10000, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x10000u);
C_20218a: /* move.w $8(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_20218e: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_202190: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202192: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_202198: /* move.l d[0], $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20219c: /* move.w $c(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_2021a0: /* mulu.w #$52, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x52u);
C_2021a4: /* moveq #$c, d[2] */
    d[2] = 0x0000000cu;
C_2021a6: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_2021a8: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2021aa: /* movea.l d[0], a0 */
    a0 = d[0];
C_2021ac: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2021b2: /* move.l $24c(a0), $14(a2) */
    *(y + (20u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_2021b8: /* move.w $e(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_2021bc: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2021be: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_2021c4: /* move.l d[0], $18(a2) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_2021c8: /* clr.l d[0] */
    d[0] = 0u;
C_2021ca: /* move.w $4(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2021ce: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2021d0: /* addi.l #$fff00000, d[0] */
    lhs = d[0]; rhs = 0xfff00000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2021d6: /* move.l d[0], $1c(a2) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_2021da: /* move.w $6(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_2021de: /* mulu.w #$4b, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x4bu);
C_2021e2: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_2021e4: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2021e6: /* movea.l d[0], a0 */
    a0 = d[0];
C_2021e8: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2021ee: /* move.l $24c(a0), $20(a2) */
    *(y + (32u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_2021f4: /* moveq #$9, d[0] */
    d[0] = 0x00000009u;
C_2021f6: /* move.l (a7)+, d[2] */
    /* local register restore */
C_2021f8: /* movea.l (a7)+, a2 */
    /* local register restore */
C_2021fa: /* rts  */
    return d[0];
}

void efmbd_update_coeffs(const uint32_t *p, EfmbdVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmbd_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmbd_dsp_init(v, 0); }
}
void efmbd_voice_init(EfmbdVoice *v) {
    uint32_t i;
    for (i = 0u; i < 18u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmbd_voice_trigger(EfmbdVoice *v) {
    efmbd_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmbd_resample_run(EfmbdVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmbdVoice
#define MD_RENDER48 efmbd_render_block_48k_float
#define MD_RENDER44 efmbd_render_block_44k1
#define MD_RESAMPLE_RUN efmbd_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmbdVoice
#define MD_SLOT_INIT efmbd_slot_init
#define MD_SLOT_PROCESS efmbd_slot_process
#define MD_VOICE_INIT efmbd_voice_init
#define MD_COEFFICIENTS efmbd_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmbd_update_coeffs
#define MD_VOICE_TRIGGER efmbd_voice_trigger
#define MD_RENDER44 efmbd_render_block_44k1
#define MD_RENDER48 efmbd_render_block_48k_float
#include "../engine_slot.inc"
#endif
