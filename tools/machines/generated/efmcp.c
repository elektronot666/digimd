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
    int32_t y[25];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmcpVoice;
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

static uint32_t md_read_x(EfmcpVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmcpVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmcpVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2073u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmcpVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2073u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmcpVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmcpVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void efmcp_dsp_init(EfmcpVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1030ec: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1030ecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030ed: /* nop */

    MD_TRANSLATION_TRACE(0x1030edu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030ee: /* move    a1,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1030eeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030ef: /* rts */
    return;
}

void efmcp_dsp_trigger(EfmcpVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_1030f0: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1030f0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f1: /* move    y:(addresses[6]+$7),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 7u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1030f1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f2: /* move    #>$7fffff,words[0] */
    words[0] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1030f2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f4: /* move    #>$50c335,words[1] */
    words[1] = (0x50c335u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1030f4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f6: /* move    y:(addresses[6]+$a),words[3] */
    words[3] = (u24(*(v->y + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1030f6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f7: /* move    a1,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1030f7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f8: /* move    a0,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1030f8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030f9: /* move    a1,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1030f9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030fa: /* move    a0,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1030fau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030fb: /* move    a0,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1030fbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030fc: /* move    a0,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1030fcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030fd: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1030fdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030fe: /* move    a0,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1030feu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1030ff: /* move    b1,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1030ffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103100: /* move    words[0],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x103100u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103101: /* move    words[1],y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103101u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103102: /* move    words[3],y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x103102u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103103: /* rts */
    return;
}

void efmcp_render_block_44k1(EfmcpVoice *v, const int32_t *sine) {
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
    uint32_t loop_103122 = 0;
    uint32_t loop_10315d = 0;
    uint32_t loop_103184 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_103104: /* move    y:(addresses[6]+$1),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 1u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103104u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103105: /* move    y:(addresses[6]+$6),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 6u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103105u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103106: /* move    a1,x:$1 */
    *(md_shared_scratch_x + 1u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103106u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103107: /* move    b1,x:$6 */
    *(md_shared_scratch_x + 6u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103107u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103108: /* move    y:(addresses[6]+$8),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 8u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103108u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103109: /* move    y:(addresses[6]+$3),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 3u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103109u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10310a: /* move    a1,x:$8 */
    *(md_shared_scratch_x + 8u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10310au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10310b: /* move    b1,x:$3 */
    *(md_shared_scratch_x + 3u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10310bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10310c: /* move    y:(addresses[6]+$4),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 4u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10310cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10310d: /* move    y:(addresses[6]+$5),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 5u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10310du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10310e: /* move    a1,x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10310eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10310f: /* move    b1,x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10310fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103110: /* move    y:(addresses[6]+$c),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 12u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103110u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103111: /* move    y:(addresses[6]+$d),a0 */
    lows[0] = (u24(*(v->y + 13u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103111u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103112: /* move    y:(addresses[6]+$e),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 14u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103112u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103113: /* move    y:(addresses[6]+$f),b0 */
    lows[1] = (u24(*(v->y + 15u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103113u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103114: /* move    a,l:<$c */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[12] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[12] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103114u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103115: /* move    b,l:<$e */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[14] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[14] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103115u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103116: /* move    y:(addresses[6]+$10),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 16u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103116u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103117: /* move    y:(addresses[6]+$11),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 17u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103117u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103118: /* move    a1,x:$10 */
    *(md_shared_scratch_x + 16u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103118u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103119: /* move    b1,x:$11 */
    *(md_shared_scratch_x + 17u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103119u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10311a: /* move    y:(addresses[6]+$13),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 19u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10311au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10311b: /* move    y:(addresses[6]+$14),a0 */
    lows[0] = (u24(*(v->y + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10311bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10311c: /* move    y:(addresses[6]+$15),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10311cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10311d: /* move    y:(addresses[6]+$17),words[1] */
    words[1] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10311du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10311e: /* move    a,l:<$13 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[19] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[19] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x10311eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10311f: /* move    b1,x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10311fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103120: /* move    words[1],x:$17 */
    *(md_shared_scratch_x + 23u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103120u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103121: /* move    addresses[7],addresses[1] */
    addresses[1] = (addresses[7]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103121u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103122: /* do      #<$20,>$103152 */
    loop_103122 = 0x20u; if (loop_103122 == 0u) goto L_103152;
    MD_TRANSLATION_TRACE(0x103122u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103124: /* move    x:$10,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103124u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103125: /* move    x:$8,words[0] */
    words[0] = (u24(*(md_shared_scratch_x + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103125u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103126: /* mpy     words[0],words[3],b         x:$1,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 1u)); mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103126u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103127: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103127u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103128: /* move    b1,x:$10 */
    *(md_shared_scratch_x + 16u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103128u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103129: /* asl     #$3,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103129u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10312a: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10312au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10312b: /* move    x:$3,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10312bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10312c: /* move    b1,words[3] */
    words[3] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10312cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10312d: /* mpy     words[3],words[2],a         l:<$13,x */
    parallel_words[0] = u24(md_shared_scratch_x[19]); parallel_words[3] = u24(md_shared_scratch_y[19]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10312du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10312e: /* asr     #$5,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 5u; lows[0] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10312eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10312f: /* move    l:<$c,b */
    parallel_words[0] = u24(md_shared_scratch_x[12]); parallel_words[3] = u24(md_shared_scratch_y[12]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10312fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103130: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103130u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103131: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x103131u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103133: /* move    b1,addresses[0] */
    addresses[0] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103133u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103134: /* sub     x,b */
    sub56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103134u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103135: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103135u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103136: /* move    x:$6,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103136u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103137: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103137u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103139: /* move    b10,l:<$c */
    parallel_words[0] = (uint32_t)(accum[1]); parallel_words[3] = lows[1]; md_shared_scratch_x[12] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[12] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103139u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10313a: /* mpy     words[2],words[0],a         x:$11,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 17u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10313au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10313b: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10313bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10313c: /* move    x:$5,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10313cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10313d: /* move    a,l:<$13 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[19] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[19] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x10313du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10313e: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10313eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10313f: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10313fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103140: /* move    b1,x:$11 */
    *(md_shared_scratch_x + 17u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103140u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103141: /* move    a1,words[0] */
    words[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103141u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103142: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103142u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103143: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103143u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103144: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103144u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103145: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103145u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103146: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103146u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103147: /* add     words[3],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103147u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103148: /* move    l:<$e,a */
    parallel_words[0] = u24(md_shared_scratch_x[14]); parallel_words[3] = u24(md_shared_scratch_y[14]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103148u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103149: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103149u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10314a: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10314au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10314b: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10314bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10314c: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x10314cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10314e: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10314eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103150: /* move    a10,l:<$e */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[14] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[14] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103150u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103151: /* move    words[0],y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine, words[0]);
    MD_TRANSLATION_TRACE(0x103151u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_103122 != 0u) goto L_103124;
L_103152: /* move    addresses[1],addresses[7] */
    addresses[7] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103152u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103153: /* move    #$15,addresses[2] */
    addresses[2] = (0x15u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103153u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103154: /* move    y:(addresses[6]+$b),words[0] */
    words[0] = (u24(*(v->y + 11u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103154u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103155: /* move    x:$15,b */
    accum[1] = sx24((int32_t)(u24(*(md_shared_scratch_x + 21u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103155u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103156: /* cmp     #<$0,b */
    accum[2] = sx24((int32_t)(0x0u)); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1];
    MD_TRANSLATION_TRACE(0x103156u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103157: /* bne     func_10315a */
    if (!flags[0]) goto L_10315a;
    MD_TRANSLATION_TRACE(0x103157u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103159: /* move    y:(addresses[6]+$2),words[0] */
    words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103159u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10315a: /* move    #>$66666,words[1] */
    words[1] = (0x66666u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10315au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10315c: /* move    y:(addresses[6]+$12),words[2] */
    words[2] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10315cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10315d: /* do      #<$20,>$103170 */
    loop_10315d = 0x20u; if (loop_10315d == 0u) goto L_103170;
    MD_TRANSLATION_TRACE(0x10315du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10315f: /* mpy     words[2],words[0],b         y:(addresses[7]),words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, addresses[7], sine); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10315fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103160: /* mpyr    words[3],words[2],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103160u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103161: /* move    x:(addresses[2]),b        b,words[2] */
    parallel_words[0] = u24(lim24(accum[1])); accum[1] = sx24((int32_t)(md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine))); lows[1] = 0u; words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103161u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103162: /* tst     b               words[2],b */
    parallel_words[0] = words[2]; flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); accum[1] = sx24((int32_t)(parallel_words[0])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103162u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103163: /* beq     func_10316f */
    if (flags[0]) goto L_10316f;
    MD_TRANSLATION_TRACE(0x103163u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103165: /* cmp     words[1],b            x:(addresses[2]),b */
    parallel_words[0] = md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine); accum[2] = sx24((int32_t)(words[1])); lows[2] = 0u; flags[0] = (accum[1] == accum[2] && lows[1] == lows[2]); flags[1] = (accum[1] < accum[2] || (accum[1] == accum[2] && lows[1] < lows[2])); flags[2] = (!flags[0] && !flags[1]); flags[3] = flags[1]; accum[1] = sx24((int32_t)(parallel_words[0])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103165u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103166: /* bge     func_10316f */
    if (!flags[1]) goto L_10316f;
    MD_TRANSLATION_TRACE(0x103166u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103168: /* sub     #<$1,b */
    sub56(accum[1], lows[1], sx24((int32_t)(0x1u)), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103168u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103169: /* move    #>$50c335,words[2] */
    words[2] = (0x50c335u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103169u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10316b: /* move    b1,x:(addresses[2]) */
    md_write_x(v, md_shared_scratch_x, addresses[2], sine, u24(accum[1]));
    MD_TRANSLATION_TRACE(0x10316bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10316c: /* bne     func_10316f */
    if (!flags[0]) goto L_10316f;
    MD_TRANSLATION_TRACE(0x10316cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10316e: /* move    y:(addresses[6]+$2),words[0] */
    words[0] = (u24(*(v->y + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10316eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10316f: /* move    a1,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine, u24(accum[0]));
    MD_TRANSLATION_TRACE(0x10316fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_10315d != 0u) goto L_10315f;
L_103170: /* move    words[2],y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(words[2]));
    MD_TRANSLATION_TRACE(0x103170u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103171: /* move    addresses[1],addresses[7] */
    addresses[7] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103171u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103172: /* move    addresses[1],addresses[4] */
    addresses[4] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103172u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103173: /* move    #$17,addresses[2] */
    addresses[2] = (0x17u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103173u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103174: /* move    y:(addresses[6]+$9),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 9u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103174u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103175: /* asr     #$1,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = accum[2] >> 1u; lows[1] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103175u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103176: /* move    #$40,words[0] */
    words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103176u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103177: /* add     words[0],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103177u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103178: /* move    a1,words[2] */
    words[2] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103178u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103179: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103179u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10317a: /* move    y:(addresses[7])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10317au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10317b: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10317bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10317c: /* move    x:(addresses[2]),words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10317cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10317d: /* move    y:(addresses[6]+$18),words[3] */
    words[3] = (u24(*(v->y + 24u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10317du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10317e: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10317eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10317f: /* mac     words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10317fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103180: /* mpy     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103180u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103181: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103181u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103182: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103182u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103183: /* mac     words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103183u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103184: /* do      #<$f,>$10318e */
    loop_103184 = 0xfu; if (loop_103184 == 0u) goto L_10318e;
    MD_TRANSLATION_TRACE(0x103184u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103186: /* mpy     words[3],words[1],a         a,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[0])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103186u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103187: /* move    x:(addresses[2]),words[0]       b,words[3] */
    parallel_words[0] = u24(lim24(accum[1])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103187u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103188: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x103188u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103189: /* mac     words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103189u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10318a: /* mpy     words[3],words[1],b         b,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[1])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10318au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10318b: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10318bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10318c: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x10318cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10318d: /* mac     words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10318du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_103184 != 0u) goto L_103186;
L_10318e: /* move    a,y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x10318eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10318f: /* move    b,y:(addresses[4]) */
    md_write_y(v, md_shared_scratch_y, addresses[4], sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x10318fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103190: /* move    b,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x103190u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103191: /* move    l:<$c,a */
    parallel_words[0] = u24(md_shared_scratch_x[12]); parallel_words[3] = u24(md_shared_scratch_y[12]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103191u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103192: /* move    l:<$e,b */
    parallel_words[0] = u24(md_shared_scratch_x[14]); parallel_words[3] = u24(md_shared_scratch_y[14]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103192u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103193: /* move    a1,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103193u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103194: /* move    a0,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103194u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103195: /* move    b1,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103195u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103196: /* move    b0,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x103196u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103197: /* move    x:$10,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 16u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103197u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103198: /* move    x:$11,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 17u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103198u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103199: /* move    a1,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103199u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10319a: /* move    b1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10319au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10319b: /* move    l:<$13,a */
    parallel_words[0] = u24(md_shared_scratch_x[19]); parallel_words[3] = u24(md_shared_scratch_y[19]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10319bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10319c: /* move    x:$15,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10319cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10319d: /* move    x:$17,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10319du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10319e: /* move    a1,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10319eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10319f: /* move    a0,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10319fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a0: /* move    b1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1031a0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a1: /* move    words[1],y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1031a1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1031a2: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t efmcp_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2023de: /* move.l a2, -(a7) */
    /* local register save */
C_2023e0: /* move.l d[2], -(a7) */
    /* local register save */
C_2023e2: /* movea.l $c(a7), a1 */
    /* resolved argument pointer */
C_2023e6: /* movea.l $10(a7), a2 */
    /* resolved argument pointer */
C_2023ea: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_2023ec: /* mulu.w #$b, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xbu);
C_2023f0: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2023f2: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2023f8: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2023fa: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_2023fc: /* addq.l #$3, d[1] */
    d[1] = d[1] + 0x3u;
C_2023fe: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_202400: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202402: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_202408: /* move.l (a0, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_20240c: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20240e: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_202412: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202416: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_20241a: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_20241c: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_20241e: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_202420: /* movea.l d[0], a0 */
    a0 = d[0];
C_202422: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202428: /* move.l $24c(a0), $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20242e: /* lea.l $c(a1), a0 */
    a0 = 12u;
C_202432: /* clr.l d[0] */
    d[0] = 0u;
C_202434: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_202438: /* lsl.l #$7, d[0] */
    d[0] = (((0x7u) & 63u) < 32u ? d[0] << ((0x7u) & 63u) : 0u);
C_20243a: /* move.l d[0], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[0]);
C_20243c: /* cmpi.l #$3fff, d[0] */
    lhs = d[0]; rhs = 0x3fffu;
C_202442: /* bgt.b $20244a */
    if ((int32_t)lhs > (int32_t)rhs) goto C_20244a;
C_202444: /* move.l #$4000, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x4000u);
C_20244a: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_20244e: /* moveq #$9, d[2] */
    d[2] = 0x00000009u;
C_202450: /* lsl.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] << ((d[2]) & 63u) : 0u);
C_202452: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_202458: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20245c: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_202460: /* mulu.w #$52, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x52u);
C_202464: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_202466: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_202468: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20246a: /* movea.l d[0], a0 */
    a0 = d[0];
C_20246c: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202472: /* move.l $24c(a0), $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_202478: /* move.l #$7a1200, d[2] */
    d[2] = 0x7a1200u;
C_20247e: /* move.l d[2], $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(d[2]);
C_202482: /* clr.l $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(0u);
C_202486: /* clr.l $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(0u);
C_20248a: /* clr.l d[0] */
    d[0] = 0u;
C_20248c: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_202490: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_202492: /* move.l #$7fffff, d[1] */
    d[1] = 0x7fffffu;
C_202498: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_20249a: /* move.l d[1], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[1]);
C_20249e: /* move.b $4(a2), d[0] */
    d[0] = (d[0] & 0xffffff00u) | ((((*(p + (4u) / 2u) >> (((4u) & 1u) ? 0u : 8u)) & 0xffu)) & 0xffu);
C_2024a2: /* lsr.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] >> ((0x2u) & 63u) : 0u);
C_2024a4: /* moveq #$3f, d[2] */
    d[2] = 0x0000003fu;
C_2024a6: /* and.l d[0], d[2] */
    lhs = d[2]; rhs = d[0]; d[2] = lhs & rhs;
C_2024a8: /* move.l d[2], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[2]);
C_2024ac: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_2024b0: /* mulu.w #$28, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x28u);
C_2024b4: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_2024b6: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2024b8: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2024ba: /* movea.l d[0], a0 */
    a0 = d[0];
C_2024bc: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2024c2: /* move.l $118(a0), $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((280u) + a0)) / 4u)));
C_2024c8: /* moveq #$c, d[0] */
    d[0] = 0x0000000cu;
C_2024ca: /* move.l (a7)+, d[2] */
    /* local register restore */
C_2024cc: /* movea.l (a7)+, a2 */
    /* local register restore */
C_2024ce: /* rts  */
    return d[0];
}

void efmcp_update_coeffs(const uint32_t *p, EfmcpVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmcp_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmcp_dsp_init(v, 0); }
}
void efmcp_voice_init(EfmcpVoice *v) {
    uint32_t i;
    for (i = 0u; i < 25u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmcp_voice_trigger(EfmcpVoice *v) {
    efmcp_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmcp_resample_run(EfmcpVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmcpVoice
#define MD_RENDER48 efmcp_render_block_48k_float
#define MD_RENDER44 efmcp_render_block_44k1
#define MD_RESAMPLE_RUN efmcp_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmcpVoice
#define MD_SLOT_INIT efmcp_slot_init
#define MD_SLOT_PROCESS efmcp_slot_process
#define MD_VOICE_INIT efmcp_voice_init
#define MD_COEFFICIENTS efmcp_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmcp_update_coeffs
#define MD_VOICE_TRIGGER efmcp_voice_trigger
#define MD_RENDER44 efmcp_render_block_44k1
#define MD_RENDER48 efmcp_render_block_48k_float
#include "../engine_slot.inc"
#endif
