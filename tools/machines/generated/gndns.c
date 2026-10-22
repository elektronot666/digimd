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
    int32_t y[6];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} GndnsVoice;
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

static uint32_t md_read_x(GndnsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 32u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(GndnsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 32u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(GndnsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 1u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2054u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(GndnsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 1u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2054u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(GndnsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(GndnsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void gndns_dsp_init(GndnsVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1000dd: /* move    #>$0,words[0] */
    words[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000ddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000df: /* move    words[0],y:(addresses[6]+$3) */
    *(v->y + 3u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000dfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e0: /* move    #>$1,words[0] */
    words[0] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000e0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e2: /* move    words[0],y:(addresses[6]+$4) */
    *(v->y + 4u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000e2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e3: /* rts */
    return;
}

void gndns_dsp_trigger(GndnsVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1000e4: /* move    #>$7ffcb9,words[0] */
    words[0] = (0x7ffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000e4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e6: /* move    words[0],y:(addresses[6]+$2) */
    *(v->y + 2u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000e6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e7: /* move    #$0,words[0] */
    words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000e7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e8: /* move    words[0],y:(addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000e8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000e9: /* rts */
    return;
}

void gndns_render_block_44k1(GndnsVoice *v, const int32_t *sine) {
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
    uint32_t loop_1000ef = 0;
    uint32_t loop_1000fd = 0;
L_1000ea: /* move    #>$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000eau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ec: /* move    #$1f,addresses[12] */
    addresses[12] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000ecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ed: /* move    y:(addresses[6]+$3),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 3u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1000edu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ee: /* move    y:(addresses[6]+$4),words[1] */
    words[1] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000eeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ef: /* do      #<$1f,>$1000f6 */
    loop_1000ef = 0x1fu; if (loop_1000ef == 0u) goto L_1000f6;
    MD_TRANSLATION_TRACE(0x1000efu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f1: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000f1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f2: /* move    words[1],words[0] */
    words[0] = (words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000f2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f3: /* move    b1,x:(addresses[4])+ */
    *(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1000f3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f4: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000f4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f5: /* move    words[0],b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1000f5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1000ef != 0u) goto L_1000f1;
L_1000f6: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000f6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f7: /* move    words[1],y:(addresses[6]+$3) */
    *(v->y + 3u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1000f7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f8: /* move    b1,y:(addresses[6]+$4) */
    *(v->y + 4u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1000f8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000f9: /* move    b1,x:(addresses[4])+ */
    *(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1000f9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000fa: /* move    y:(addresses[6]+$1),words[3] */
    words[3] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000fau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000fb: /* move    y:(addresses[6]+$2),words[1] */
    words[1] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000fbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000fc: /* move    y:(addresses[6]+$5),words[0] */
    words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000fcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000fd: /* do      #<$20,>$100106 */
    loop_1000fd = 0x20u; if (loop_1000fd == 0u) goto L_100106;
    MD_TRANSLATION_TRACE(0x1000fdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ff: /* move    x:(addresses[4])+,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000ffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100100: /* mpy     words[1],words[2],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x100100u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100101: /* mpysu   words[3],words[0],b */
    mpy_su32(sx24((int32_t)(words[3])), words[0], &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x100101u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100102: /* dmac    ss words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); lows[1] = u24(accum[1]); accum[1] = accum[1] >> 24; add56(accum[2], lows[2], accum[1], lows[1], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x100102u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100103: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x100103u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100104: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x100104u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100105: /* move    b0,words[0] */
    words[0] = (lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x100105u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1000fd != 0u) goto L_1000ff;
L_100106: /* move    words[1],y:(addresses[6]+$2) */
    *(v->y + 2u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x100106u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100107: /* move    words[0],y:(addresses[6]+$5) */
    *(v->y + 5u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x100107u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_100108: /* rts */
    return;
}

uint32_t gndns_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201194: /* movea.l $8(a7), a0 */
    /* resolved argument pointer */
C_201198: /* clr.l d[0] */
    d[0] = 0u;
C_20119a: /* move.w (a0), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20119c: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_20119e: /* lea.l $24bf94.l, a1 */
    /* resolved table pointer */
C_2011a4: /* movea.l $4(a7), a0 */
    /* resolved argument pointer */
C_2011a8: /* move.l (a1, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((1028u + (d[0] * 4u + (0))) / 4u));
C_2011ac: /* move.l d[0], $4(a0) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_2011b0: /* moveq #$2, d[0] */
    d[0] = 0x00000002u;
C_2011b2: /* rts  */
    return d[0];
}

void gndns_update_coeffs(const uint32_t *p, GndnsVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = gndns_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { gndns_dsp_init(v, 0); }
}
void gndns_voice_init(GndnsVoice *v) {
    uint32_t i;
    for (i = 0u; i < 6u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void gndns_voice_trigger(GndnsVoice *v) {
    gndns_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t gndns_resample_run(GndnsVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE GndnsVoice
#define MD_RENDER48 gndns_render_block_48k_float
#define MD_RENDER44 gndns_render_block_44k1
#define MD_RESAMPLE_RUN gndns_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE GndnsVoice
#define MD_SLOT_INIT gndns_slot_init
#define MD_SLOT_PROCESS gndns_slot_process
#define MD_VOICE_INIT gndns_voice_init
#define MD_COEFFICIENTS gndns_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER gndns_update_coeffs
#define MD_VOICE_TRIGGER gndns_voice_trigger
#define MD_RENDER44 gndns_render_block_44k1
#define MD_RENDER48 gndns_render_block_48k_float
#include "../engine_slot.inc"
#endif
