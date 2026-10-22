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
    int32_t y[28];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmhhVoice;
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

static uint32_t md_read_x(EfmhhVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmhhVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmhhVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2076u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmhhVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2076u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmhhVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmhhVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void efmhh_dsp_init(EfmhhVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_10332e: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10332eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10332f: /* nop */

    MD_TRANSLATION_TRACE(0x10332fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103330: /* move    a1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103330u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103331: /* rts */
    return;
}

void efmhh_dsp_trigger(EfmhhVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_103332: /* move    #>$50c335,b */
    accum[1] = sx24((int32_t)(0x50c335u)); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103332u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103334: /* move    y:(addresses[6]+$15),words[1] */
    words[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103334u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103335: /* add     words[1],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103335u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103336: /* move    #>$2000,a */
    accum[0] = sx24((int32_t)(0x2000u)); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x103336u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103338: /* move    #>$7fffff,words[1] */
    words[1] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103338u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10333a: /* move    a0,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10333au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10333b: /* move    a0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10333bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10333c: /* move    a0,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10333cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10333d: /* move    a0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10333du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10333e: /* move    a1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10333eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10333f: /* move    a0,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10333fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103340: /* move    a0,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103340u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103341: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103341u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103342: /* move    a0,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103342u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103343: /* move    a0,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103343u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103344: /* move    a0,y:(addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103344u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103345: /* move    a0,y:(addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103345u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103346: /* move    words[1],y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103346u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103347: /* move    b,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x103347u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103348: /* rts */
    return;
}

void efmhh_render_block_44k1(EfmhhVoice *v, const int32_t *sine) {
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
    uint32_t loop_103375 = 0;
    uint32_t loop_1033ce = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_103349: /* move    y:(addresses[6]+$1),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 1u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103349u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10334a: /* move    y:(addresses[6]+$2),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 2u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10334au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10334b: /* move    a1,x:$1 */
    *(md_shared_scratch_x + 1u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10334bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10334c: /* move    b1,x:$2 */
    *(md_shared_scratch_x + 2u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10334cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10334d: /* move    y:(addresses[6]+$3),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 3u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10334du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10334e: /* move    y:(addresses[6]+$4),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 4u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10334eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10334f: /* move    y:(addresses[6]+$5),words[1] */
    words[1] = (u24(*(v->y + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10334fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103350: /* move    a1,x:$3 */
    *(md_shared_scratch_x + 3u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103350u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103351: /* move    b1,x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103351u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103352: /* move    words[1],x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103352u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103353: /* move    y:(addresses[6]+$6),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 6u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103353u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103354: /* move    y:(addresses[6]+$7),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 7u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103354u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103355: /* move    a1,x:$6 */
    *(md_shared_scratch_x + 6u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103355u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103356: /* move    b1,x:$7 */
    *(md_shared_scratch_x + 7u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103356u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103357: /* move    y:(addresses[6]+$9),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 9u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103357u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103358: /* move    y:(addresses[6]+$a),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 10u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103358u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103359: /* move    a1,x:$9 */
    *(md_shared_scratch_x + 9u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103359u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10335a: /* move    b1,x:$a */
    *(md_shared_scratch_x + 10u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10335au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10335b: /* move    y:(addresses[6]+$b),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 11u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10335bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10335c: /* move    y:(addresses[6]+$c),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 12u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10335cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10335d: /* move    a1,x:$b */
    *(md_shared_scratch_x + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10335du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10335e: /* move    b1,x:$c */
    *(md_shared_scratch_x + 12u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10335eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10335f: /* move    y:(addresses[6]+$d),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 13u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10335fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103360: /* move    y:(addresses[6]+$e),a0 */
    lows[0] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103360u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103361: /* move    y:(addresses[6]+$f),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 15u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103361u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103362: /* move    y:(addresses[6]+$10),b0 */
    lows[1] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103362u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103363: /* move    y:(addresses[6]+$11),words[1] */
    words[1] = (u24(*(v->y + 17u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103363u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103364: /* move    y:(addresses[6]+$12),words[0] */
    words[0] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103364u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103365: /* move    a,l:<$d */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103365u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103366: /* move    b,l:<$f */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[15] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[15] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103366u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103367: /* move    x,l:<$11 */
    parallel_words[0] = (uint32_t)(sx24((int32_t)words[1])); parallel_words[3] = words[0]; md_shared_scratch_x[17] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[17] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103367u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103368: /* move    y:(addresses[6]+$14),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 20u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103368u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103369: /* move    y:(addresses[6]+$15),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103369u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10336a: /* move    a1,x:$14 */
    *(md_shared_scratch_x + 20u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10336au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10336b: /* move    b1,x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10336bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10336c: /* move    y:(addresses[6]+$16),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 22u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x10336cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10336d: /* move    y:(addresses[6]+$17),a0 */
    lows[0] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10336du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10336e: /* move    y:(addresses[6]+$18),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 24u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x10336eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10336f: /* move    y:(addresses[6]+$19),b0 */
    lows[1] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10336fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103370: /* move    y:(addresses[6]+$1a),words[1] */
    words[1] = (u24(*(v->y + 26u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103370u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103371: /* move    a,l:<$16 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[22] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[22] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103371u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103372: /* move    b,l:<$18 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[24] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[24] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103372u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103373: /* move    words[1],x:$1a */
    *(md_shared_scratch_x + 26u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103373u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103374: /* move    addresses[7],addresses[1] */
    addresses[1] = (addresses[7]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103374u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103375: /* do      #<$20,>$1033bb */
    loop_103375 = 0x20u; if (loop_103375 == 0u) goto L_1033bb;
    MD_TRANSLATION_TRACE(0x103375u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103377: /* move    x:$3,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103377u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103378: /* move    x:$1,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103378u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103379: /* mpy     words[3],words[2],a         l:<$16,x */
    parallel_words[0] = u24(md_shared_scratch_x[22]); parallel_words[3] = u24(md_shared_scratch_y[22]); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103379u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10337a: /* asr     #$5,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 5u; lows[0] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10337au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10337b: /* move    l:<$d,b */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10337bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10337c: /* add     x,b */
    add56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10337cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10337d: /* and     #>$7fff,b */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | (u24(accum[1]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x10337du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10337f: /* move    b1,addresses[0] */
    addresses[0] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10337fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103380: /* sub     x,b */
    sub56(accum[1], lows[1], sx24((int32_t)words[1]), words[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103380u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103381: /* add     a,b */
    add56(accum[1], lows[1], accum[0], lows[0], &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103381u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103382: /* move    x:$6,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103382u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103383: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103383u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103385: /* move    b10,l:<$d */
    parallel_words[0] = (uint32_t)(accum[1]); parallel_words[3] = lows[1]; md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103385u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103386: /* mpy     words[2],words[0],a         x:$14,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 20u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103386u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103387: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103387u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103388: /* move    x:$5,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 5u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103388u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103389: /* move    a,l:<$16 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[22] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[22] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103389u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10338a: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10338au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10338b: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10338bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10338c: /* move    b1,x:$14 */
    *(md_shared_scratch_x + 20u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10338cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10338d: /* move    a1,words[0] */
    words[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10338du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10338e: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10338eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10338f: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10338fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103390: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103390u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103391: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103391u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103392: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103392u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103393: /* add     words[3],b            l:<$18,x */
    parallel_words[0] = u24(md_shared_scratch_x[24]); parallel_words[3] = u24(md_shared_scratch_y[24]); add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103393u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103394: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103394u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103395: /* move    l:<$f,a */
    parallel_words[0] = u24(md_shared_scratch_x[15]); parallel_words[3] = u24(md_shared_scratch_y[15]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103395u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103396: /* add     x,a */
    add56(accum[0], lows[0], sx24((int32_t)words[1]), words[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103396u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103397: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x103397u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103399: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103399u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10339a: /* sub     x,a */
    sub56(accum[0], lows[0], sx24((int32_t)words[1]), words[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10339au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10339b: /* add     b,a             x:$c,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 12u)); add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10339bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10339c: /* move    x:$7,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 7u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10339cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10339d: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10339du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10339f: /* move    a10,l:<$f */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[15] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[15] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x10339fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a0: /* mpy     words[2],words[0],a         x:$15,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 21u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033a0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a1: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033a1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a2: /* move    x:$2,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033a2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a3: /* move    a,l:<$18 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[24] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[24] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1033a3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a4: /* mpysu   words[2],words[1],a */
    mpy_su32(sx24((int32_t)(words[2])), words[1], &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033a4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a5: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033a5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a6: /* move    a,words[2] */
    words[2] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033a6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a7: /* mpyr    words[2],words[0],a         b1,x:$15 */
    parallel_words[0] = u24(accum[1]); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); *(md_shared_scratch_x + 21u) = sx24((int32_t)(parallel_words[0]));
    MD_TRANSLATION_TRACE(0x1033a7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a8: /* move    x:$b,b */
    accum[1] = sx24((int32_t)(u24(*(md_shared_scratch_x + 11u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1033a8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033a9: /* move    a1,words[3] */
    words[3] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033a9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033aa: /* move    l:<$11,a */
    parallel_words[0] = u24(md_shared_scratch_x[17]); parallel_words[3] = u24(md_shared_scratch_y[17]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1033aau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ab: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033abu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ac: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033acu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ad: /* add     b,a             x:$9,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 9u)); add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033adu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ae: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1033aeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b0: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033b0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b2: /* move    a10,l:<$11 */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[17] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[17] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1033b2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b3: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033b3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b4: /* move    x:$a,words[0] */
    words[0] = (u24(*(md_shared_scratch_x + 10u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033b4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b5: /* add     words[0],a */
    add56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033b5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b6: /* nop */

    MD_TRANSLATION_TRACE(0x1033b6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b7: /* move    a,words[0] */
    words[0] = (u24(lim24(accum[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033b7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b8: /* mpyr    words[0],words[3],a */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033b8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033b9: /* nop */

    MD_TRANSLATION_TRACE(0x1033b9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ba: /* move    a1,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine, u24(accum[0]));
    MD_TRANSLATION_TRACE(0x1033bau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_103375 != 0u) goto L_103377;
L_1033bb: /* move    addresses[1],addresses[7] */
    addresses[7] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033bbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033bc: /* move    addresses[1],addresses[4] */
    addresses[4] = (addresses[1]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033bcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033bd: /* move    #$1a,addresses[2] */
    addresses[2] = (0x1au) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033bdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033be: /* move    y:(addresses[6]+$8),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 8u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1033beu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033bf: /* asr     #$1,a,b */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[1] = accum[2] >> 1u; lows[1] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033bfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c0: /* move    #$40,words[0] */
    words[0] = (((0x40u) << 16)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c1: /* add     words[0],b */
    add56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033c1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c2: /* move    a1,words[2] */
    words[2] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c3: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c4: /* move    y:(addresses[7])+,words[3] */
    words[3] = (md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c5: /* mpy     words[3],words[1],a */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033c5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c6: /* move    x:(addresses[2]),words[0] */
    words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c7: /* move    y:(addresses[6]+$1b),words[3] */
    words[3] = (u24(*(v->y + 27u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c8: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1033c8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033c9: /* macr    words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033c9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ca: /* mpy     words[3],words[1],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1033cau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033cb: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033cbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033cc: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1033ccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033cd: /* macr    words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033cdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ce: /* do      #<$f,>$1033d8 */
    loop_1033ce = 0xfu; if (loop_1033ce == 0u) goto L_1033d8;
    MD_TRANSLATION_TRACE(0x1033ceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d0: /* mpy     words[3],words[1],a         a,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[0])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1033d0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d1: /* move    x:(addresses[2]),words[0]       b,words[3] */
    parallel_words[0] = u24(lim24(accum[1])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033d1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d2: /* sub     words[0],a            a1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[0]); sub56(accum[0], lows[0], sx24((int32_t)(words[0])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1033d2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d3: /* macr    words[3],words[2],a         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[0], lows[0], accum[2], lows[2], &accum[0], &lows[0]); accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033d3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d4: /* mpy     words[3],words[1],b         b,y:(addresses[4])+ */
    parallel_words[0] = u24(lim24(accum[1])); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[1])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1033d4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d5: /* move    x:(addresses[2]),words[0]       a,words[3] */
    parallel_words[0] = u24(lim24(accum[0])); words[0] = (md_read_x_raw(v, md_shared_scratch_x, addresses[2], sine)) & 0xffffffu; words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033d5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d6: /* sub     words[0],b            b1,x:(addresses[2]) */
    parallel_words[0] = u24(accum[1]); sub56(accum[1], lows[1], sx24((int32_t)(words[0])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); md_write_x(v, md_shared_scratch_x, addresses[2], sine, parallel_words[0]);
    MD_TRANSLATION_TRACE(0x1033d6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d7: /* macr    words[3],words[2],b         y:(addresses[7])+,words[3] */
    parallel_words[0] = md_read_y_raw(v, md_shared_scratch_y, md_agu(&addresses[7], addresses[15], 1, 0u), sine); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); add56(accum[1], lows[1], accum[2], lows[2], &accum[1], &lows[1]); accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033d7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1033ce != 0u) goto L_1033d0;
L_1033d8: /* move    a,y:(addresses[4])+ */
    md_write_y(v, md_shared_scratch_y, md_agu(&addresses[4], addresses[12], 1, 0u), sine, u24(lim24(accum[0])));
    MD_TRANSLATION_TRACE(0x1033d8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033d9: /* move    b,y:(addresses[4]) */
    md_write_y(v, md_shared_scratch_y, addresses[4], sine, u24(lim24(accum[1])));
    MD_TRANSLATION_TRACE(0x1033d9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033da: /* move    b,y:(addresses[6]+$1b) */
    *(v->y + 27u) = sx24((int32_t)(u24(lim24(accum[1]))));
    MD_TRANSLATION_TRACE(0x1033dau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033db: /* move    l:<$d,a */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1033dbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033dc: /* move    l:<$f,b */
    parallel_words[0] = u24(md_shared_scratch_x[15]); parallel_words[3] = u24(md_shared_scratch_y[15]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1033dcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033dd: /* move    l:<$11,x */
    parallel_words[0] = u24(md_shared_scratch_x[17]); parallel_words[3] = u24(md_shared_scratch_y[17]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1033ddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033de: /* move    a1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1033deu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033df: /* move    a0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1033dfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e0: /* move    b1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1033e0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e1: /* move    b0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x1033e1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e2: /* move    words[1],y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1033e2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e3: /* move    words[0],y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1033e3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e4: /* move    x:$14,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 20u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1033e4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e5: /* move    x:$15,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 21u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1033e5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e6: /* move    a1,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1033e6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e7: /* move    b1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1033e7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e8: /* move    l:<$16,a */
    parallel_words[0] = u24(md_shared_scratch_x[22]); parallel_words[3] = u24(md_shared_scratch_y[22]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1033e8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033e9: /* move    l:<$18,b */
    parallel_words[0] = u24(md_shared_scratch_x[24]); parallel_words[3] = u24(md_shared_scratch_y[24]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1033e9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ea: /* move    x:$1a,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 26u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1033eau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033eb: /* move    a1,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1033ebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ec: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x1033ecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ed: /* move    b1,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1033edu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ee: /* move    b0,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x1033eeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033ef: /* move    words[1],y:(addresses[6]+$1a) */
    *(v->y + 26u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1033efu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1033f0: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t efmhh_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2026f8: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_2026fc: /* movem.l d[2]/a2-a3, (a7) */
    /* native locals replace original callee saves */
C_202700: /* movea.l $10(a7), a1 */
    /* resolved argument pointer */
C_202704: /* movea.l $14(a7), a2 */
    /* resolved argument pointer */
C_202708: /* move.w (a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_20270a: /* mulu.w #$5, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x5u);
C_20270e: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_202710: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_202716: /* lea.l $24bd94.l, a3 */
    /* resolved table pointer */
C_20271c: /* move.l d[0], d[1] */
    d[1] = d[0];
C_20271e: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_202720: /* addq.l #$4, d[1] */
    d[1] = d[1] + 0x4u;
C_202722: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_202724: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202726: /* move.l (a3, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_20272a: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20272c: /* move.l d[0], $4(a1) */
    *(y + (4u) / 4u) = (int32_t)(d[0]);
C_202730: /* move.w $2(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_202734: /* mulu.w #$46, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x46u);
C_202738: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_20273a: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_20273c: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20273e: /* movea.l d[0], a0 */
    a0 = d[0];
C_202740: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202746: /* move.l $24c(a0), $8(a1) */
    *(y + (8u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20274c: /* lea.l $c(a1), a0 */
    a0 = 12u;
C_202750: /* clr.l d[0] */
    d[0] = 0u;
C_202752: /* move.w $a(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_202756: /* move.b #$9, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x9u) & 0xffu);
C_20275a: /* lsl.l d[2], d[0] */
    d[0] = (((d[2]) & 63u) < 32u ? d[0] << ((d[2]) & 63u) : 0u);
C_20275c: /* move.l d[0], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[0]);
C_20275e: /* cmpi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu;
C_202764: /* bgt.b $20276c */
    if ((int32_t)lhs > (int32_t)rhs) goto C_20276c;
C_202766: /* move.l #$10000, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x10000u);
C_20276c: /* move.w $8(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_202770: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_202772: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_202774: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_20277a: /* move.l d[0], $10(a1) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20277e: /* move.w $c(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_202782: /* mulu.w #$52, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x52u);
C_202786: /* moveq #$c, d[2] */
    d[2] = 0x0000000cu;
C_202788: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_20278a: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20278c: /* movea.l d[0], a0 */
    a0 = d[0];
C_20278e: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202794: /* move.l $24c(a0), $14(a1) */
    *(y + (20u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20279a: /* clr.l $18(a1) */
    *(y + (24u) / 4u) = (int32_t)(0u);
C_20279e: /* move.w $e(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (14u) / 2u)) & 0xffffu);
C_2027a2: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2027a4: /* andi.l #$1fffe00, d[0] */
    lhs = d[0]; rhs = 0x1fffe00u; d[0] = lhs & rhs;
C_2027aa: /* move.l d[0], $1c(a1) */
    *(y + (28u) / 4u) = (int32_t)(d[0]);
C_2027ae: /* move.l #$f4240, d[0] */
    d[0] = 0xf4240u;
C_2027b4: /* move.l d[0], $20(a1) */
    *(y + (32u) / 4u) = (int32_t)(d[0]);
C_2027b8: /* clr.l d[0] */
    d[0] = 0u;
C_2027ba: /* move.w $4(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_2027be: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_2027c0: /* move.l d[0], $24(a1) */
    *(y + (36u) / 4u) = (int32_t)(d[0]);
C_2027c4: /* move.l #$7fffff, d[1] */
    d[1] = 0x7fffffu;
C_2027ca: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_2027cc: /* move.l d[1], $28(a1) */
    *(y + (40u) / 4u) = (int32_t)(d[1]);
C_2027d0: /* move.w $6(a2), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_2027d4: /* mulu.w #$c, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xcu);
C_2027d8: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2027da: /* addi.l #$40, d[0] */
    lhs = d[0]; rhs = 0x40u; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_2027e0: /* move.l d[0], d[1] */
    d[1] = d[0];
C_2027e2: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_2027e4: /* move.b #$7f, d[2] */
    d[2] = (d[2] & 0xffffff00u) | ((0x7fu) & 0xffu);
C_2027e8: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_2027ea: /* move.l (a3, d[0].l * 4), d[0] */
    d[0] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_2027ee: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_2027f0: /* asr.l #$4, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x4u) & 63u) < 32u ? ((0x4u) & 63u) : 31u));
C_2027f2: /* move.l d[0], $2c(a1) */
    *(y + (44u) / 4u) = (int32_t)(d[0]);
C_2027f6: /* move.l #$ffffff, d[0] */
    d[0] = 0xffffffu;
C_2027fc: /* move.l d[0], $30(a1) */
    *(y + (48u) / 4u) = (int32_t)(d[0]);
C_202800: /* moveq #$d, d[0] */
    d[0] = 0x0000000du;
C_202802: /* movem.l (a7), d[2]/a2-a3 */
    /* native locals replace original callee saves */
C_202806: /* lea.l $c(a7), a7 */
    /* local register save area */
C_20280a: /* rts  */
    return d[0];
}

void efmhh_update_coeffs(const uint32_t *p, EfmhhVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmhh_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmhh_dsp_init(v, 0); }
}
void efmhh_voice_init(EfmhhVoice *v) {
    uint32_t i;
    for (i = 0u; i < 28u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmhh_voice_trigger(EfmhhVoice *v) {
    efmhh_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmhh_resample_run(EfmhhVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmhhVoice
#define MD_RENDER48 efmhh_render_block_48k_float
#define MD_RENDER44 efmhh_render_block_44k1
#define MD_RESAMPLE_RUN efmhh_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmhhVoice
#define MD_SLOT_INIT efmhh_slot_init
#define MD_SLOT_PROCESS efmhh_slot_process
#define MD_VOICE_INIT efmhh_voice_init
#define MD_COEFFICIENTS efmhh_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmhh_update_coeffs
#define MD_VOICE_TRIGGER efmhh_voice_trigger
#define MD_RENDER44 efmhh_render_block_44k1
#define MD_RENDER48 efmhh_render_block_48k_float
#include "../engine_slot.inc"
#endif
