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
    int32_t y[17];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} TrxxtVoice;
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
extern const uint32_t source_10190f[64];
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

static uint32_t md_read_x(TrxxtVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1054991u && address < 1055055u) { value = *(source_10190f + (address - 1054991u)); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(TrxxtVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(TrxxtVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2065u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1054991u && address < 1055055u) { value = *(source_10190f + (address - 1054991u)); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(TrxxtVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2065u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(TrxxtVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(TrxxtVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void trxxt_dsp_init(TrxxtVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_101882: /* rts */
    return;
}

void trxxt_dsp_trigger(TrxxtVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_101883: /* move    #>$3,words[0] */
    words[0] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101883u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101885: /* move    words[0],y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x101885u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101886: /* move    #>$7ffcb9,words[0] */
    words[0] = (0x7ffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101886u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101888: /* move    words[0],y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x101888u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101889: /* move    #$0,words[0] */
    words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101889u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10188a: /* move    words[0],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x10188au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10188b: /* move    words[0],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x10188bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10188c: /* move    y:(addresses[6]+$4),words[0] */
    words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10188cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10188d: /* move    y:(addresses[6]+$5),words[1] */
    words[1] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10188du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10188e: /* move    words[0],y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x10188eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10188f: /* move    words[1],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x10188fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101890: /* rts */
    return;
}

void trxxt_render_block_44k1(TrxxtVoice *v, const int32_t *sine) {
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
    uint32_t loop_1018a5 = 0;
    uint32_t loop_1018af = 0;
    uint32_t loop_1018bb = 0;
    uint32_t loop_1018d5 = 0;
    uint32_t loop_1018e4 = 0;
    uint32_t loop_1018f3 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_101891: /* move    y:(addresses[6]+$b),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 11u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x101891u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101892: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x101892u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101893: /* beq     func_1018c4 */
    if (flags[0]) goto L_1018c4;
    MD_TRANSLATION_TRACE(0x101893u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101895: /* sub     #>$1,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x1u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x101895u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101897: /* nop */

    MD_TRANSLATION_TRACE(0x101897u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101898: /* move    a,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x101898u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101899: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x101899u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10189a: /* beq     func_1018b8 */
    if (flags[0]) goto L_1018b8;
    MD_TRANSLATION_TRACE(0x10189au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10189c: /* sub     #>$1,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x1u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10189cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10189e: /* nop */

    MD_TRANSLATION_TRACE(0x10189eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10189f: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10189fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a0: /* beq     func_1018ac */
    if (flags[0]) goto L_1018ac;
    MD_TRANSLATION_TRACE(0x1018a0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a2: /* move    #>$733333,words[0] */
    words[0] = (0x733333u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018a2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a4: /* move    y:(addresses[6]+$10),words[1] */
    words[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018a4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a5: /* do      #<$20,>$1018ab */
    loop_1018a5 = 0x20u; if (loop_1018a5 == 0u) goto L_1018ab;
    MD_TRANSLATION_TRACE(0x1018a5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a7: /* mpy     words[1],words[0],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018a7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a8: /* nop */

    MD_TRANSLATION_TRACE(0x1018a8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018a9: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x1018a9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018aa: /* move    a,words[1] */
    words[1] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018aau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1018a5 != 0u) goto L_1018a7;
L_1018ab: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_1018ac: /* move    y:(addresses[6]+$9),words[0] */
    words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018acu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ad: /* move    #>$10190f,addresses[0] */
    addresses[0] = (0x10190fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018adu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018af: /* do      #<$10,>$1018b7 */
    loop_1018af = 0x10u; if (loop_1018af == 0u) goto L_1018b7;
    MD_TRANSLATION_TRACE(0x1018afu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b1: /* move    y:(addresses[0])+,words[2] */
    words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[0], addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018b1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b2: /* move    y:(addresses[0])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[0], addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018b2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b3: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018b3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b4: /* mpy     words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018b4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b5: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x1018b5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b6: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x1018b6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1018af != 0u) goto L_1018b1;
L_1018b7: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_1018b8: /* move    y:(addresses[6]+$9),words[0] */
    words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018b8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018b9: /* move    #>$10192f,addresses[0] */
    addresses[0] = (0x10192fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018b9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018bb: /* do      #<$10,>$1018c3 */
    loop_1018bb = 0x10u; if (loop_1018bb == 0u) goto L_1018c3;
    MD_TRANSLATION_TRACE(0x1018bbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018bd: /* move    y:(addresses[0])+,words[2] */
    words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[0], addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018bdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018be: /* move    y:(addresses[0])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[0], addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018beu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018bf: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018bfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c0: /* mpy     words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018c0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c1: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x1018c1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c2: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x1018c2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1018bb != 0u) goto L_1018bd;
L_1018c3: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_1018c4: /* move    #>$20,addresses[1] */
    addresses[1] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018c4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c6: /* move    #$1f,addresses[9] */
    addresses[9] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018c6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c7: /* move    y:(addresses[6]+$2),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 2u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1018c7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c8: /* asr     #$6,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 6u; lows[0] = ((lows[2] >> 6u) | ((uint32_t)accum[2] << 18u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018c8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018c9: /* move    y:(addresses[6]+$3),words[2] */
    words[2] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018c9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ca: /* move    y:(addresses[6]+$a),words[3] */
    words[3] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018cau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018cb: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018cbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018cc: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018ccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018cd: /* move    y:(addresses[6]+$7),words[2] */
    words[2] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018cdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ce: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018ceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018cf: /* nop */

    MD_TRANSLATION_TRACE(0x1018cfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d0: /* move    b,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x1018d0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d1: /* move    y:(addresses[6]+$e),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 14u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1018d1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d2: /* move    y:(addresses[6]+$f),b0 */
    lows[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018d2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d3: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018d3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d4: /* move    a0,words[0] */
    words[0] = (lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018d4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d5: /* do      #<$20,>$1018e0 */
    loop_1018d5 = 0x20u; if (loop_1018d5 == 0u) goto L_1018e0;
    MD_TRANSLATION_TRACE(0x1018d5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d7: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1018d7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018d9: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018d9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018da: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018dau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018db: /* nop */

    MD_TRANSLATION_TRACE(0x1018dbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018dc: /* nop */

    MD_TRANSLATION_TRACE(0x1018dcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018dd: /* move    x:(addresses[2]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[2]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018ddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018df: /* move    words[0],x:(addresses[1])+ */
    *(md_shared_scratch_x + (32u + ((addresses[1]++) & 31u))) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1018dfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1018d5 != 0u) goto L_1018d7;
L_1018e0: /* move    b1,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1018e0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e1: /* move    b0,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x1018e1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e2: /* move    y:(addresses[6]+$1),words[0] */
    words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018e2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e3: /* move    y:(addresses[6]+$8),words[1] */
    words[1] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018e3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e4: /* do      #<$20,>$1018eb */
    loop_1018e4 = 0x20u; if (loop_1018e4 == 0u) goto L_1018eb;
    MD_TRANSLATION_TRACE(0x1018e4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e6: /* move    x:(addresses[1]),words[2] */
    words[2] = (u24(*(md_shared_scratch_x + (32u + ((addresses[1]) & 31u))))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018e6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e7: /* mpy     words[1],words[2],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018e7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e8: /* mpy     words[1],words[0],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018e8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018e9: /* move    a,x:(addresses[1])+ */
    *(md_shared_scratch_x + (32u + ((addresses[1]++) & 31u))) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x1018e9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ea: /* move    b,words[1] */
    words[1] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018eau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1018e4 != 0u) goto L_1018e6;
L_1018eb: /* move    words[1],y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1018ebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ec: /* move    y:(addresses[6]+$4),words[0] */
    words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018ecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ed: /* move    y:(addresses[6]+$5),words[1] */
    words[1] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018edu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ee: /* move    y:(addresses[6]+$c),words[2] */
    words[2] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018eeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ef: /* move    y:(addresses[6]+$d),words[3] */
    words[3] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018efu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f0: /* move    y:(addresses[6]+$6),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 6u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1018f0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f1: /* move    words[0],x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1018f1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f2: /* move    words[1],x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1018f2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f3: /* do      #<$20,>$10190a */
    loop_1018f3 = 0x20u; if (loop_1018f3 == 0u) goto L_10190a;
    MD_TRANSLATION_TRACE(0x1018f3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f5: /* move    x:(addresses[1])+,b */
    accum[1] = sx24((int32_t)(u24(*(md_shared_scratch_x + (32u + ((addresses[1]++) & 31u)))))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1018f5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f6: /* neg     b */
    accum[1] = (int32_t)(0u - (uint32_t)accum[1] - (uint32_t)(lows[1] != 0u)); lows[1] = (0u - lows[1]) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018f6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f7: /* cmp     words[0],b */
    accum[2] = sx24((int32_t)(words[0])); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x1018f7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018f8: /* blt     func_101901 */
    if (flags[1]) goto L_101901;
    MD_TRANSLATION_TRACE(0x1018f8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018fa: /* move    words[2],b */
    accum[1] = sx24((int32_t)(words[2])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1018fau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018fb: /* sub     a,b */
    sub56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1018fbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018fc: /* nop */

    MD_TRANSLATION_TRACE(0x1018fcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018fd: /* move    x:$5,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018fdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018fe: /* move    b,words[2] */
    words[2] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1018feu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1018ff: /* bra     func_101909 */
    goto L_101909;
    MD_TRANSLATION_TRACE(0x1018ffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101901: /* cmp     words[1],b */
    accum[2] = sx24((int32_t)(words[1])); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x101901u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101902: /* bgt     func_101909 */
    if (flags[2]) goto L_101909;
    MD_TRANSLATION_TRACE(0x101902u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101904: /* move    words[3],b */
    accum[1] = sx24((int32_t)(words[3])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x101904u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101905: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x101905u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101906: /* nop */

    MD_TRANSLATION_TRACE(0x101906u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101907: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101907u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101908: /* move    b,words[3] */
    words[3] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x101908u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_101909: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x101909u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1018f3 != 0u) goto L_1018f5;
L_10190a: /* move    y:-(addresses[7]),words[0] */
    words[0] = (md_read_y_raw(v, md_shared_scratch_y, --addresses[7], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10190au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10190b: /* move    words[0],y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x10190bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10190c: /* move    words[2],y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x10190cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10190d: /* move    words[3],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x10190du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10190e: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t trxxt_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2014c2: /* move.l a2, -(a7) */
    /* local register save */
C_2014c4: /* move.l d[2], -(a7) */
    /* local register save */
C_2014c6: /* movea.l $c(a7), a1 */
    /* resolved argument pointer */
C_2014ca: /* movea.l $10(a7), a2 */
    /* resolved argument pointer */
C_2014ce: /* clr.l d[0] */
    d[0] = 0u;
C_2014d0: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_2014d2: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_2014d4: /* addi.l #$60, d[0] */
    lhs = d[0]; rhs = 0x60u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2014da: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2014dc: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_2014de: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_2014e0: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_2014e2: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_2014e8: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_2014ec: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2014ee: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_2014f2: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_2014f6: /* mulu.w #$37, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x37u);
C_2014fa: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_2014fc: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2014fe: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_201500: /* movea.l d[0], a0 */
    a0 = d[0];
C_201502: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_201508: /* move.l $320(a0), $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((800u) + a0)) / 4u)));
C_20150e: /* clr.l d[0] */
    d[0] = 0u;
C_201510: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_201514: /* addq.l #$3, d[0] */
    d[0] = d[0] + 0x3u;
C_201516: /* asr.l #$6, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x6u) & 63u) < 32u ? ((0x6u) & 63u) : 31u));
C_201518: /* move.l d[0], $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_20151c: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_201520: /* mulu.w #$f, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xfu);
C_201524: /* addi.l #$7c2035, d[0] */
    lhs = d[0]; rhs = 0x7c2035u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20152a: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_20152e: /* clr.l d[0] */
    d[0] = 0u;
C_201530: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_201534: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_201536: /* move.l #$7fffff, d[2] */
    d[2] = 0x7fffffu;
C_20153c: /* sub.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_20153e: /* move.l d[2], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[2]);
C_201542: /* addi.l #$400000, d[0] */
    lhs = d[0]; rhs = 0x400000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201548: /* move.l d[0], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_20154c: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_201550: /* mulu.w #$180, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x180u);
C_201554: /* move.l #$7fffff, d[1] */
    d[1] = 0x7fffffu;
C_20155a: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_20155c: /* move.l d[1], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[1]);
C_201560: /* addi.l #$ff800001, d[0] */
    lhs = d[0]; rhs = 0xff800001u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201566: /* move.l d[0], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_20156a: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_20156e: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_201570: /* moveq #$d, d[2] */
    d[2] = 0x0000000du;
C_201572: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_201574: /* moveq #$fe, d[1] */
    d[1] = 0xfffffffeu;
C_201576: /* and.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; d[1] = lhs & rhs;
C_201578: /* move.l d[1], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[1]);
C_20157c: /* tst.l (a1) */
    lhs = (uint32_t)*(y + (0u) / 4u); rhs = 0u;
C_20157e: /* beq.b $201584 */
    if (lhs == rhs) goto C_201584;
C_201580: /* moveq #$a, d[0] */
    d[0] = 0x0000000au;
C_201582: /* bra.b $201586 */
    goto C_201586;
C_201584: /* moveq #$8, d[0] */
    d[0] = 0x00000008u;
C_201586: /* move.l (a7)+, d[2] */
    /* local register restore */
C_201588: /* movea.l (a7)+, a2 */
    /* local register restore */
C_20158a: /* rts  */
    return d[0];
}

void trxxt_update_coeffs(const uint32_t *p, TrxxtVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxxt_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxxt_dsp_init(v, 0); }
}
void trxxt_voice_init(TrxxtVoice *v) {
    uint32_t i;
    for (i = 0u; i < 17u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxxt_voice_trigger(TrxxtVoice *v) {
    trxxt_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxxt_resample_run(TrxxtVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxxtVoice
#define MD_RENDER48 trxxt_render_block_48k_float
#define MD_RENDER44 trxxt_render_block_44k1
#define MD_RESAMPLE_RUN trxxt_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxxtVoice
#define MD_SLOT_INIT trxxt_slot_init
#define MD_SLOT_PROCESS trxxt_slot_process
#define MD_VOICE_INIT trxxt_voice_init
#define MD_COEFFICIENTS trxxt_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxxt_update_coeffs
#define MD_VOICE_TRIGGER trxxt_voice_trigger
#define MD_RENDER44 trxxt_render_block_44k1
#define MD_RENDER48 trxxt_render_block_48k_float
#include "../engine_slot.inc"
#endif
