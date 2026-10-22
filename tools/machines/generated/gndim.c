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
    int32_t y[9];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} GndimVoice;
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

static uint32_t md_read_x(GndimVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 32u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(GndimVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 32u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(GndimVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 1u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2057u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(GndimVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 1u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2057u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(GndimVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(GndimVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void gndim_dsp_init(GndimVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_100109: /* rts */
    return;
}

void gndim_dsp_trigger(GndimVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_10010a: /* move    y:(addresses[6]+$1),words[0] */
    words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10010au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10010b: /* move    y:(addresses[6]+$2),words[1] */
    words[1] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10010bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10010c: /* move    y:(addresses[6]+$3),words[2] */
    words[2] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10010cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10010d: /* move    y:(addresses[6]+$4),words[3] */
    words[3] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10010du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10010e: /* move    words[0],y:(addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x10010eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10010f: /* move    words[1],y:(addresses[6]+$6) */
    *(v->y + 6u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x10010fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100110: /* move    words[2],y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x100110u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100111: /* move    words[3],y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x100111u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100112: /* rts */
    return;
}

void gndim_render_block_44k1(GndimVoice *v, const int32_t *sine) {
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
    uint32_t loop_100119 = 0;
L_100113: /* move    y:(addresses[6]+$6),words[0] */
    words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x100113u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100114: /* move    y:(addresses[6]+$8),words[1] */
    words[1] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x100114u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100115: /* move    #>$0,words[2] */
    words[2] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x100115u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100117: /* move    y:(addresses[6]+$5),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 5u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x100117u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100118: /* move    y:(addresses[6]+$7),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 7u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x100118u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100119: /* do      #<$20,>$10012d */
    loop_100119 = 0x20u; if (loop_100119 == 0u) goto L_10012d;
    MD_TRANSLATION_TRACE(0x100119u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10011b: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10011bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10011c: /* beq     func_100123 */
    if (flags[0]) goto L_100123;
    MD_TRANSLATION_TRACE(0x10011cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10011e: /* sub     #>$1,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x1u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10011eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100120: /* move    words[0],y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, words[0]);
    MD_TRANSLATION_TRACE(0x100120u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100121: /* bra     func_10012c */
    goto L_10012c;
    MD_TRANSLATION_TRACE(0x100121u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100123: /* tst     b */
    flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x100123u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100124: /* beq     func_10012b */
    if (flags[0]) goto L_10012b;
    MD_TRANSLATION_TRACE(0x100124u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100126: /* sub     #>$1,b */
    sub56(accum[1], lows[1], sx24((int32_t)(0x1u)), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x100126u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100128: /* move    words[1],y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, words[1]);
    MD_TRANSLATION_TRACE(0x100128u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100129: /* bra     func_10012c */
    goto L_10012c;
    MD_TRANSLATION_TRACE(0x100129u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10012b: /* move    words[2],y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, words[2]);
    MD_TRANSLATION_TRACE(0x10012bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10012c: /* nop */

    MD_TRANSLATION_TRACE(0x10012cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_100119 != 0u) goto L_10011b;
L_10012d: /* move    a,y:(addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x10012du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10012e: /* move    b,y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x10012eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10012f: /* rts */
    return;
}

uint32_t gndim_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2011b4: /* movea.l $4(a7), a1 */
    /* resolved argument pointer */
C_2011b8: /* movea.l $8(a7), a0 */
    /* resolved argument pointer */
C_2011bc: /* clr.l d[0] */
    d[0] = 0u;
C_2011be: /* move.w (a0), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_2011c0: /* cmpi.l #$1fff, d[0] */
    lhs = d[0]; rhs = 0x1fffu;
C_2011c6: /* bhi.b $2011cc */
    if (lhs > rhs) goto C_2011cc;
C_2011c8: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_2011ca: /* bra.b $2011e0 */
    goto C_2011e0;
C_2011cc: /* addi.l #$ffffe000, d[0] */
    lhs = d[0]; rhs = 0xffffe000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2011d2: /* muls.l d[0], d[0] */
    d[0] = d[0] * (d[0]);
C_2011d6: /* moveq #$e, d[1] */
    d[1] = 0x0000000eu;
C_2011d8: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2011da: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2011e0: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_2011e4: /* move.w $2(a0), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_2011e8: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_2011ea: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_2011ec: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_2011f0: /* clr.l d[0] */
    d[0] = 0u;
C_2011f2: /* move.w $4(a0), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2011f6: /* cmpi.l #$1fff, d[0] */
    lhs = d[0]; rhs = 0x1fffu;
C_2011fc: /* bhi.b $201202 */
    if (lhs > rhs) goto C_201202;
C_2011fe: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_201200: /* bra.b $201216 */
    goto C_201216;
C_201202: /* addi.l #$ffffe000, d[0] */
    lhs = d[0]; rhs = 0xffffe000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201208: /* muls.l d[0], d[0] */
    d[0] = d[0] * (d[0]);
C_20120c: /* moveq #$e, d[1] */
    d[1] = 0x0000000eu;
C_20120e: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_201210: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201216: /* move.l d[0], $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_20121a: /* move.w $6(a0), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_20121e: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_201220: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_201222: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_201226: /* moveq #$5, d[0] */
    d[0] = 0x00000005u;
C_201228: /* rts  */
    return d[0];
}

void gndim_update_coeffs(const uint32_t *p, GndimVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = gndim_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { gndim_dsp_init(v, 0); }
}
void gndim_voice_init(GndimVoice *v) {
    uint32_t i;
    for (i = 0u; i < 9u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void gndim_voice_trigger(GndimVoice *v) {
    gndim_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t gndim_resample_run(GndimVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE GndimVoice
#define MD_RENDER48 gndim_render_block_48k_float
#define MD_RENDER44 gndim_render_block_44k1
#define MD_RESAMPLE_RUN gndim_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE GndimVoice
#define MD_SLOT_INIT gndim_slot_init
#define MD_SLOT_PROCESS gndim_slot_process
#define MD_VOICE_INIT gndim_voice_init
#define MD_COEFFICIENTS gndim_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER gndim_update_coeffs
#define MD_VOICE_TRIGGER gndim_voice_trigger
#define MD_RENDER44 gndim_render_block_44k1
#define MD_RENDER48 gndim_render_block_48k_float
#include "../engine_slot.inc"
#endif
