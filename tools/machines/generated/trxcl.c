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
    int32_t y[20];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} TrxclVoice;
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

static uint32_t md_read_source(TrxclVoice *v, uint32_t address) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address >= 0x20000u && address < 0x200000u) { value = md_source_words[address - 0x20000u]; }
    else if (address >= 0x200000u && address < 0xffff80u) { value = 0u; }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
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

static uint32_t md_read_x(TrxclVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(TrxclVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(TrxclVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2068u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else if (address >= 0x20000u && address < 0x200000u) { value = md_read_source(v, address); }
    else if (address >= 0x200000u && address < 0xffff80u) { value = md_read_source(v, address); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(TrxclVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2068u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(TrxclVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(TrxclVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_source_raw(TrxclVoice *v,uint32_t address) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_source(v,address);
    return value;
}
void trxcl_dsp_init(TrxclVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102aa9: /* move    #>$0,words[0] */
    words[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102aa9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aab: /* move    words[0],y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102aabu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aac: /* move    words[0],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102aacu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aad: /* move    words[0],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102aadu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aae: /* move    words[0],y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102aaeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aaf: /* move    words[0],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102aafu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab0: /* move    words[0],y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102ab0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab1: /* move    words[0],y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102ab1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab2: /* rts */
    return;
}

void trxcl_dsp_trigger(TrxclVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_102ab3: /* move    #>$50c335,b */
    accum[1] = sx24((int32_t)(0x50c335u)); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102ab3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab5: /* move    y:(addresses[6]+$9),words[1] */
    words[1] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ab5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab6: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ab6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab7: /* nop */

    MD_TRANSLATION_TRACE(0x102ab7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab8: /* move    b,y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x102ab8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ab9: /* move    #$0,words[0] */
    words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ab9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aba: /* move    words[0],y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102abau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102abb: /* move    words[0],y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102abbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102abc: /* move    words[0],y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102abcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102abd: /* move    words[0],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102abdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102abe: /* rts */
    return;
}

void trxcl_render_block_44k1(TrxclVoice *v, const int32_t *sine) {
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
    uint32_t loop_102ad0 = 0;
    uint32_t loop_102ada = 0;
    uint32_t loop_102aec = 0;
    uint32_t loop_102afa = 0;
    uint32_t loop_102b0d = 0;
    uint32_t loop_102b1b = 0;
    uint32_t loop_102b23 = 0;
    uint32_t loop_102b64 = 0;
    uint32_t loop_102b80 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_102abf: /* move    #>$0,addresses[1] */
    addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102abfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ac1: /* move    #>$1f,addresses[9] */
    addresses[9] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ac1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ac3: /* move    #>$20,addresses[3] */
    addresses[3] = (0x20u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ac3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ac5: /* move    #>$1f,addresses[11] */
    addresses[11] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ac5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ac7: /* move    y:(addresses[6]+$2),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 2u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102ac7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ac8: /* tst     a */
    flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ac8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ac9: /* beq     func_102af4 */
    if (flags[0]) goto L_102af4;
    MD_TRANSLATION_TRACE(0x102ac9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102acb: /* cmp     #>$20,a */
    accum[2] = sx24((int32_t)(0x20u)); lows[2] = 0u; flags[0] = (accum[0] == accum[2] && lows[0] == lows[2]); flags[1] = (accum[0] < accum[2] || (accum[0] == accum[2] && lows[0] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x102acbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102acd: /* bgt     func_102aeb */
    if (flags[2]) goto L_102aeb;
    MD_TRANSLATION_TRACE(0x102acdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102acf: /* move    y:(addresses[6]+$8),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 8u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102acfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad0: /* do      a,>$102ad3 */
    loop_102ad0 = u24(lim24(accum[0])); if (loop_102ad0 == 0u) goto L_102ad3;
    MD_TRANSLATION_TRACE(0x102ad0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad2: /* move    b,x:(addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102ad2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102ad0 != 0u) goto L_102ad2;
L_102ad3: /* sub     #>$20,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x20u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ad3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad5: /* neg     a */
    accum[0] = (int32_t)(0u - (uint32_t)accum[0] - (uint32_t)(lows[0] != 0u)); lows[0] = (0u - lows[0]) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102ad5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad6: /* move    y:(addresses[6]+$a),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 10u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102ad6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad7: /* move    y:(addresses[6]+$b),b0 */
    lows[1] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ad7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad8: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ad8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ad9: /* move    a0,words[0] */
    words[0] = (lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ad9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ada: /* do      a,>$102ae5 */
    loop_102ada = u24(lim24(accum[0])); if (loop_102ada == 0u) goto L_102ae5;
    MD_TRANSLATION_TRACE(0x102adau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102adc: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102adcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ade: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102adeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102adf: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102adfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae0: /* nop */

    MD_TRANSLATION_TRACE(0x102ae0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae1: /* nop */

    MD_TRANSLATION_TRACE(0x102ae1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae2: /* move    x:(addresses[2]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[2]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ae2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae4: /* move    words[0],x:(addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, words[0]);
    MD_TRANSLATION_TRACE(0x102ae4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102ada != 0u) goto L_102adc;
L_102ae5: /* move    b1,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102ae5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae6: /* move    b0,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x102ae6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae7: /* move    #$0,words[0] */
    words[0] = (((0x0u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102ae7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae8: /* move    words[0],y:(addresses[6]+$2) */
    *(v->y + 2u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x102ae8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102ae9: /* bra     func_102b07 */
    goto L_102b07;
    MD_TRANSLATION_TRACE(0x102ae9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aeb: /* move    y:(addresses[6]+$8),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 8u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102aebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aec: /* do      #<$20,>$102aef */
    loop_102aec = 0x20u; if (loop_102aec == 0u) goto L_102aef;
    MD_TRANSLATION_TRACE(0x102aecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aee: /* move    b,x:(addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102aeeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102aec != 0u) goto L_102aee;
L_102aef: /* sub     #<$20,a */
    sub56(accum[0], lows[0], sx24((int32_t)(0x20u)), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102aefu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af0: /* nop */

    MD_TRANSLATION_TRACE(0x102af0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af1: /* move    a,y:(addresses[6]+$2) */
    *(v->y + 2u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x102af1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af2: /* bra     func_102b07 */
    goto L_102b07;
    MD_TRANSLATION_TRACE(0x102af2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af4: /* move    y:(addresses[6]+$3),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 3u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102af4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af5: /* asr     #$1,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 1u; lows[0] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102af5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af6: /* move    y:(addresses[6]+$a),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 10u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102af6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af7: /* move    y:(addresses[6]+$b),b0 */
    lows[1] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102af7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af8: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102af8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102af9: /* move    a0,words[0] */
    words[0] = (lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102af9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102afa: /* do      #<$20,>$102b05 */
    loop_102afa = 0x20u; if (loop_102afa == 0u) goto L_102b05;
    MD_TRANSLATION_TRACE(0x102afau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102afc: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102afcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102afe: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102afeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102aff: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102affu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b00: /* nop */

    MD_TRANSLATION_TRACE(0x102b00u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b01: /* nop */

    MD_TRANSLATION_TRACE(0x102b01u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b02: /* move    x:(addresses[2]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[2]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b02u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b04: /* move    words[0],x:(addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, words[0]);
    MD_TRANSLATION_TRACE(0x102b04u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102afa != 0u) goto L_102afc;
L_102b05: /* move    b1,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102b05u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b06: /* move    b0,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x102b06u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b07: /* move    y:(addresses[6]+$4),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 4u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102b07u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b08: /* asr     #$1,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 1u; lows[0] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b08u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b09: /* move    y:(addresses[6]+$c),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 12u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102b09u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b0a: /* move    y:(addresses[6]+$d),b0 */
    lows[1] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b0au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b0b: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b0bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b0c: /* move    a0,words[0] */
    words[0] = (lows[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b0cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b0d: /* do      #<$20,>$102b18 */
    loop_102b0d = 0x20u; if (loop_102b0d == 0u) goto L_102b18;
    MD_TRANSLATION_TRACE(0x102b0du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b0f: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x102b0fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b11: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b11u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b12: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b12u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b13: /* nop */

    MD_TRANSLATION_TRACE(0x102b13u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b14: /* nop */

    MD_TRANSLATION_TRACE(0x102b14u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b15: /* move    x:(addresses[2]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[2]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b15u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b17: /* move    words[0],x:(addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine, words[0]);
    MD_TRANSLATION_TRACE(0x102b17u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102b0d != 0u) goto L_102b0f;
L_102b18: /* move    b1,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x102b18u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b19: /* move    b0,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x102b19u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b1a: /* move    y:(addresses[6]+$7),words[1] */
    words[1] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b1au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b1b: /* do      #<$20,>$102b21 */
    loop_102b1b = 0x20u; if (loop_102b1b == 0u) goto L_102b21;
    MD_TRANSLATION_TRACE(0x102b1bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b1d: /* move    x:(addresses[3]),words[2] */
    words[2] = (md_read_x_raw(v, md_shared_scratch_x, addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b1du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b1e: /* mpy     words[1],words[2],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b1eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b1f: /* nop */

    MD_TRANSLATION_TRACE(0x102b1fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b20: /* move    a,x:(addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102b20u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102b1b != 0u) goto L_102b1d;
L_102b21: /* move    y:(addresses[6]+$1),words[0] */
    words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b21u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b22: /* move    y:(addresses[6]+$9),words[1] */
    words[1] = (u24(*(v->y + 9u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b22u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b23: /* do      #<$20,>$102b2f */
    loop_102b23 = 0x20u; if (loop_102b23 == 0u) goto L_102b2f;
    MD_TRANSLATION_TRACE(0x102b23u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b25: /* move    x:(addresses[1])+,a */
    accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102b25u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b26: /* move    x:(addresses[3]),b */
    accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, addresses[3], sine))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102b26u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b27: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b27u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b28: /* nop */

    MD_TRANSLATION_TRACE(0x102b28u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b29: /* move    a,words[2] */
    words[2] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b29u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b2a: /* mpy     words[1],words[2],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b2au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b2b: /* asr     #$3,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 3u; lows[0] = ((lows[2] >> 3u) | ((uint32_t)accum[2] << 21u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b2bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b2c: /* mpy     words[1],words[0],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b2cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b2d: /* move    a,x:(addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102b2du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b2e: /* move    b,words[1] */
    words[1] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b2eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102b23 != 0u) goto L_102b25;
L_102b2f: /* move    words[1],y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x102b2fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b30: /* move    #>$ffffff,addresses[11] */
    addresses[11] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b30u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b32: /* move    #>$ffffff,addresses[8] */
    addresses[8] = (0xffffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b32u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b34: /* move    y:(addresses[6]+$5),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 5u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102b34u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b35: /* move    y:(addresses[6]+$6),addresses[2] */
    addresses[2] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b35u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b37: /* move    #$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b37u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b38: /* move    #$3,addresses[12] */
    addresses[12] = (0x3u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b38u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b39: /* move    x:(addresses[2]+$140000),words[0] */
    words[0] = (md_read_source(v, addresses[2] + 1310720u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b39u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b3b: /* move    x:(addresses[2]+$140080),words[1] */
    words[1] = (md_read_source(v, addresses[2] + 1310848u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b3bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b3d: /* add     words[0],a            a1,b1 */
    parallel_words[0] = u24(accum[0]); add56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((parallel_words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x102b3du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b3e: /* move    #$1e,addresses[0] */
    addresses[0] = (0x1eu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b3eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b3f: /* add     words[1],b            a1,addresses[1] */
    parallel_words[0] = u24(accum[0]); add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); addresses[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b3fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b40: /* move    #$40,addresses[3] */
    addresses[3] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b40u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b41: /* move    b1,addresses[2] */
    addresses[2] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b41u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b42: /* nop */

    MD_TRANSLATION_TRACE(0x102b42u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b43: /* move    x:(addresses[1]+$140900),words[1] */
    words[1] = (md_read_source(v, addresses[1] + 1313024u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b43u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b45: /* move    x:(addresses[2]+$141900),words[3] */
    words[3] = (md_read_source(v, addresses[2] + 1317120u)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b45u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b47: /* move    #$0,addresses[1] */
    addresses[1] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b47u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b48: /* move    #$1,addresses[9] */
    addresses[9] = (0x1u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b48u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b49: /* mpy     words[3],words[1],a         words[3],words[0] */
    parallel_words[0] = words[3]; mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[0] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b49u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b4a: /* mpy     -words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[2] = (int32_t)(0u - (uint32_t)accum[2] - (uint32_t)(lows[2] != 0u)); lows[2] = (0u - lows[2]) & 0xffffffu; accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b4au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b4b: /* asl     a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = (int32_t)(((uint32_t)accum[2] << 1u) | (lows[2] >> 23u)); lows[0] = (lows[2] << 1u) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x102b4bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b4c: /* move    #$40,words[1] */
    words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b4cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b4d: /* move    a,words[0] */
    words[0] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b4du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b4e: /* move    b,y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102b4eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b4f: /* move    words[1],y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b4fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b50: /* move    #$80,words[1] */
    words[1] = (((0x80u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b50u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b51: /* move    words[1],y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b51u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b52: /* move    #$40,words[1] */
    words[1] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b52u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b53: /* move    words[1],y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b53u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b54: /* move    y:(addresses[4])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b54u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b55: /* move    y:(addresses[6]+$f),words[1] */
    words[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b55u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b56: /* move    words[1],x:(addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b56u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b57: /* move    y:(addresses[6]+$e),words[1] */
    words[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b57u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b58: /* move    words[1],x:(addresses[1])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b58u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b59: /* move    y:(addresses[6]+$11),words[1] */
    words[1] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b59u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b5a: /* move    words[1],x:(addresses[3])+ */
    md_write_x(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b5au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b5b: /* move    y:(addresses[6]+$10),words[1] */
    words[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b5bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b5c: /* move    words[1],x:(addresses[3]) */
    md_write_x(v, md_shared_scratch_x, addresses[3], sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b5cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b5d: /* move    y:(addresses[6]+$13),words[1] */
    words[1] = (u24(*(v->y + 19u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b5du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b5e: /* move    words[1],x:(addresses[0]) */
    md_write_x(v, md_shared_scratch_x, addresses[0], sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b5eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b5f: /* move    y:(addresses[6]+$12),words[1] */
    words[1] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b5fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b60: /* move    words[1],x:(addresses[0]+$1) */
    md_write_x(v, md_shared_scratch_x, addresses[0] + 1u, sine, words[1]);
    MD_TRANSLATION_TRACE(0x102b60u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b61: /* move    x:(addresses[1])+,words[1] */
    words[1] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b61u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b62: /* mpy     words[3],words[1],a         x:(addresses[1]),words[1] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, addresses[1], sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b62u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b63: /* mac     words[1],words[0],a         x:(addresses[0])+,words[1]      y:(addresses[4])+,words[2] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[0], addresses[8], 1, 0u), sine); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b63u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b64: /* do      #<$20,>$102b72 */
    loop_102b64 = 0x20u; if (loop_102b64 == 0u) goto L_102b72;
    MD_TRANSLATION_TRACE(0x102b64u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b66: /* mac     words[1],words[2],a         x:(addresses[0])+,words[1]      y:(addresses[4])+,words[2] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[0], addresses[8], 1, 0u), sine); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b66u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b67: /* mac     words[1],words[2],a         x:(addresses[0])-,words[1]      y:(addresses[4])+,words[2] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[0], addresses[8], -1, 0u), sine); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b67u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b68: /* mac     words[1],words[2],a         y:(addresses[3])-,words[2] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[3], addresses[11], -1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b68u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b69: /* asl     a               x:(addresses[3])+,words[1]      y:(addresses[4])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = (int32_t)(((uint32_t)accum[2] << 1u) | (lows[2] >> 23u)); lows[0] = (lows[2] << 1u) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[3] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b69u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b6a: /* mpy     words[3],words[1],b         x:(addresses[3])+,words[3] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b6au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b6b: /* mac     words[0],words[3],b         a,words[1]            y:(addresses[4])+,words[2] */
    parallel_words[0] = u24(lim24(accum[0])); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b6bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b6c: /* mac     words[1],words[2],b         x:(addresses[1])+,words[1]      y:(addresses[4])+,words[2] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b6cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b6d: /* mac     words[1],words[2],b         x:(addresses[1]),words[1]       y:(addresses[4])+,words[2] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, addresses[1], sine); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b6du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b6e: /* mac     words[1],words[2],b         a,x:(addresses[1])+       y:(addresses[4])+,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine, parallel_words[0]); words[3] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b6eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b6f: /* asl     b               x:(addresses[1])+,words[1] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine); accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 1u) | (lows[2] >> 23u)); lows[1] = (lows[2] << 1u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b6fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b70: /* mpy     words[3],words[1],a         x:(addresses[0])+,words[1]      a,words[2] */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[0], addresses[8], 1, 0u), sine); parallel_words[1] = u24(lim24(accum[0])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu; words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b70u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b71: /* mac     words[2],words[0],a         b,x:(addresses[3])        y:(addresses[4])+,words[2] */
    parallel_words[0] = u24(lim24(accum[1])); parallel_words[1] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[3], sine, parallel_words[0]); words[2] = (parallel_words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b71u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102b64 != 0u) goto L_102b66;
L_102b72: /* move    x:(addresses[1])+,words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b72u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b73: /* move    words[3],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102b73u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b74: /* move    x:(addresses[1])+,words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[1], addresses[9], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b74u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b75: /* move    words[3],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102b75u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b76: /* move    x:(addresses[3])-,words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], -1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b76u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b77: /* move    words[3],y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102b77u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b78: /* move    x:(addresses[3]),words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, addresses[3], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b78u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b79: /* move    words[3],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102b79u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b7a: /* move    x:(addresses[0]),words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, addresses[0], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b7au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b7b: /* move    words[3],y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102b7bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b7c: /* move    x:-(addresses[0]),words[3] */
    words[3] = (md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[0], addresses[8], -1, 1u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b7cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b7d: /* move    words[3],y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x102b7du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b7e: /* move    #>$40,addresses[3] */
    addresses[3] = (0x40u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x102b7eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b80: /* do      #<$10,>$102b86 */
    loop_102b80 = 0x10u; if (loop_102b80 == 0u) goto L_102b86;
    MD_TRANSLATION_TRACE(0x102b80u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b82: /* move    x:(addresses[3])+,a */
    accum[0] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x102b82u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b83: /* move    x:(addresses[3])+,b */
    accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, md_agu(&addresses[3], addresses[11], 1, 0u), sine))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x102b83u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b84: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x102b84u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_102b85: /* move    b,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x102b85u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_102b80 != 0u) goto L_102b82;
L_102b86: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t trxcl_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_201cb0: /* move.l a2, -(a7) */
    /* local register save */
C_201cb2: /* move.l d[2], -(a7) */
    /* local register save */
C_201cb4: /* movea.l $c(a7), a1 */
    /* resolved argument pointer */
C_201cb8: /* movea.l $10(a7), a2 */
    /* resolved argument pointer */
C_201cbc: /* tst.l (a1) */
    lhs = (uint32_t)*(y + (0u) / 4u); rhs = 0u;
C_201cbe: /* bne.b $201cc6 */
    if (lhs != rhs) goto C_201cc6;
C_201cc0: /* clr.l d[0] */
    d[0] = 0u;
C_201cc2: /* bra.w $201d7a */
    goto C_201d7a;
C_201cc6: /* move.w (a2), d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_201cc8: /* move.w d[1], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[1] & 0xffffu)) & 0xffffu);
C_201cca: /* mulu.w #$cccd, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xcccdu);
C_201cce: /* moveq #$12, d[2] */
    d[2] = 0x00000012u;
C_201cd0: /* lsr.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] >> ((d[2]) & 63u) : 0u);
C_201cd2: /* addi.l #$578, d[0] */
    lhs = d[0]; rhs = 0x578u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201cd8: /* move.l d[0], $c(a1) */
    *(y + (12u) / 4u) = (int32_t)(d[0]);
C_201cdc: /* move.w d[1], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[1] & 0xffffu)) & 0xffffu);
C_201cde: /* mulu.w #$8f9d, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x8f9du);
C_201ce2: /* clr.w d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((0u) & 0xffffu);
C_201ce4: /* swap d[0] */
    d[0] = (d[0] << 16) | (d[0] >> 16);
C_201ce6: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_201ce8: /* andi.l #$ffff, d[1] */
    lhs = d[1]; rhs = 0xffffu; d[1] = lhs & rhs;
C_201cee: /* lsr.l #$1, d[1] */
    d[1] = (((0x1u) & 63u) < 32u ? d[1] >> ((0x1u) & 63u) : 0u);
C_201cf0: /* add.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201cf2: /* andi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu; d[0] = lhs & rhs;
C_201cf8: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_201cfa: /* addi.l #$458, d[0] */
    lhs = d[0]; rhs = 0x458u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201d00: /* move.l d[0], $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_201d04: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_201d08: /* mulu.w #$28, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x28u);
C_201d0c: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_201d0e: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_201d10: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_201d12: /* movea.l d[0], a0 */
    a0 = d[0];
C_201d14: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_201d1a: /* move.l $320(a0), $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((800u) + a0)) / 4u)));
C_201d20: /* clr.l d[0] */
    d[0] = 0u;
C_201d22: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_201d26: /* addi.l #$1f4, d[0] */
    lhs = d[0]; rhs = 0x1f4u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201d2c: /* asr.l #$5, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x5u) & 63u) < 32u ? ((0x5u) & 63u) : 31u));
C_201d2e: /* move.l d[0], $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_201d32: /* clr.l d[0] */
    d[0] = 0u;
C_201d34: /* move.b $6(a2), d[0] */
    d[0] = (d[0] & 0xffffff00u) | ((((*(p + (6u) / 2u) >> (((6u) & 1u) ? 0u : 8u)) & 0xffu)) & 0xffu);
C_201d38: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201d3e: /* move.l d[0], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_201d42: /* clr.l d[1] */
    d[1] = 0u;
C_201d44: /* move.w $8(a2), d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_201d48: /* move.w d[1], d[0] */
    d[0] = (d[0] & 0xffff0000u) | (((d[1] & 0xffffu)) & 0xffffu);
C_201d4a: /* mulu.w #$cccd, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xcccdu);
C_201d4e: /* lsr.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] >> ((d[2]) & 63u) : 0u);
C_201d50: /* addi.l #$578, d[0] */
    lhs = d[0]; rhs = 0x578u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_201d56: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_201d5a: /* lsl.l #$8, d[1] */
    d[1] = (((0x8u) & 63u) < 32u ? d[1] << ((0x8u) & 63u) : 0u);
C_201d5c: /* addi.l #$100000, d[1] */
    lhs = d[1]; rhs = 0x100000u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[1] = lhs + rhs;
C_201d62: /* move.l d[1], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[1]);
C_201d66: /* clr.l d[0] */
    d[0] = 0u;
C_201d68: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_201d6c: /* move.b #$9, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x9u) & 0xffu);
C_201d70: /* lsl.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] << ((d[2]) & 63u) : 0u);
C_201d72: /* neg.l d[0] */
    d[0] = 0u - d[0];
C_201d74: /* move.l d[0], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_201d78: /* moveq #$9, d[0] */
    d[0] = 0x00000009u;
C_201d7a: /* move.l (a7)+, d[2] */
    /* local register restore */
C_201d7c: /* movea.l (a7)+, a2 */
    /* local register restore */
C_201d7e: /* rts  */
    return d[0];
}

void trxcl_update_coeffs(const uint32_t *p, TrxclVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = trxcl_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { trxcl_dsp_init(v, 0); }
}
void trxcl_voice_init(TrxclVoice *v) {
    uint32_t i;
    for (i = 0u; i < 20u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void trxcl_voice_trigger(TrxclVoice *v) {
    trxcl_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t trxcl_resample_run(TrxclVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxclVoice
#define MD_RENDER48 trxcl_render_block_48k_float
#define MD_RENDER44 trxcl_render_block_44k1
#define MD_RESAMPLE_RUN trxcl_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxclVoice
#define MD_SLOT_INIT trxcl_slot_init
#define MD_SLOT_PROCESS trxcl_slot_process
#define MD_VOICE_INIT trxcl_voice_init
#define MD_COEFFICIENTS trxcl_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER trxcl_update_coeffs
#define MD_VOICE_TRIGGER trxcl_voice_trigger
#define MD_RENDER44 trxcl_render_block_44k1
#define MD_RENDER48 trxcl_render_block_48k_float
#include "../engine_slot.inc"
#endif
