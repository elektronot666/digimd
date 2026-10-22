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

} Trxb2Voice;
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

static uint32_t md_read_x(Trxb2Voice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(Trxb2Voice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(Trxb2Voice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2074u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(Trxb2Voice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2074u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(Trxb2Voice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(Trxb2Voice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void trxb2_dsp_init(Trxb2Voice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102c31: /* move    #>$7fef9e,words[0] */
    words[0] = (0x7fef9eu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c31u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c33: /* move    words[0],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c33u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c34: /* move    #>$75c28f,words[0] */
    words[0] = (0x75c28fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c34u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c36: /* move    words[0],y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c36u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c37: /* move    #>$12e70c,words[0] */
    words[0] = (0x12e70cu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c37u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c39: /* move    words[0],y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c39u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c3a: /* move    #>$1914a5,words[0] */
    words[0] = (0x1914a5u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c3au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c3c: /* move    words[0],y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c3cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c3d: /* move    #>$7fffff,words[0] */
    words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c3du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c3f: /* move    words[0],y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c3fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c40: /* move    #>$0,words[0] */
    words[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c40u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c42: /* move    words[0],y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c42u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c43: /* move    words[0],y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c43u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c44: /* move    words[0],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c44u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c45: /* move    words[0],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c45u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c46: /* move    words[0],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c46u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c47: /* move    words[0],y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c47u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c48: /* move    words[0],y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c48u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c49: /* rts */
    return;
}

void trxb2_dsp_trigger(Trxb2Voice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102c4a: /* move    y:(addresses[6]+$2),words[0] */
    words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c4au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c4b: /* move    words[0],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c4bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c4c: /* move    #>$147ae,words[0] */
    words[0] = (0x147aeu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c4cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c4e: /* move    words[0],y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c4eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c4f: /* move    y:(addresses[6]+$b),words[0] */
    words[0] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c4fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c50: /* move    words[0],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c50u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c51: /* move    y:(addresses[6]+$c),words[0] */
    words[0] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c51u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c52: /* move    words[0],y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c52u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c53: /* move    #>$7fffff,words[0] */
    words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c53u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c55: /* move    words[0],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c55u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c56: /* move    #>$7fffff,words[0] */
    words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c56u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c58: /* move    words[0],y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c58u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c59: /* rts */
    return;
}

void trxb2_render_block_44k1(Trxb2Voice *v, const int32_t *sine) {
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
    uint32_t loop_102c61 = 0;
    uint32_t loop_102c75 = 0;
    uint32_t loop_102cb3 = 0;
    uint32_t loop_102cc1 = 0;
    uint32_t loop_102ce6 = 0;
    uint32_t loop_102cfb = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_102c5a: /* clr     b */
    accum[1] = 0; lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c5au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c5b: /* move    y:(addresses[6]+$19),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 25u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102c5bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c5c: /* move    y:(addresses[6]+$1),words[1] */
    words[1] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c5cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c5d: /* move    y:(addresses[6]+$e),words[2] */
    words[2] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c5du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c5e: /* move    y:(addresses[6]+$d),words[3] */
    words[3] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c5eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c5f: /* move    #>$40,addresses[4] */
    addresses[4] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c5fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c61: /* do      #<$20,>$102c6b */
    loop_102c61 = 0x20u; if (loop_102c61 == 0u) goto L_102c6b;
    MD_TRANSLATION_TRACE(0x102c61u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c63: /* mpy     words[3],words[2],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c63u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c64: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c64u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c65: /* move    a,words[2] */
    words[2] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c65u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c66: /* add     words[2],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[2])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c66u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c67: /* and     #>$7fffff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffffu));
    MD_TRANSLATION_TRACE(0x102c67u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c69: /* nop */

    MD_TRANSLATION_TRACE(0x102c69u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c6a: /* move    b1,x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(accum[1]));
    MD_TRANSLATION_TRACE(0x102c6au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102c61 != 0u) goto L_102c63;
L_102c6b: /* move    words[2],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x102c6bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c6c: /* move    b1,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102c6cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c6d: /* move    #>$148000,addresses[2] */
    addresses[2] = (0x148000u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c6du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c6f: /* move    #>$7fff,addresses[10] */
    addresses[10] = (0x7fffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c6fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c71: /* move    #>$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c71u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c73: /* move    #>$40,addresses[5] */
    addresses[5] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c73u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c75: /* do      #<$20,>$102cab */
    loop_102c75 = 0x20u; if (loop_102c75 == 0u) goto L_102cab;
    MD_TRANSLATION_TRACE(0x102c75u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c77: /* move    x:(addresses[5])+,a */
    accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102c77u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c78: /* move    #>$ffe001,b */
    accum[1] = sx24((int32_t)(0xffe001u)); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102c78u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c7a: /* brset   #$16,a1,func_102c8f */
    if ((((u24(accum[0])) >> 22u) & 1u)) goto L_102c8f;
    MD_TRANSLATION_TRACE(0x102c7au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c7c: /* brset   #$15,a1,func_102c85 */
    if ((((u24(accum[0])) >> 21u) & 1u)) goto L_102c85;
    MD_TRANSLATION_TRACE(0x102c7cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c7e: /* move    a1,words[3] */
    words[3] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c7eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c7f: /* move    y:(addresses[6]+$5),words[0] */
    words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c7fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c80: /* mpy     words[0],words[3],a */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c80u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c81: /* max     a,b */
    if (accum[0] > accum[1] || (accum[0] == accum[1] && lows[0] >= lows[1])) { accum[1] = accum[0]; lows[1] = lows[0]; flags[4] = 0u; } else { flags[4] = 1u; }
    MD_TRANSLATION_TRACE(0x102c81u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c82: /* neg     b */
    accum[1] = (int32_t)(0u - (uint32_t)accum[1] - (uint32_t)(lows[1] != 0u)); lows[1] = (0u - lows[1]) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c82u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c83: /* bra     func_102ca6 */
    goto L_102ca6;
    MD_TRANSLATION_TRACE(0x102c83u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c85: /* sub     #>$400000,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x400000u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c85u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c87: /* move    y:(addresses[6]+$6),words[0] */
    words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c87u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c88: /* move    a1,words[3] */
    words[3] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c88u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c89: /* mpy     words[0],words[3],a */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c89u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c8a: /* max     a,b */
    if (accum[0] > accum[1] || (accum[0] == accum[1] && lows[0] >= lows[1])) { accum[1] = accum[0]; lows[1] = lows[0]; flags[4] = 0u; } else { flags[4] = 1u; }
    MD_TRANSLATION_TRACE(0x102c8au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c8b: /* add     #>$4000,b */
    add56(accum[1], lows[1], sx24((int32_t)(0x4000u)), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c8bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c8d: /* bra     func_102ca6 */
    goto L_102ca6;
    MD_TRANSLATION_TRACE(0x102c8du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c8f: /* brset   #$15,a1,func_102c9c */
    if ((((u24(accum[0])) >> 21u) & 1u)) goto L_102c9c;
    MD_TRANSLATION_TRACE(0x102c8fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c91: /* sub     #>$400000,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x400000u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c91u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c93: /* move    y:(addresses[6]+$7),words[0] */
    words[0] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c93u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c94: /* move    a1,words[3] */
    words[3] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c94u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c95: /* mpy     words[0],words[3],a */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c95u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c96: /* max     a,b */
    if (accum[0] > accum[1] || (accum[0] == accum[1] && lows[0] >= lows[1])) { accum[1] = accum[0]; lows[1] = lows[0]; flags[4] = 0u; } else { flags[4] = 1u; }
    MD_TRANSLATION_TRACE(0x102c96u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c97: /* neg     b */
    accum[1] = (int32_t)(0u - (uint32_t)accum[1] - (uint32_t)(lows[1] != 0u)); lows[1] = (0u - lows[1]) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c97u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c98: /* add     #>$4000,b */
    add56(accum[1], lows[1], sx24((int32_t)(0x4000u)), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c98u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c9a: /* bra     func_102ca6 */
    goto L_102ca6;
    MD_TRANSLATION_TRACE(0x102c9au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c9c: /* sub     #>$400000,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x400000u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c9cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c9e: /* sub     #>$400000,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x400000u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c9eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca0: /* move    y:(addresses[6]+$8),words[0] */
    words[0] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ca0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca1: /* move    a1,words[3] */
    words[3] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ca1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca2: /* mpy     words[0],words[3],a */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ca2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca3: /* max     a,b */
    if (accum[0] > accum[1] || (accum[0] == accum[1] && lows[0] >= lows[1])) { accum[1] = accum[0]; lows[1] = lows[0]; flags[4] = 0u; } else { flags[4] = 1u; }
    MD_TRANSLATION_TRACE(0x102ca3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca4: /* add     #>$8000,b */
    add56(accum[1], lows[1], sx24((int32_t)(0x8000u)), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ca4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca6: /* nop */

    MD_TRANSLATION_TRACE(0x102ca6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca7: /* move    b1,addresses[18] */
    addresses[18] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ca7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca8: /* move    x:(addresses[2]+addresses[18]),words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, md_index(addresses[2], addresses[10], sx24((int32_t)addresses[18])), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ca8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ca9: /* nop */

    MD_TRANSLATION_TRACE(0x102ca9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102caa: /* move    words[0],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[0]);
    MD_TRANSLATION_TRACE(0x102caau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102c75 != 0u) goto L_102c77;
L_102cab: /* move    #>$ffffff,addresses[10] */
    addresses[10] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cabu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cad: /* move    #>$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cadu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102caf: /* move    #>$40,addresses[5] */
    addresses[5] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cafu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb1: /* move    y:(addresses[6]+$9),words[1] */
    words[1] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cb1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb2: /* move    y:(addresses[6]+$a),words[3] */
    words[3] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cb2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb3: /* do      #<$20,>$102cbb */
    loop_102cb3 = 0x20u; if (loop_102cb3 == 0u) goto L_102cbb;
    MD_TRANSLATION_TRACE(0x102cb3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb5: /* move    x:(addresses[4])+,a */
    accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102cb5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb6: /* add     words[3],a */
    add56(accum[0], lows[0], sx24((int32_t)(words[3])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cb6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb7: /* asr     words[1],a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; shift_count[0] = words[1] & 63u; accum[1] = accum[2]; lows[1] = lows[2]; while (shift_count[0] != 0u) { lows[1] = ((lows[1] >> 1u) | ((uint32_t)accum[1] << 23u)) & 0xffffffu; accum[1] = accum[1] >> 1u; shift_count[0]--; } flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cb7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb8: /* move    #$0,b0 */
    lows[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cb8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cb9: /* asl     words[1],b,a */
    accum[2] = accum[1]; lows[2] = lows[1]; shift_count[0] = words[1] & 63u; accum[0] = accum[2]; lows[0] = lows[2]; while (shift_count[0] != 0u) { accum[0] = (int32_t)(((uint32_t)accum[0] << 1u) | (lows[0] >> 23u)); lows[0] = (lows[0] << 1u) & 0xffffffu; shift_count[0]--; } flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cb9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cba: /* move    a,x:(addresses[5])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[5], addresses[13], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102cbau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102cb3 != 0u) goto L_102cb5;
L_102cbb: /* move    #>$20,addresses[4] */
    addresses[4] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cbbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cbd: /* move    y:(addresses[6]+$13),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 19u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102cbdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cbe: /* move    y:(addresses[6]+$14),words[1] */
    words[1] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cbeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cbf: /* move    #>$0,words[2] */
    words[2] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cbfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc1: /* do      #<$7,>$102ccb */
    loop_102cc1 = 0x7u; if (loop_102cc1 == 0u) goto L_102ccb;
    MD_TRANSLATION_TRACE(0x102cc1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc3: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cc3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc4: /* move    words[1],words[0] */
    words[0] = (words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cc4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc5: /* move    b1,x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(accum[1]));
    MD_TRANSLATION_TRACE(0x102cc5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc6: /* move    words[2],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[2]);
    MD_TRANSLATION_TRACE(0x102cc6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc7: /* move    words[2],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[2]);
    MD_TRANSLATION_TRACE(0x102cc7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc8: /* move    words[2],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[2]);
    MD_TRANSLATION_TRACE(0x102cc8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cc9: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cc9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cca: /* move    words[0],b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102ccau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102cc1 != 0u) goto L_102cc3;
L_102ccb: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ccbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ccc: /* move    words[1],y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102cccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ccd: /* move    b1,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102ccdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cce: /* move    b1,x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(accum[1]));
    MD_TRANSLATION_TRACE(0x102cceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ccf: /* move    words[2],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[2]);
    MD_TRANSLATION_TRACE(0x102ccfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd0: /* move    words[2],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[2]);
    MD_TRANSLATION_TRACE(0x102cd0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd1: /* move    words[2],x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[2]);
    MD_TRANSLATION_TRACE(0x102cd1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd2: /* move    y:(addresses[6]+$12),words[0] */
    words[0] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cd2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd3: /* move    y:(addresses[6]+$11),words[2] */
    words[2] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cd3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd4: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cd4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd5: /* nop */

    MD_TRANSLATION_TRACE(0x102cd5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd6: /* move    a,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x102cd6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd7: /* move    y:(addresses[6]+$18),words[0] */
    words[0] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cd7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd8: /* move    y:(addresses[6]+$4),words[2] */
    words[2] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cd8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cd9: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cd9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cda: /* move    #>$7ffff8,words[0] */
    words[0] = (0x7ffff8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cdau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cdc: /* move    a,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x102cdcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cdd: /* cmp     #>$147ae,a */
    accum[2] = sx24((int32_t)(0x147aeu)); lows[2] = 0u; flags[0] = (accum[0] == accum[2] && lows[0] == lows[2]); flags[1] = (accum[0] < accum[2] || (accum[0] == accum[2] && lows[0] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x102cddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cdf: /* bpl     func_102ce2 */
    if (!flags[3]) goto L_102ce2;
    MD_TRANSLATION_TRACE(0x102cdfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce1: /* move    y:(addresses[6]+$3),words[0] */
    words[0] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ce1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce2: /* move    y:(addresses[6]+$15),words[1] */
    words[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ce2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce3: /* move    y:(addresses[6]+$10),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 16u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102ce3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce4: /* move    #>$40,addresses[4] */
    addresses[4] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ce4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce6: /* do      #<$20,>$102cf1 */
    loop_102ce6 = 0x20u; if (loop_102ce6 == 0u) goto L_102cf1;
    MD_TRANSLATION_TRACE(0x102ce6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce8: /* mpy     words[1],words[0],b         x:(addresses[4]),words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, addresses[4], sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ce8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ce9: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ce9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cea: /* cmp     #>$400000,b */
    accum[2] = sx24((int32_t)(0x400000u)); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x102ceau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cec: /* clr     a               ifge */
    if (!flags[1]) { accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); }
    MD_TRANSLATION_TRACE(0x102cecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ced: /* move    b,words[1] */
    words[1] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cedu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cee: /* mpy     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ceeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cef: /* move    b,x:(addresses[4])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102cefu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf0: /* nop */

    MD_TRANSLATION_TRACE(0x102cf0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102ce6 != 0u) goto L_102ce8;
L_102cf1: /* move    words[1],y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102cf1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf2: /* move    a,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x102cf2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf3: /* move    #>$733333,words[0] */
    words[0] = (0x733333u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cf3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf5: /* move    y:(addresses[6]+$17),words[2] */
    words[2] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cf5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf6: /* move    y:(addresses[6]+$11),words[1] */
    words[1] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cf6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf7: /* move    #>$40,addresses[4] */
    addresses[4] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cf7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cf9: /* move    #>$20,addresses[3] */
    addresses[3] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cf9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cfb: /* do      #<$20,>$102d04 */
    loop_102cfb = 0x20u; if (loop_102cfb == 0u) goto L_102d04;
    MD_TRANSLATION_TRACE(0x102cfbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cfd: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102cfdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cfe: /* move    x:(addresses[4])+,b */
    accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[4], addresses[12], 1, 0u), sine))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102cfeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102cff: /* move    a,words[2] */
    words[2] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102cffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102d00: /* asl     #$3,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[0] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102d00u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102d01: /* add     a,b             x:(addresses[3])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine); add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102d01u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102d02: /* mac     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102d02u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102d03: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102d03u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102cfb != 0u) goto L_102cfd;
L_102d04: /* move    words[2],y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x102d04u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102d05: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t trxb2_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201fd2: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_201fd6: /* movem.l d[2]/a2-a3, (a7) */
    /* native locals replace original callee saves */
C_201fda: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_201fde: /* movea.l $14(a7), a2 */
    /* resolved argument pointer */
C_201fe2: /* clr.l d[0] */
    d[0] = 0u;
C_201fe4: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_201fe6: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_201fe8: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_201fee: /* move.l (a0, d[0].l * 4), d[1] */
    d[1] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_201ff2: /* addi.l #$fffff800, d[1] */
    lhs = d[1]; rhs = 0xfffff800u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201ff8: /* move.l d[1], d[0] */
    d[0] = d[1];
C_201ffa: /* lsl.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] << ((0x3u) & 63u) : 0u);
C_201ffc: /* add.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201ffe: /* addi.l #$76e, d[0] */
    lhs = d[0]; rhs = 0x76eu; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202004: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_202008: /* clr.l d[2] */
    d[2] = 0u;
C_20200a: /* move.w $4(a2), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_20200e: /* move.l d[2], d[0] */
    d[0] = d[2];
C_202010: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_202012: /* movea.l d[0], a0 */
    a0 = d[0];
C_202014: /* lea.l (a0, d[2].l * 4), a0 */
    a0 = (d[2] * 4u + (0)) + a0;
C_202018: /* move.l a0, d[0] */
    d[0] = a0;
C_20201a: /* move.l a0, d[1] */
    d[1] = a0;
C_20201c: /* lsl.l #$5, d[1] */
    d[1] = (((0x5u) & 63u) < 32u ? d[1] << ((0x5u) & 63u) : 0u);
C_20201e: /* add.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202020: /* add.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202022: /* lsl.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] << ((0x3u) & 63u) : 0u);
C_202024: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_202026: /* lsl.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] << ((0x3u) & 63u) : 0u);
C_202028: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_20202a: /* moveq #$e, d[1] */
    d[1] = 0x0000000eu;
C_20202c: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_20202e: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_202032: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202036: /* mulu.w #$3, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x3u);
C_20203a: /* asr.l #$7, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_20203c: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20203e: /* movea.l d[0], a0 */
    a0 = d[0];
C_202040: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202046: /* move.l $190(a0), $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((400u) + a0)) / 4u)));
C_20204c: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_202050: /* lsr.l #$4, d[0] */
    d[0] = (((0x4u) & 63u) < 32u ? d[0] >> ((0x4u) & 63u) : 0u);
C_202052: /* andi.l #$ffc, d[0] */
    lhs = d[0]; rhs = 0xffcu; d[0] = lhs & rhs;
C_202058: /* movea.l d[0], a0 */
    a0 = d[0];
C_20205a: /* adda.l #$24c794, a0 */
    /* resolved pointer base */
C_202060: /* move.l $320(a0), $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(*(control_24bb90 + ((3076u + ((800u) + a0)) / 4u)));
C_202066: /* move.w $e(a2), d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_20206a: /* mulu.w d[1], d[1] */
    d[1] = (d[1] & 0xffffu) * ((d[1] & 0xffffu));
C_20206c: /* moveq #$c, d[2] */
    d[2] = 0x0000000cu;
C_20206e: /* asr.l d[2], d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_202070: /* move.l d[1], d[0] */
    d[0] = d[1];
C_202072: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_202074: /* move.l d[1], d[2] */
    d[2] = d[1];
C_202076: /* lsl.l #$6, d[2] */
    d[2] = (((0x6u) & 63u) < 32u ? d[2] << ((0x6u) & 63u) : 0u);
C_202078: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_20207a: /* addi.l #$ffff8000, d[0] */
    lhs = d[0]; rhs = 0xffff8000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202080: /* move.l d[0], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_202084: /* move.l d[1], d[2] */
    d[2] = d[1];
C_202086: /* lsl.l #$3, d[2] */
    d[2] = (((0x3u) & 63u) < 32u ? d[2] << ((0x3u) & 63u) : 0u);
C_202088: /* movea.l d[2], a3 */
    a3 = d[2];
C_20208a: /* lea.l (a3, d[1].l * 2), a0 */
    a0 = (d[1] * 2u + (0)) + a3;
C_20208e: /* move.l a0, d[0] */
    d[0] = a0;
C_202090: /* lsl.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] << ((0x3u) & 63u) : 0u);
C_202092: /* sub.l a0, d[0] */
    lhs = d[0]; rhs = a0; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_202094: /* addi.l #$8000, d[0] */
    lhs = d[0]; rhs = 0x8000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20209a: /* move.l d[0], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_20209e: /* lea.l (a0, a0.l * 4), a0 */
    a0 = (a0 * 4u + (0)) + a0;
C_2020a2: /* move.l #$ffff8000, d[0] */
    d[0] = 0xffff8000u;
C_2020a8: /* sub.l a0, d[0] */
    lhs = d[0]; rhs = a0; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_2020aa: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_2020ae: /* lsl.l #$5, d[1] */
    d[1] = (((0x5u) & 63u) < 32u ? d[1] << ((0x5u) & 63u) : 0u);
C_2020b0: /* add.l d[1], d[2] */
    lhs = d[2]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[2] = lhs + rhs;
C_2020b2: /* addi.l #$8000, d[2] */
    lhs = d[2]; rhs = 0x8000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[2] = lhs + rhs;
C_2020b8: /* move.l d[2], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[2]);
C_2020bc: /* clr.l d[0] */
    d[0] = 0u;
C_2020be: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_2020c2: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_2020c4: /* lea.l $24d794.l, a0 */
    /* resolved table pointer */
C_2020ca: /* move.l (a0, d[0].l * 4), d[1] */
    d[1] = *(control_24bb90 + ((7172u + (d[0] * 4u + (0))) / 4u));
C_2020ce: /* move.l d[1], d[0] */
    d[0] = d[1];
C_2020d0: /* lsl.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] << ((0x5u) & 63u) : 0u);
C_2020d2: /* lsl.l #$3, d[1] */
    d[1] = (((0x3u) & 63u) < 32u ? d[1] << ((0x3u) & 63u) : 0u);
C_2020d4: /* sub.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_2020d6: /* move.l #$26d0b, d[2] */
    d[2] = 0x26d0bu;
C_2020dc: /* divs.l d[2], d[0] */
    d[0] = md_divs32(d[0], d[2]);
C_2020e0: /* move.l d[0], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_2020e4: /* moveq #$1, d[1] */
    d[1] = 0x00000001u;
C_2020e6: /* lsl.l d[0], d[1] */
    d[1] = (((d[0]) & 63u) < 32u ? d[1] << ((d[0]) & 63u) : 0u);
C_2020e8: /* asr.l #$1, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x1u) & 63u) < 32u ? ((0x1u) & 63u) : 31u));
C_2020ea: /* move.l d[1], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[1]);
C_2020ee: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_2020f2: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_2020f4: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2020f6: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_2020fc: /* move.l d[0], $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(d[0]);
C_202100: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_202104: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_202106: /* andi.l #$ffff00, d[0] */
    lhs = d[0]; rhs = 0xffff00u; d[0] = lhs & rhs;
C_20210c: /* move.l d[0], $30(a1) */
    *(y + (48u) / 4u) = (int32_t)(d[0]);
C_202110: /* moveq #$d, d[0] */
    d[0] = 0x0000000du;
C_202112: /* movem.l (a7), d[2]/a2-a3 */
    /* native locals replace original callee saves */
C_202116: /* lea.l $c(a7), a7 */
    /* local register save area */
C_20211a: /* rts  */
    return d[0];
}

void trxb2_update_coeffs(const uint32_t *p, Trxb2Voice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxb2_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxb2_dsp_init(v, 0); }
}
void trxb2_voice_init(Trxb2Voice *v) {
    uint32_t i;
    for (i = 0u; i < 26u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxb2_voice_trigger(Trxb2Voice *v) {
    trxb2_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxb2_resample_run(Trxb2Voice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE Trxb2Voice
#define MD_RENDER48 trxb2_render_block_48k_float
#define MD_RENDER44 trxb2_render_block_44k1
#define MD_RESAMPLE_RUN trxb2_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE Trxb2Voice
#define MD_SLOT_INIT trxb2_slot_init
#define MD_SLOT_PROCESS trxb2_slot_process
#define MD_VOICE_INIT trxb2_voice_init
#define MD_COEFFICIENTS trxb2_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxb2_update_coeffs
#define MD_VOICE_TRIGGER trxb2_voice_trigger
#define MD_RENDER44 trxb2_render_block_44k1
#define MD_RENDER48 trxb2_render_block_48k_float
#include "../engine_slot.inc"
#endif
