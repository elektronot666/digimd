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
    int32_t y[29];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmrsVoice;
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

static uint32_t md_read_x(EfmrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2077u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmrsVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2077u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmrsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmrsVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void efmrs_dsp_init(EfmrsVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1031a3: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031a3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a4: /* nop */

    MD_TRANSLATION_TRACE(0x1031a4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a5: /* move    a1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031a5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a6: /* move    a1,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031a6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a7: /* rts */
    return;
}

void efmrs_dsp_trigger(EfmrsVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1031a8: /* move    #>$2000,a */
    accum[0] = sx24((int32_t)(0x2000u)); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1031a8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031aa: /* move    #>$50c335,words[1] */
    words[1] = (0x50c335u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031aau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ac: /* move    a1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031acu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ad: /* move    a0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031adu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ae: /* move    a1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031aeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031af: /* move    a0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031afu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b0: /* move    a1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031b0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b1: /* move    a0,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031b1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b2: /* move    a1,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031b2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b3: /* move    a0,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031b3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b4: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031b4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b5: /* move    a0,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031b5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b6: /* move    a0,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031b6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b7: /* move    a0,y:(addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1031b7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b8: /* move    words[1],y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1031b8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031b9: /* move    words[1],y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1031b9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ba: /* rts */
    return;
}

void efmrs_render_block_44k1(EfmrsVoice *v, const int32_t *sine) {
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
    uint32_t loop_1031ec = 0;
    uint32_t loop_10324c = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_1031bb: /* move    y:(addresses[6]+$1),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 1u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031bbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031bc: /* move    y:(addresses[6]+$2),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 2u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031bcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031bd: /* move    a1,x:$1 */
    *(md_shared_scratch_x + 1u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031bdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031be: /* move    b1,x:$2 */
    *(md_shared_scratch_x + 2u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031beu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031bf: /* move    y:(addresses[6]+$3),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 3u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031bfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c0: /* move    y:(addresses[6]+$4),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 4u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031c0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c1: /* move    a1,x:$3 */
    *(md_shared_scratch_x + 3u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031c1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c2: /* move    b1,x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031c2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c3: /* move    y:(addresses[6]+$5),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 5u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031c3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c4: /* move    y:(addresses[6]+$6),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 6u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031c4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c5: /* move    a1,x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031c5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c6: /* move    b1,x:$6 */
    *(md_shared_scratch_x + 6u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031c6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c7: /* move    y:(addresses[6]+$7),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 7u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031c7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c8: /* move    y:(addresses[6]+$8),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 8u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031c8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031c9: /* move    a1,x:$7 */
    *(md_shared_scratch_x + 7u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031c9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ca: /* move    b1,x:$8 */
    *(md_shared_scratch_x + 8u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031cau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031cb: /* move    y:(addresses[6]+$9),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 9u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031cbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031cc: /* move    y:(addresses[6]+$a),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 10u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031ccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031cd: /* move    a1,x:$9 */
    *(md_shared_scratch_x + 9u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031cdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ce: /* move    b1,x:$a */
    *(md_shared_scratch_x + 10u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031ceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031cf: /* move    y:(addresses[6]+$b),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 11u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031cfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d0: /* move    y:(addresses[6]+$c),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 12u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031d0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d1: /* move    a1,x:$b */
    *(md_shared_scratch_x + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031d1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d2: /* move    b1,x:$c */
    *(md_shared_scratch_x + 12u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031d2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d3: /* move    y:(addresses[6]+$d),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 13u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1031d3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d4: /* move    y:(addresses[6]+$e),a0 */
    lows[0] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031d4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d5: /* move    y:(addresses[6]+$f),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 15u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1031d5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d6: /* move    y:(addresses[6]+$10),b0 */
    lows[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031d6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d7: /* move    a,l:<$d */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031d7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d8: /* move    b,l:<$f */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[15] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[15] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031d8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031d9: /* move    y:(addresses[6]+$11),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 17u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1031d9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031da: /* move    y:(addresses[6]+$12),a0 */
    lows[0] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031dau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031db: /* move    y:(addresses[6]+$13),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 19u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1031dbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031dc: /* move    y:(addresses[6]+$14),b0 */
    lows[1] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031dcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031dd: /* move    a,l:<$11 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[17] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[17] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031ddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031de: /* move    b,l:<$13 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[19] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[19] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031deu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031df: /* move    y:(addresses[6]+$15),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031dfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e0: /* move    y:(addresses[6]+$16),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 22u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1031e0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e1: /* move    a1,x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1031e1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e2: /* move    b1,x:$16 */
    *(md_shared_scratch_x + 22u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031e2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e3: /* move    y:(addresses[6]+$17),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 23u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1031e3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e4: /* move    y:(addresses[6]+$18),a0 */
    lows[0] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031e4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e5: /* move    y:(addresses[6]+$19),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 25u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1031e5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e6: /* move    y:(addresses[6]+$1a),b0 */
    lows[1] = (u24(*(v->y + 26u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031e6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e7: /* move    y:(addresses[6]+$1b),words[1] */
    words[1] = (u24(*(v->y + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031e7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e8: /* move    a,l:<$17 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[23] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[23] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031e8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031e9: /* move    b,l:<$19 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[25] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[25] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031e9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ea: /* move    words[1],x:$1b */
    *(md_shared_scratch_x + 27u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1031eau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031eb: /* move    addresses[7],addresses[1] */
    addresses[1] = (addresses[7]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031ebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ec: /* do      #<$20,>$103239 */
    loop_1031ec = 0x20u; if (loop_1031ec == 0u) goto L_103239;
    MD_TRANSLATION_TRACE(0x1031ecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ee: /* move    x:$2,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031eeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ef: /* move    x:$1,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031efu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f0: /* mpy     words[3],words[2],a         l:<$17,x */
    parallel_words[0] = u24(md_shared_scratch_x[23]); parallel_words[3] = u24(md_shared_scratch_y[23]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1031f0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f1: /* asr     #$5,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 5u; lows[0] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031f1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f2: /* move    l:<$d,b */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1031f2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f3: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031f3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f4: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1031f4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f6: /* move    b1,addresses[0] */
    addresses[0] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031f6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f7: /* sub     x,b */
    sub56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031f7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f8: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031f8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031f9: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031f9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031fa: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031fau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031fc: /* move    b10,l:<$d */
    parallel_words[0] = (uint32_t)(accum[1]); parallel_words[3] = lows[1]; md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1031fcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031fd: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031fdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031fe: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1031feu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031ff: /* move    x:$3,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1031ffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103200: /* move    a,l:<$17 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[23] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[23] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103200u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103201: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103201u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103202: /* move    l:<$f,a */
    parallel_words[0] = u24(md_shared_scratch_x[15]); parallel_words[3] = u24(md_shared_scratch_y[15]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103202u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103203: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103203u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103204: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103204u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103205: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103205u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103206: /* add     words[3],b            x:$15,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 21u)); add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103206u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103207: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103207u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103208: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103208u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103209: /* add     b,a             x:$9,words[3] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 9u)); add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103209u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10320a: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x10320au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10320c: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10320cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10320e: /* move    a10,l:<$f */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[15] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[15] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x10320eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10320f: /* mpy     words[3],words[2],b         x:$5,words[3] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 5u)); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10320fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103210: /* mpyr    words[2],words[0],a         x:$6,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 6u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103210u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103211: /* move    b1,x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103211u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103212: /* move    a1,x:$1d */
    *(md_shared_scratch_x + 29u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103212u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103213: /* mpy     words[3],words[2],a         l:<$19,x */
    parallel_words[0] = u24(md_shared_scratch_x[25]); parallel_words[3] = u24(md_shared_scratch_y[25]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103213u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103214: /* asr     #$5,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 5u; lows[0] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103214u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103215: /* move    l:<$11,b */
    parallel_words[0] = u24(md_shared_scratch_x[17]); parallel_words[3] = u24(md_shared_scratch_y[17]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103215u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103216: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103216u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103217: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x103217u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103219: /* move    b1,addresses[0] */
    addresses[0] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103219u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10321a: /* sub     x,b */
    sub56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10321au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10321b: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10321bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10321c: /* move    x:$8,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10321cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10321d: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10321du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10321f: /* move    b10,l:<$11 */
    parallel_words[0] = (uint32_t)(accum[1]); parallel_words[3] = lows[1]; md_shared_scratch_x[17] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[17] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x10321fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103220: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103220u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103221: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103221u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103222: /* move    x:$7,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103222u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103223: /* move    a,l:<$19 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[25] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[25] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103223u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103224: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103224u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103225: /* move    l:<$13,a */
    parallel_words[0] = u24(md_shared_scratch_x[19]); parallel_words[3] = u24(md_shared_scratch_y[19]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103225u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103226: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103226u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103227: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103227u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103228: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103228u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103229: /* add     words[3],b            x:$16,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 22u)); add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103229u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10322a: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10322au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10322b: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10322bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10322c: /* add     b,a             x:$a,words[3] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 10u)); add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10322cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10322d: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x10322du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10322f: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10322fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103231: /* move    a10,l:<$13 */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[19] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[19] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103231u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103232: /* mpy     words[3],words[2],b         x:$b,words[3] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 11u)); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103232u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103233: /* mpyr    words[2],words[0],a         x:$1d,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 29u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103233u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103234: /* mpyr    words[3],words[1],b         b1,x:$16 */
    parallel_words[0] = u24(accum[1]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); *(md_shared_scratch_x + 22u) = sx24((int32_t)(parallel_words[0]));
    MD_TRANSLATION_TRACE(0x103234u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103235: /* nop */

    MD_TRANSLATION_TRACE(0x103235u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103236: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103236u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103237: /* nop */

    MD_TRANSLATION_TRACE(0x103237u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103238: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x103238u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1031ec != 0u) goto L_1031ee;
L_103239: /* move    addresses[1],addresses[7] */
    addresses[7] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103239u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10323a: /* move    addresses[1],addresses[4] */
    addresses[4] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10323au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10323b: /* move    #$1b,addresses[2] */
    addresses[2] = (0x1bu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10323bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10323c: /* move    y:(addresses[6]+$c),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 12u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10323cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10323d: /* asr     #$1,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = accum[2] >> 1u; lows[1] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10323du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10323e: /* move    #$40,words[0] */
    words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10323eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10323f: /* add     words[0],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10323fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103240: /* move    a1,words[2] */
    words[2] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103240u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103241: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103241u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103242: /* move    y:(addresses[7])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103242u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103243: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103243u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103244: /* move    x:(addresses[2]),words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103244u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103245: /* move    y:(addresses[6]+$1c),words[3] */
    words[3] = (u24(*(v->y + 28u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103245u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103246: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103246u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103247: /* macr    words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103247u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103248: /* mpy     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103248u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103249: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103249u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10324a: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10324au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10324b: /* macr    words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10324bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10324c: /* do      #<$f,>$103256 */
    loop_10324c = 0xfu; if (loop_10324c == 0u) goto L_103256;
    MD_TRANSLATION_TRACE(0x10324cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10324e: /* mpy     words[3],words[1],a         a,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[0])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10324eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10324f: /* move    x:(addresses[2]),words[0]       b,words[3] */
    parallel_words[0] = u24(lim24(accum[1])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10324fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103250: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103250u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103251: /* mac     words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103251u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103252: /* mpy     words[3],words[1],b         b,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[1])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103252u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103253: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103253u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103254: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103254u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103255: /* mac     words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103255u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_10324c != 0u) goto L_10324e;
L_103256: /* move    a,y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x103256u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103257: /* move    b,y:(addresses[4]) */
    md_write_y(v, md_shared_scratch_y, addresses[4], sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x103257u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103258: /* move    b,y:(addresses[6]+$1c) */
    *(v->y + 28u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x103258u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103259: /* move    l:<$d,a */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103259u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10325a: /* move    l:<$f,b */
    parallel_words[0] = u24(md_shared_scratch_x[15]); parallel_words[3] = u24(md_shared_scratch_y[15]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10325au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10325b: /* move    a1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10325bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10325c: /* move    a0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10325cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10325d: /* move    b1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10325du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10325e: /* move    b0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x10325eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10325f: /* move    l:<$11,a */
    parallel_words[0] = u24(md_shared_scratch_x[17]); parallel_words[3] = u24(md_shared_scratch_y[17]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10325fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103260: /* move    l:<$13,b */
    parallel_words[0] = u24(md_shared_scratch_x[19]); parallel_words[3] = u24(md_shared_scratch_y[19]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103260u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103261: /* move    a1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103261u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103262: /* move    a0,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103262u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103263: /* move    b1,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103263u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103264: /* move    b0,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x103264u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103265: /* move    x:$15,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103265u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103266: /* move    x:$16,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 22u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103266u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103267: /* move    a1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103267u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103268: /* move    b1,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103268u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103269: /* move    l:<$17,a */
    parallel_words[0] = u24(md_shared_scratch_x[23]); parallel_words[3] = u24(md_shared_scratch_y[23]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103269u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10326a: /* move    l:<$19,b */
    parallel_words[0] = u24(md_shared_scratch_x[25]); parallel_words[3] = u24(md_shared_scratch_y[25]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10326au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10326b: /* move    x:$1b,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10326bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10326c: /* move    a1,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10326cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10326d: /* move    a0,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10326du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10326e: /* move    b1,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10326eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10326f: /* move    b0,y:(addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x10326fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103270: /* move    words[1],y:(addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103270u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103271: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t efmrs_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2024d0: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_2024d4: /* movem.l d[2]/a2-a3, (a7) */
    /* native locals replace original callee saves */
C_2024d8: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_2024dc: /* movea.l $14(a7), a2 */
    /* resolved argument pointer */
C_2024e0: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_2024e4: /* mulu.w #$5, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x5u);
C_2024e8: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2024ea: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2024f0: /* lea.l $24bd94.l, a3 */
    /* resolved table pointer */
C_2024f6: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2024f8: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_2024fa: /* addq.l #$3, d[1] */
    d[1] = d[1] + 0x3u;
C_2024fc: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_2024fe: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202500: /* move.l (a3, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_202504: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202506: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_20250a: /* moveq #$60, d[0] */
    d[0] = 0x00000060u;
C_20250c: /* swap d[0] */
    d[0] = (d[0] << 16) | (d[0] >> 16);
C_20250e: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_202512: /* lea.l $c(a1), a0 */
    a0 = 12u;
C_202516: /* clr.l d[0] */
    d[0] = 0u;
C_202518: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_20251c: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_20251e: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202520: /* move.l d[0], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[0]);
C_202522: /* cmpi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu;
C_202528: /* bgt.b $202530 */
    if ((int32_t)lhs > (int32_t)rhs) goto C_202530;
C_20252a: /* move.l #$10000, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x10000u);
C_202530: /* move.l #$7a1200, d[2] */
    d[2] = 0x7a1200u;
C_202536: /* move.l d[2], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[2]);
C_20253a: /* clr.l d[0] */
    d[0] = 0u;
C_20253c: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20253e: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_202540: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202546: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202548: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_20254a: /* addq.l #$4, d[1] */
    d[1] = d[1] + 0x4u;
C_20254c: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_20254e: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202550: /* move.l (a3, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_202554: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202556: /* move.l d[0], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_20255a: /* moveq #$60, d[0] */
    d[0] = 0x00000060u;
C_20255c: /* swap d[0] */
    d[0] = (d[0] << 16) | (d[0] >> 16);
C_20255e: /* move.l d[0], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_202562: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_202566: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_202568: /* andi.l #$7fff80, d[0] */
    lhs = d[0]; rhs = 0x7fff80u; d[0] = lhs & rhs;
C_20256e: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_202572: /* move.l #$7a1200, d[1] */
    d[1] = 0x7a1200u;
C_202578: /* move.l d[1], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[1]);
C_20257c: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_202580: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_202584: /* move.b #$c, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0xcu) & 0xffu);
C_202588: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_20258a: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20258c: /* movea.l d[0], a0 */
    a0 = d[0];
C_20258e: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202594: /* move.l $24c(a0), $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20259a: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_20259e: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_2025a2: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_2025a4: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2025a6: /* movea.l d[0], a0 */
    a0 = d[0];
C_2025a8: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2025ae: /* move.l $24c(a0), $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_2025b4: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_2025b8: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_2025ba: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2025bc: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_2025c2: /* move.l d[0], $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(d[0]);
C_2025c6: /* clr.l d[0] */
    d[0] = 0u;
C_2025c8: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_2025cc: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_2025ce: /* move.l #$7fffff, d[2] */
    d[2] = 0x7fffffu;
C_2025d4: /* sub.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_2025d6: /* move.l d[2], $30(a1) */
    *(y + (48u) / 4u) = (int32_t)(d[2]);
C_2025da: /* moveq #$d, d[0] */
    d[0] = 0x0000000du;
C_2025dc: /* movem.l (a7), d[2]/a2-a3 */
    /* native locals replace original callee saves */
C_2025e0: /* lea.l $c(a7), a7 */
    /* local register save area */
C_2025e4: /* rts  */
    return d[0];
}

void efmrs_update_coeffs(const uint32_t *p, EfmrsVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmrs_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmrs_dsp_init(v, 0); }
}
void efmrs_voice_init(EfmrsVoice *v) {
    uint32_t i;
    for (i = 0u; i < 29u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmrs_voice_trigger(EfmrsVoice *v) {
    efmrs_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmrs_resample_run(EfmrsVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmrsVoice
#define MD_RENDER48 efmrs_render_block_48k_float
#define MD_RENDER44 efmrs_render_block_44k1
#define MD_RESAMPLE_RUN efmrs_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmrsVoice
#define MD_SLOT_INIT efmrs_slot_init
#define MD_SLOT_PROCESS efmrs_slot_process
#define MD_VOICE_INIT efmrs_voice_init
#define MD_COEFFICIENTS efmrs_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmrs_update_coeffs
#define MD_VOICE_TRIGGER efmrs_voice_trigger
#define MD_RENDER44 efmrs_render_block_44k1
#define MD_RENDER48 efmrs_render_block_48k_float
#include "../engine_slot.inc"
#endif
