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
    int32_t y[16];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} PimaVoice;
extern uint32_t md_control_tempo_denominator;
extern uint32_t md_divs32(uint32_t, uint32_t);
extern uint32_t md_divu32(uint32_t, uint32_t);
extern const uint32_t control_2462e8[512];
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

static uint32_t md_read_x(PimaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(PimaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(PimaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 256u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2064u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(PimaVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 256u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2064u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(PimaVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(PimaVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void pima_dsp_init(PimaVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_145aa7: /* move    #>$2228,words[0] */
    words[0] = (0x2228u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145aa7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aa9: /* move    words[0],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145aa9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aaa: /* move    #>$442,words[0] */
    words[0] = (0x442u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145aaau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aac: /* move    words[0],y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145aacu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aad: /* move    #>$0,words[0] */
    words[0] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145aadu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aaf: /* move    words[0],y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145aafu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab0: /* move    words[0],y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab1: /* move    words[0],y:(addresses[6]+$9) */
    *(v->y + 9u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab2: /* move    words[0],y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab3: /* move    words[0],y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab4: /* move    words[0],y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab5: /* move    words[0],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab6: /* rts */
    return;
}

void pima_dsp_trigger(PimaVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_145ab7: /* move    #>$66666,words[0] */
    words[0] = (0x66666u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ab7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ab9: /* move    words[0],y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ab9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aba: /* rts */
    return;
}

void pima_render_block_44k1(PimaVoice *v, const int32_t *sine) {
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
    uint32_t loop_145ac0 = 0;
    uint32_t loop_145acd = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_145abb: /* move    #>$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145abbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145abd: /* move    #$1f,addresses[12] */
    addresses[12] = (0x1fu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145abdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145abe: /* move    y:(addresses[6]+$d),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 13u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x145abeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145abf: /* move    y:(addresses[6]+$e),words[1] */
    words[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145abfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac0: /* do      #<$1f,>$145ac7 */
    loop_145ac0 = 0x1fu; if (loop_145ac0 == 0u) goto L_145ac7;
    MD_TRANSLATION_TRACE(0x145ac0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac2: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ac2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac3: /* move    words[1],words[0] */
    words[0] = (words[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ac3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac4: /* move    b1,x:(addresses[4])+ */
    *(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x145ac4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac5: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ac5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac6: /* move    words[0],b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((words[0]) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x145ac6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_145ac0 != 0u) goto L_145ac2;
L_145ac7: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ac7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac8: /* move    words[1],y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x145ac8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ac9: /* move    b1,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x145ac9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aca: /* move    b1,x:(addresses[4])+ */
    *(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x145acau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145acb: /* move    #>$0,addresses[4] */
    addresses[4] = (0x0u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145acbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145acd: /* do      #<$20,>$145af4 */
    loop_145acd = 0x20u; if (loop_145acd == 0u) goto L_145af4;
    MD_TRANSLATION_TRACE(0x145acdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145acf: /* move    y:(addresses[6]+$4),words[0] */
    words[0] = (u24(*(v->y + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145acfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad0: /* move    y:(addresses[6]+$8),words[2] */
    words[2] = (u24(*(v->y + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ad0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad1: /* mpy     words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ad1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad2: /* move    x:(addresses[4]),b */
    accum[1] = sx24((int32_t)(u24(*(md_shared_scratch_x + (0u + ((addresses[4]) & 31u)))))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x145ad2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad3: /* abs     b */
    if (accum[1] < 0) { accum[1] = (int32_t)(0u - (uint32_t)accum[1] - (uint32_t)(lows[1] != 0u)); lows[1] = (0u - lows[1]) & 0xffffffu; } flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ad3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad4: /* move    y:(addresses[6]+$1),words[0] */
    words[0] = (u24(*(v->y + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ad4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad5: /* move    y:(addresses[6]+$7),words[1] */
    words[1] = (u24(*(v->y + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ad5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad6: /* cmp     words[0],b */
    accum[2] = sx24((int32_t)(words[0])); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x145ad6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad7: /* add     words[1],a            ifle */
    if (!flags[2]) { add56(accum[0], lows[0], sx24((int32_t)(words[1])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); }
    MD_TRANSLATION_TRACE(0x145ad7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad8: /* move    x:(addresses[4])+,words[0] */
    words[0] = (u24(*(md_shared_scratch_x + (0u + ((addresses[4]++) & 31u))))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ad8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ad9: /* move    a,y:(addresses[6]+$8) */
    *(v->y + 8u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x145ad9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ada: /* move    a,words[2] */
    words[2] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145adau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145adb: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145adbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145adc: /* move    y:(addresses[6]+$2),words[2] */
    words[2] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145adcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145add: /* move    a,words[0] */
    words[0] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145addu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ade: /* mpyr    words[2],words[0],b */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145adeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145adf: /* asl     #$3,b,a */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[0] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[0] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145adfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae0: /* asr     a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 1u; lows[0] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ae0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae1: /* move    y:(addresses[6]+$5),words[0] */
    words[0] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ae1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae2: /* move    y:(addresses[6]+$a),words[2] */
    words[2] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ae2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae3: /* macr    -words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[2] = (int32_t)(0u - (uint32_t)accum[2] - (uint32_t)(lows[2] != 0u)); lows[2] = (0u - lows[2]) & 0xffffffu; add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ae3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae4: /* asl     a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = (int32_t)(((uint32_t)accum[2] << 1u) | (lows[2] >> 23u)); lows[0] = (lows[2] << 1u) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ae4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae5: /* move    y:(addresses[6]+$6),words[0] */
    words[0] = (u24(*(v->y + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ae5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae6: /* move    y:(addresses[6]+$b),words[2] */
    words[2] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ae6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae7: /* macr    -words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[2] = (int32_t)(0u - (uint32_t)accum[2] - (uint32_t)(lows[2] != 0u)); lows[2] = (0u - lows[2]) & 0xffffffu; add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145ae7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae8: /* move    y:(addresses[6]+$a),words[0] */
    words[0] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145ae8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145ae9: /* move    words[0],y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x145ae9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aea: /* move    a,y:(addresses[6]+$a) */
    *(v->y + 10u) = sx24((int32_t)(u24(lim24(accum[0]))));
    MD_TRANSLATION_TRACE(0x145aeau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aeb: /* move    y:(addresses[6]+$b),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 11u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x145aebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aec: /* move    y:(addresses[6]+$f),words[0] */
    words[0] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145aecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aed: /* move    a,words[2] */
    words[2] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145aedu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aee: /* sub     words[0],a */
    sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145aeeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145aef: /* move    words[2],y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x145aefu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145af0: /* move    y:(addresses[6]+$3),words[0] */
    words[0] = (u24(*(v->y + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x145af0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145af1: /* mpy     words[1],words[0],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x145af1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145af2: /* move    a,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x145af2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_145af3: /* move    b,y:(addresses[6]+$7) */
    *(v->y + 7u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x145af3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_145acd != 0u) goto L_145acf;
L_145af4: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t pima_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_203e88: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_203e8c: /* movem.l d[2]/a2-a3, (a7) */
    /* native locals replace original callee saves */
C_203e90: /* movea.l $10(a7), a3 */
    /* resolved argument pointer */
C_203e94: /* movea.l $14(a7), a2 */
    /* resolved argument pointer */
C_203e98: /* clr.l d[0] */
    d[0] = 0u;
C_203e9a: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_203e9c: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_203e9e: /* lea.l $2462e8.l, a0 */
    /* resolved table pointer */
C_203ea4: /* movea.l (a0, d[0].l * 4), a0 */
    a0 = *(control_2462e8 + ((0u + (d[0] * 4u + (0))) / 4u));
C_203ea8: /* move.l a0, $4(a3) */
    *(y + (4u) / 4u) = (int32_t)(a0);
C_203eac: /* clr.l d[1] */
    d[1] = 0u;
C_203eae: /* move.w $4(a2), d[1] */
    d[1] = (d[1] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_203eb2: /* lsr.l #$7, d[1] */
    d[1] = (((0x7u) & 63u) < 32u ? d[1] >> ((0x7u) & 63u) : 0u);
C_203eb4: /* lea.l $2464e8.l, a1 */
    /* resolved table pointer */
C_203eba: /* lea.l $2466e8.l, a0 */
    /* resolved table pointer */
C_203ec0: /* move.l (a1, d[0].l * 4), d[0] */
    d[0] = *(control_2462e8 + ((512u + (d[0] * 4u + (0))) / 4u));
C_203ec4: /* move.l (a0, d[1].l * 4), d[2] */
    d[2] = *(control_2462e8 + ((1024u + (d[1] * 4u + (0))) / 4u));
C_203ec8: /* muls.l d[2], d[0] */
    d[0] = d[0] * (d[2]);
C_203ecc: /* move.l d[0], $8(a3) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_203ed0: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_203ed4: /* lsr.l #$5, d[0] */
    d[0] = (((0x5u) & 63u) < 32u ? d[0] >> ((0x5u) & 63u) : 0u);
C_203ed6: /* lea.l $24bf94.l, a1 */
    /* resolved table pointer */
C_203edc: /* andi.l #$7fc, d[0] */
    lhs = d[0]; rhs = 0x7fcu; d[0] = lhs & rhs;
C_203ee2: /* lea.l (a1, d[0].l), a0 */
    a0 = d[0] * 1u + (0);
C_203ee6: /* move.l $400(a0), $c(a3) */
    *(y + (12u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((1024u) + a0)) / 4u)));
C_203eec: /* lea.l (a1, d[1].l * 4), a0 */
    a0 = d[1] * 4u + (0);
C_203ef0: /* move.l $200(a0), $10(a3) */
    *(y + (16u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((512u) + a0)) / 4u)));
C_203ef6: /* clr.l d[0] */
    d[0] = 0u;
C_203ef8: /* move.b $a(a2), d[0] */
    d[0] = (d[0] & 0xffffff00u) | ((((*(p + (10u) / 2u) >> (((10u) & 1u) ? 0u : 8u)) & 0xffu)) & 0xffu);
C_203efc: /* move.l (a1, d[0].l * 4), d[1] */
    d[1] = *(control_24bb90 + ((1028u + (d[0] * 4u + (0))) / 4u));
C_203f00: /* asr.l #$8, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_203f02: /* clr.l d[0] */
    d[0] = 0u;
C_203f04: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_203f08: /* lsr.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] >> ((0x7u) & 63u) : 0u);
C_203f0a: /* lea.l $2468e8.l, a0 */
    /* resolved table pointer */
C_203f10: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_2462e8 + ((1536u + (d[0] * 4u + (0))) / 4u));
C_203f14: /* muls.l d[1], d[0] */
    d[0] = d[0] * (d[1]);
C_203f18: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_203f1a: /* move.l d[0], $14(a3) */
    *(y + (20u) / 4u) = (int32_t)(d[0]);
C_203f1e: /* muls.l d[1], d[1] */
    d[1] = d[1] * (d[1]);
C_203f22: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_203f24: /* move.l d[1], $18(a3) */
    *(y + (24u) / 4u) = (int32_t)(d[1]);
C_203f28: /* moveq #$7, d[0] */
    d[0] = 0x00000007u;
C_203f2a: /* movem.l (a7), d[2]/a2-a3 */
    /* native locals replace original callee saves */
C_203f2e: /* lea.l $c(a7), a7 */
    /* local register save area */
C_203f32: /* rts  */
    return d[0];
}

void pima_update_coeffs(const uint32_t *p, PimaVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = pima_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { pima_dsp_init(v, 0); }
}
void pima_voice_init(PimaVoice *v) {
    uint32_t i;
    for (i = 0u; i < 16u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void pima_voice_trigger(PimaVoice *v) {
    pima_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t pima_resample_run(PimaVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE PimaVoice
#define MD_RENDER48 pima_render_block_48k_float
#define MD_RENDER44 pima_render_block_44k1
#define MD_RESAMPLE_RUN pima_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE PimaVoice
#define MD_SLOT_INIT pima_slot_init
#define MD_SLOT_PROCESS pima_slot_process
#define MD_VOICE_INIT pima_voice_init
#define MD_COEFFICIENTS pima_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER pima_update_coeffs
#define MD_VOICE_TRIGGER pima_voice_trigger
#define MD_RENDER44 pima_render_block_44k1
#define MD_RENDER48 pima_render_block_48k_float
#include "../engine_slot.inc"
#endif
