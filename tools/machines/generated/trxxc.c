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

} TrxxcVoice;
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
extern const uint32_t source_102c0f[34];
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

static uint32_t md_read_x(TrxxcVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1059855u && address < 1059889u) { value = *(source_102c0f + (address - 1059855u)); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(TrxxcVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(TrxxcVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2065u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 1059855u && address < 1059889u) { value = *(source_102c0f + (address - 1059855u)); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(TrxxcVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2065u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(TrxxcVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(TrxxcVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void trxxc_dsp_init(TrxxcVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102b87: /* rts */
    return;
}

void trxxc_dsp_trigger(TrxxcVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102b88: /* move    #>$3,words[0] */
    words[0] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b88u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b8a: /* move    words[0],y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102b8au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b8b: /* move    #>$7ffcb9,words[0] */
    words[0] = (0x7ffcb9u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b8bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b8d: /* move    words[0],y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102b8du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b8e: /* move    #$0,words[0] */
    words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b8eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b8f: /* move    words[0],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102b8fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b90: /* move    words[0],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102b90u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b91: /* move    y:(addresses[6]+$4),words[0] */
    words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b91u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b92: /* move    y:(addresses[6]+$5),words[1] */
    words[1] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b92u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b93: /* move    words[0],y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102b93u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b94: /* move    words[1],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102b94u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b95: /* rts */
    return;
}

void trxxc_render_block_44k1(TrxxcVoice *v, const int32_t *sine) {
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
    uint32_t loop_102baa = 0;
    uint32_t loop_102bb1 = 0;
    uint32_t loop_102bbb = 0;
    uint32_t loop_102bd5 = 0;
    uint32_t loop_102be4 = 0;
    uint32_t loop_102bf3 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_102b96: /* move    y:(addresses[6]+$b),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 11u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102b96u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b97: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b97u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b98: /* beq     func_102bc4 */
    if (flags[0]) goto L_102bc4;
    MD_TRANSLATION_TRACE(0x102b98u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b9a: /* sub     #>$1,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x1u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b9au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b9c: /* nop */

    MD_TRANSLATION_TRACE(0x102b9cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b9d: /* move    a,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x102b9du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b9e: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b9eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b9f: /* beq     func_102bb8 */
    if (flags[0]) goto L_102bb8;
    MD_TRANSLATION_TRACE(0x102b9fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ba1: /* sub     #>$1,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x1u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ba1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ba3: /* nop */

    MD_TRANSLATION_TRACE(0x102ba3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ba4: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ba4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ba5: /* beq     func_102bb1 */
    if (flags[0]) goto L_102bb1;
    MD_TRANSLATION_TRACE(0x102ba5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ba7: /* move    #>$733333,words[0] */
    words[0] = (0x733333u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ba7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ba9: /* move    y:(addresses[6]+$10),words[1] */
    words[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ba9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102baa: /* do      #<$20,>$102bb0 */
    loop_102baa = 0x20u; if (loop_102baa == 0u) goto L_102bb0;
    MD_TRANSLATION_TRACE(0x102baau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bac: /* mpy     words[1],words[0],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bacu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bad: /* nop */

    MD_TRANSLATION_TRACE(0x102badu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bae: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102baeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102baf: /* move    a,words[1] */
    words[1] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bafu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102baa != 0u) goto L_102bac;
L_102bb0: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_102bb1: /* do      #<$10,>$102bb7 */
    loop_102bb1 = 0x10u; if (loop_102bb1 == 0u) goto L_102bb7;
    MD_TRANSLATION_TRACE(0x102bb1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bb3: /* move    #$0,a */
    accum[0] = sx24((int32_t)(((0x0u) << 16))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102bb3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bb4: /* move    #$0,b */
    accum[1] = sx24((int32_t)(((0x0u) << 16))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102bb4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bb5: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102bb5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bb6: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102bb6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102bb1 != 0u) goto L_102bb3;
L_102bb7: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_102bb8: /* move    y:(addresses[6]+$9),words[0] */
    words[0] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bb8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bb9: /* move    #>$102c0f,addresses[0] */
    addresses[0] = (0x102c0fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bb9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bbb: /* do      #<$10,>$102bc3 */
    loop_102bbb = 0x10u; if (loop_102bbb == 0u) goto L_102bc3;
    MD_TRANSLATION_TRACE(0x102bbbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bbd: /* move    y:(addresses[0])+,words[2] */
    words[2] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[0], addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bbdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bbe: /* move    y:(addresses[0])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[0], addresses[8], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bbeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bbf: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bbfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc0: /* mpy     words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bc0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc1: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102bc1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc2: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102bc2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102bbb != 0u) goto L_102bbd;
L_102bc3: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
L_102bc4: /* move    #>$20,addresses[1] */
    addresses[1] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bc4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc6: /* move    #$1f,addresses[9] */
    addresses[9] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bc6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc7: /* move    y:(addresses[6]+$2),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 2u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102bc7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc8: /* asr     #$6,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 6u; lows[0] = ((lows[2] >> 6u) | ((uint32_t)accum[2] << 18u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bc8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bc9: /* move    y:(addresses[6]+$3),words[2] */
    words[2] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bc9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bca: /* move    y:(addresses[6]+$a),words[3] */
    words[3] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bcau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bcb: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bcbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bcc: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bcd: /* move    y:(addresses[6]+$7),words[2] */
    words[2] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bcdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bce: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bcf: /* nop */

    MD_TRANSLATION_TRACE(0x102bcfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd0: /* move    b,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x102bd0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd1: /* move    y:(addresses[6]+$e),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 14u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102bd1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd2: /* move    y:(addresses[6]+$f),b0 */
    lows[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bd2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd3: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bd3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd4: /* move    a0,words[0] */
    words[0] = (lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bd4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd5: /* do      #<$20,>$102be0 */
    loop_102bd5 = 0x20u; if (loop_102bd5 == 0u) goto L_102be0;
    MD_TRANSLATION_TRACE(0x102bd5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd7: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102bd7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bd9: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bd9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bda: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bdau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bdb: /* nop */

    MD_TRANSLATION_TRACE(0x102bdbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bdc: /* nop */

    MD_TRANSLATION_TRACE(0x102bdcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bdd: /* move    x:(addresses[2]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[2]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bdf: /* move    words[0],x:(addresses[1])+ */
    *(md_shared_scratch_x + (32u + ((addresses[1]++) & 31u))) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102bdfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102bd5 != 0u) goto L_102bd7;
L_102be0: /* move    b1,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102be0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be1: /* move    b0,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x102be1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be2: /* move    y:(addresses[6]+$1),words[0] */
    words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102be2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be3: /* move    y:(addresses[6]+$8),words[1] */
    words[1] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102be3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be4: /* do      #<$20,>$102beb */
    loop_102be4 = 0x20u; if (loop_102be4 == 0u) goto L_102beb;
    MD_TRANSLATION_TRACE(0x102be4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be6: /* move    x:(addresses[1]),words[2] */
    words[2] = (u24(*(md_shared_scratch_x + (32u + ((addresses[1]) & 31u))))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102be6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be7: /* mpy     words[1],words[2],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102be7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be8: /* mpy     words[1],words[0],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102be8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102be9: /* move    a,x:(addresses[1])+ */
    *(md_shared_scratch_x + (32u + ((addresses[1]++) & 31u))) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x102be9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bea: /* move    b,words[1] */
    words[1] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102beau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102be4 != 0u) goto L_102be6;
L_102beb: /* move    words[1],y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102bebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bec: /* move    y:(addresses[6]+$4),words[0] */
    words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102becu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bed: /* move    y:(addresses[6]+$5),words[1] */
    words[1] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bedu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bee: /* move    y:(addresses[6]+$c),words[2] */
    words[2] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102beeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bef: /* move    y:(addresses[6]+$d),words[3] */
    words[3] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102befu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf0: /* move    y:(addresses[6]+$6),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 6u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102bf0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf1: /* move    words[0],x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102bf1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf2: /* move    words[1],x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102bf2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf3: /* do      #<$20,>$102c0a */
    loop_102bf3 = 0x20u; if (loop_102bf3 == 0u) goto L_102c0a;
    MD_TRANSLATION_TRACE(0x102bf3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf5: /* move    x:(addresses[1])+,b */
    accum[1] = sx24((int32_t)(u24(*(md_shared_scratch_x + (32u + ((addresses[1]++) & 31u)))))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102bf5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf6: /* neg     b */
    accum[1] = (int32_t)(0u - (uint32_t)accum[1] - (uint32_t)(lows[1] != 0u)); lows[1] = (0u - lows[1]) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bf6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf7: /* cmp     words[0],b */
    accum[2] = sx24((int32_t)(words[0])); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x102bf7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bf8: /* blt     func_102c01 */
    if (flags[1]) goto L_102c01;
    MD_TRANSLATION_TRACE(0x102bf8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bfa: /* move    words[2],b */
    accum[1] = sx24((int32_t)(words[2])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102bfau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bfb: /* sub     a,b */
    sub56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102bfbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bfc: /* nop */

    MD_TRANSLATION_TRACE(0x102bfcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bfd: /* move    x:$5,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bfdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bfe: /* move    b,words[2] */
    words[2] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102bfeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102bff: /* bra     func_102c09 */
    goto L_102c09;
    MD_TRANSLATION_TRACE(0x102bffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c01: /* cmp     words[1],b */
    accum[2] = sx24((int32_t)(words[1])); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x102c01u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c02: /* bgt     func_102c09 */
    if (flags[2]) goto L_102c09;
    MD_TRANSLATION_TRACE(0x102c02u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c04: /* move    words[3],b */
    accum[1] = sx24((int32_t)(words[3])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102c04u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c05: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102c05u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c06: /* nop */

    MD_TRANSLATION_TRACE(0x102c06u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c07: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c07u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c08: /* move    b,words[3] */
    words[3] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c08u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c09: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102c09u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102bf3 != 0u) goto L_102bf5;
L_102c0a: /* move    y:-(addresses[7]),words[0] */
    words[0] = (md_read_y_raw(v, md_shared_scratch_y, --addresses[7], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102c0au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c0b: /* move    words[0],y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102c0bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c0c: /* move    words[2],y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x102c0cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c0d: /* move    words[3],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102c0du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102c0e: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t trxxc_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201d80: /* move.l a2, -(a7) */
    /* local register save */
C_201d82: /* move.l d[2], -(a7) */
    /* local register save */
C_201d84: /* movea.l $c(a7), a1 */
    /* resolved argument pointer */
C_201d88: /* movea.l $10(a7), a2 */
    /* resolved argument pointer */
C_201d8c: /* clr.l d[0] */
    d[0] = 0u;
C_201d8e: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_201d90: /* lsr.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] >> ((0x6u) & 63u) : 0u);
C_201d92: /* move.l d[0], d[1] */
    d[1] = d[0];
C_201d94: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_201d96: /* addq.l #$1, d[1] */
    d[1] = d[1] + 0x1u;
C_201d98: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_201d9a: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_201d9c: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_201da2: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_201da6: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_201da8: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_201dac: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_201db0: /* mulu.w #$28, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x28u);
C_201db4: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_201db6: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_201db8: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_201dba: /* movea.l d[0], a0 */
    a0 = d[0];
C_201dbc: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_201dc2: /* move.l $320(a0), $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((800u) + a0)) / 4u)));
C_201dc8: /* clr.l d[0] */
    d[0] = 0u;
C_201dca: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_201dce: /* addq.l #$3, d[0] */
    d[0] = d[0] + 0x3u;
C_201dd0: /* asr.l #$6, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x6u) & 63u) < 32u ? ((0x6u) & 63u) : 31u));
C_201dd2: /* move.l d[0], $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_201dd6: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_201dda: /* mulu.w #$f, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xfu);
C_201dde: /* addi.l #$7c2035, d[0] */
    lhs = d[0]; rhs = 0x7c2035u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201de4: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_201de8: /* clr.l d[0] */
    d[0] = 0u;
C_201dea: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_201dee: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_201df0: /* move.l #$7fffff, d[2] */
    d[2] = 0x7fffffu;
C_201df6: /* sub.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[2] = lhs - rhs;
C_201df8: /* move.l d[2], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[2]);
C_201dfc: /* addi.l #$400000, d[0] */
    lhs = d[0]; rhs = 0x400000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201e02: /* move.l d[0], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_201e06: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_201e0a: /* mulu.w #$180, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x180u);
C_201e0e: /* move.l #$7fffff, d[1] */
    d[1] = 0x7fffffu;
C_201e14: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_201e16: /* move.l d[1], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[1]);
C_201e1a: /* addi.l #$ff800001, d[0] */
    lhs = d[0]; rhs = 0xff800001u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201e20: /* move.l d[0], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_201e24: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_201e28: /* mulu.w d[0], d[0] */
    d[0] = (d[0] & 0xffffu) * ((d[0] & 0xffffu));
C_201e2a: /* moveq #$d, d[2] */
    d[2] = 0x0000000du;
C_201e2c: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_201e2e: /* moveq #$fe, d[1] */
    d[1] = 0xfffffffeu;
C_201e30: /* and.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; d[1] = lhs & rhs;
C_201e32: /* move.l d[1], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[1]);
C_201e36: /* tst.l (a1) */
    lhs = (uint32_t)*(y + (0u) / 4u); rhs = 0u;
C_201e38: /* beq.b $201e3e */
    if (lhs == rhs) goto C_201e3e;
C_201e3a: /* moveq #$a, d[0] */
    d[0] = 0x0000000au;
C_201e3c: /* bra.b $201e40 */
    goto C_201e40;
C_201e3e: /* moveq #$8, d[0] */
    d[0] = 0x00000008u;
C_201e40: /* move.l (a7)+, d[2] */
    /* local register restore */
C_201e42: /* movea.l (a7)+, a2 */
    /* local register restore */
C_201e44: /* rts  */
    return d[0];
}

void trxxc_update_coeffs(const uint32_t *p, TrxxcVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxxc_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxxc_dsp_init(v, 0); }
}
void trxxc_voice_init(TrxxcVoice *v) {
    uint32_t i;
    for (i = 0u; i < 17u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxxc_voice_trigger(TrxxcVoice *v) {
    trxxc_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxxc_resample_run(TrxxcVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxxcVoice
#define MD_RENDER48 trxxc_render_block_48k_float
#define MD_RENDER44 trxxc_render_block_44k1
#define MD_RESAMPLE_RUN trxxc_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxxcVoice
#define MD_SLOT_INIT trxxc_slot_init
#define MD_SLOT_PROCESS trxxc_slot_process
#define MD_VOICE_INIT trxxc_voice_init
#define MD_COEFFICIENTS trxxc_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxxc_update_coeffs
#define MD_VOICE_TRIGGER trxxc_voice_trigger
#define MD_RENDER44 trxxc_render_block_44k1
#define MD_RENDER48 trxxc_render_block_48k_float
#include "../engine_slot.inc"
#endif
