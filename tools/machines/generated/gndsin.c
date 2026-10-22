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
    int32_t y[11];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} GndsinVoice;
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

static uint32_t md_read_x(GndsinVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 32u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(GndsinVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 32u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(GndsinVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 1u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2059u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(GndsinVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 1u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2059u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(GndsinVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(GndsinVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void gndsin_dsp_init(GndsinVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1000a3: /* rts */
    return;
}

void gndsin_dsp_trigger(GndsinVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1000a4: /* move    #>$7ffff8,words[0] */
    words[0] = (0x7ffff8u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000a4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000a6: /* move    words[0],y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000a6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000a7: /* move    words[0],y:(addresses[6]+$6) */
    *(v->y + 6u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000a7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000a8: /* move    #$0,words[0] */
    words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000a8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000a9: /* move    words[0],y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000a9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000aa: /* move    words[0],y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000aau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ab: /* move    words[0],y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000abu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ac: /* rts */
    return;
}

void gndsin_render_block_44k1(GndsinVoice *v, const int32_t *sine) {
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
    uint32_t loop_1000be = 0;
    uint32_t loop_1000d0 = 0;
L_1000ad: /* move    #>$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000adu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000af: /* move    #$1f,addresses[12] */
    addresses[12] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000afu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b0: /* move    y:(addresses[6]+$1),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 1u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1000b0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b1: /* move    y:(addresses[6]+$3),words[2] */
    words[2] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000b1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b2: /* move    y:(addresses[6]+$6),words[3] */
    words[3] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000b2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b3: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000b3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b4: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000b4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b5: /* asr     #$6,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 6u; lows[0] = ((lows[2] >> 6u) | ((uint32_t)accum[2] << 18u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000b5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b6: /* move    y:(addresses[6]+$4),words[2] */
    words[2] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000b6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b7: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000b7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b8: /* nop */

    MD_TRANSLATION_TRACE(0x1000b8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000b9: /* move    b,y:(addresses[6]+$6) */
    *(v->y + 6u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x1000b9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ba: /* move    y:(addresses[6]+$7),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 7u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1000bau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000bb: /* move    y:(addresses[6]+$8),b0 */
    lows[1] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000bbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000bc: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000bcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000bd: /* move    a0,words[0] */
    words[0] = (lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000bdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000be: /* do      #<$20,>$1000cb */
    loop_1000be = 0x20u; if (loop_1000be == 0u) goto L_1000cb;
    MD_TRANSLATION_TRACE(0x1000beu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c0: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1000c0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c2: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000c2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c3: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000c3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c4: /* nop */

    MD_TRANSLATION_TRACE(0x1000c4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c5: /* nop */

    MD_TRANSLATION_TRACE(0x1000c5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c6: /* move    x:(addresses[2]+$148000),a */
    accum[0] = sx24((int32_t)(u24(*(sine + addresses[2])))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1000c6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c8: /* asr     #$2,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 2u; lows[0] = ((lows[2] >> 2u) | ((uint32_t)accum[2] << 22u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000c8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000c9: /* nop */

    MD_TRANSLATION_TRACE(0x1000c9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ca: /* move    a,x:(addresses[4])+ */
    *(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x1000cau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1000be != 0u) goto L_1000c0;
L_1000cb: /* move    b1,y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1000cbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000cc: /* move    b0,y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x1000ccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000cd: /* move    y:(addresses[6]+$2),words[3] */
    words[3] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000cdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000ce: /* move    y:(addresses[6]+$a),words[1] */
    words[1] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000ceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000cf: /* move    y:(addresses[6]+$9),words[0] */
    words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000cfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d0: /* do      #<$20,>$1000da */
    loop_1000d0 = 0x20u; if (loop_1000d0 == 0u) goto L_1000da;
    MD_TRANSLATION_TRACE(0x1000d0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d2: /* move    x:(addresses[4])+,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000d2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d3: /* mpysu   words[2],words[0],a */
    mpy_su32(sx24((int32_t)(words[2])), words[0], &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000d3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d4: /* dmac    ss words[2],words[1],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); lows[0] = u24(accum[0]); accum[0] = accum[0] >> 24; add56(accum[2], lows[2], accum[0], lows[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000d4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d5: /* mpysu   words[3],words[0],b */
    mpy_su32(sx24((int32_t)(words[3])), words[0], &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000d5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d6: /* dmac    ss words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); lows[1] = u24(accum[1]); accum[1] = accum[1] >> 24; add56(accum[2], lows[2], accum[1], lows[1], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1000d6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d7: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x1000d7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d8: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000d8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000d9: /* move    b0,words[0] */
    words[0] = (lows[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1000d9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1000d0 != 0u) goto L_1000d2;
L_1000da: /* move    words[1],y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1000dau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000db: /* move    words[0],y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1000dbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1000dc: /* rts */
    return;
}

uint32_t gndsin_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201130: /* move.l a2, -(a7) */
    /* local register save */
C_201132: /* move.l d[2], -(a7) */
    /* local register save */
C_201134: /* movea.l $c(a7), a2 */
    /* resolved argument pointer */
C_201138: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_20113c: /* clr.l d[0] */
    d[0] = 0u;
C_20113e: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_201140: /* lsr.l #$4, d[0] */
    d[0] = (((0x4u) & 63u) < 32u ? d[0] >> ((0x4u) & 63u) : 0u);
C_201142: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201144: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_201146: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_201148: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_20114a: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_201150: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_201154: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_201156: /* move.l d[0], $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_20115a: /* clr.l d[0] */
    d[0] = 0u;
C_20115c: /* move.w $2(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_201160: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_201162: /* lea.l $24bf94.l, a0 */
    /* resolved table pointer */
C_201168: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((1028u + (d[0] * 4u + (0))) / 4u));
C_20116c: /* move.l d[0], $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_201170: /* move.w $4(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_201174: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_201176: /* asr.l #$7, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_201178: /* move.l d[0], $c(a2) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_20117c: /* clr.l d[0] */
    d[0] = 0u;
C_20117e: /* move.w $6(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_201182: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_201184: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((1028u + (d[0] * 4u + (0))) / 4u));
C_201188: /* move.l d[0], $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20118c: /* moveq #$5, d[0] */
    d[0] = 0x00000005u;
C_20118e: /* move.l (a7)+, d[2] */
    /* local register restore */
C_201190: /* movea.l (a7)+, a2 */
    /* local register restore */
C_201192: /* rts  */
    return d[0];
}

void gndsin_update_coeffs(const uint32_t *p, GndsinVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = gndsin_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { gndsin_dsp_init(v, 0); }
}
void gndsin_voice_init(GndsinVoice *v) {
    uint32_t i;
    for (i = 0u; i < 11u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void gndsin_voice_trigger(GndsinVoice *v) {
    gndsin_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t gndsin_resample_run(GndsinVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE GndsinVoice
#define MD_RENDER48 gndsin_render_block_48k_float
#define MD_RENDER44 gndsin_render_block_44k1
#define MD_RESAMPLE_RUN gndsin_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE GndsinVoice
#define MD_SLOT_INIT gndsin_slot_init
#define MD_SLOT_PROCESS gndsin_slot_process
#define MD_VOICE_INIT gndsin_voice_init
#define MD_COEFFICIENTS gndsin_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER gndsin_update_coeffs
#define MD_VOICE_TRIGGER gndsin_voice_trigger
#define MD_RENDER44 gndsin_render_block_44k1
#define MD_RENDER48 gndsin_render_block_48k_float
#include "../engine_slot.inc"
#endif
