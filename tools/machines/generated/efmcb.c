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
    int32_t y[26];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
    uint32_t memory_fault;

} EfmcbVoice;
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

static uint32_t md_read_x(EfmcbVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if ((address >= 256u && address < 0x800u) || (address >= 0xc00u && address < 0x20000u)) { value = md_boot_x_read(address); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_x(EfmcbVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}
static uint32_t md_read_y(EfmcbVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine) {
    uint32_t value;
    value = 0u; address = address & 0xffffffu;
    if (address < 128u) { value = u24(*(scratch + address)); }
    else if (address >= 0x800u && address < 2074u) { value = u24(*(v->y + (address - 0x800u))); }
    else if (address >= 0x100u && address < 0x140u) { value = u24(*(v->blk44 + (address & 31u))); }
    else if (address >= 0x148000u && address < 0x150000u) { value = u24(*(sine + (address - 0x148000u))); }
    else { MD_MEMORY_FAULT(v,address); }
    return value;
}
static void md_write_y(EfmcbVoice *v, int32_t *scratch, uint32_t address, const int32_t *sine, uint32_t value) {
    address = address & 0xffffffu;
    if (address < 128u) { *(scratch + address) = sx24((int32_t)value); }
    else if (address >= 0x800u && address < 2074u) { *(v->y + (address - 0x800u)) = sx24((int32_t)value); }
    else if (address >= 0x100u && address < 0x140u) { *(v->blk44 + (address & 31u)) = sx24((int32_t)value); }
    else { MD_MEMORY_FAULT(v,address); }
}

static uint32_t md_read_x_raw(EfmcbVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_x(v,scratch,address,sine);
    return value;
}

static uint32_t md_read_y_raw(EfmcbVoice *v,int32_t *scratch,uint32_t address,const int32_t *sine) {
    uint32_t value;
    address = address & 0xffffffu; value=0u;
    if (address < 0xffff80u) value=md_read_y(v,scratch,address,sine);
    return value;
}
void efmcb_dsp_init(EfmcbVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_103272: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103272u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103273: /* nop */

    MD_TRANSLATION_TRACE(0x103273u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103274: /* move    a1,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103274u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103275: /* move    a1,y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103275u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103276: /* rts */
    return;
}

void efmcb_dsp_trigger(EfmcbVoice *v, const int32_t *sine) {
    int32_t accum[3] = {0,0,0};
    uint32_t lows[3] = {0,0,0};
    uint32_t words[4] = {0,0,0,0};
    uint32_t addresses[24] = {0u,0u,0u,0u,0u,0u,0x800u,0x100u,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0xffffffu,0u,0u,0u,0u,0u,0u,0u,0u};
    uint32_t flags[5] = {0,0,0,0,0};
    uint32_t repeat = 0;
    uint32_t long_address[3] = {0,0,0};
    uint32_t parallel_words[6] = {0,0,0,0,0,0};
    uint32_t shift_count[1];
L_103277: /* clr     a */
    accum[0] = 0; lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103277u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103278: /* move    #>$7fffff,words[1] */
    words[1] = (0x7fffffu) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103278u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10327a: /* move    #>$5a8279,words[3] */
    words[3] = (0x5a8279u) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10327au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10327c: /* move    a1,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10327cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10327d: /* move    a0,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10327du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10327e: /* move    a1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10327eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10327f: /* move    a0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10327fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103280: /* move    a1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103280u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103281: /* move    a0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103281u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103282: /* move    a1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103282u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103283: /* move    a0,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103283u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103284: /* move    a0,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103284u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103285: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103285u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103286: /* move    a0,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103286u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103287: /* move    a0,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103287u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103288: /* move    words[1],y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103288u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103289: /* move    words[3],y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x103289u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10328a: /* move    words[3],y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(words[3]));
    MD_TRANSLATION_TRACE(0x10328au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10328b: /* rts */
    return;
}

void efmcb_render_block_44k1(EfmcbVoice *v, const int32_t *sine) {
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
    uint32_t loop_1032b8 = 0;
md_shared_scratch_y[32] = sx24((int32_t)md_shared_y20);
L_10328c: /* move    y:(addresses[6]+$1),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 1u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10328cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10328d: /* move    y:(addresses[6]+$2),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 2u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10328du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10328e: /* move    a1,x:$1 */
    *(md_shared_scratch_x + 1u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10328eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10328f: /* move    b1,x:$2 */
    *(md_shared_scratch_x + 2u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10328fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103290: /* move    y:(addresses[6]+$3),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 3u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103290u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103291: /* move    y:(addresses[6]+$4),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 4u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103291u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103292: /* move    a1,x:$3 */
    *(md_shared_scratch_x + 3u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103292u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103293: /* move    b1,x:$4 */
    *(md_shared_scratch_x + 4u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103293u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103294: /* move    y:(addresses[6]+$5),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 5u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103294u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103295: /* move    y:(addresses[6]+$6),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 6u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103295u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103296: /* move    a1,x:$5 */
    *(md_shared_scratch_x + 5u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103296u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103297: /* move    b1,x:$6 */
    *(md_shared_scratch_x + 6u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103297u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103298: /* move    y:(addresses[6]+$7),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 7u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103298u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103299: /* move    y:(addresses[6]+$8),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 8u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103299u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10329a: /* move    a1,x:$7 */
    *(md_shared_scratch_x + 7u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10329au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10329b: /* move    b1,x:$8 */
    *(md_shared_scratch_x + 8u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10329bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10329c: /* move    y:(addresses[6]+$9),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 9u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10329cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10329d: /* move    y:(addresses[6]+$a),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 10u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x10329du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10329e: /* move    a1,x:$9 */
    *(md_shared_scratch_x + 9u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10329eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10329f: /* move    b1,x:$a */
    *(md_shared_scratch_x + 10u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10329fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a0: /* move    y:(addresses[6]+$b),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 11u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1032a0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a1: /* move    y:(addresses[6]+$c),a0 */
    lows[0] = (u24(*(v->y + 12u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032a1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a2: /* move    y:(addresses[6]+$d),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 13u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1032a2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a3: /* move    y:(addresses[6]+$e),b0 */
    lows[1] = (u24(*(v->y + 14u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032a3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a4: /* move    a,l:<$b */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[11] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[11] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032a4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a5: /* move    b,l:<$d */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032a5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a6: /* move    y:(addresses[6]+$f),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 15u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1032a6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a7: /* move    y:(addresses[6]+$10),a0 */
    lows[0] = (u24(*(v->y + 16u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032a7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a8: /* move    y:(addresses[6]+$11),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 17u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1032a8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032a9: /* move    y:(addresses[6]+$12),b0 */
    lows[1] = (u24(*(v->y + 18u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032a9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032aa: /* move    a,l:<$f */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[15] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[15] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032aau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ab: /* move    b,l:<$11 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[17] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[17] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032abu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ac: /* move    y:(addresses[6]+$13),a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(v->y + 19u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1032acu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ad: /* move    y:(addresses[6]+$14),b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(v->y + 20u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x1032adu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ae: /* move    y:(addresses[6]+$15),words[1] */
    words[1] = (u24(*(v->y + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032aeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032af: /* move    a1,x:$13 */
    *(md_shared_scratch_x + 19u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x1032afu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b0: /* move    b1,x:$14 */
    *(md_shared_scratch_x + 20u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1032b0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b1: /* move    words[1],x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x1032b1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b2: /* move    y:(addresses[6]+$16),a */
    accum[0] = sx24((int32_t)(u24(*(v->y + 22u)))); lows[0] = 0u;
    MD_TRANSLATION_TRACE(0x1032b2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b3: /* move    y:(addresses[6]+$17),a0 */
    lows[0] = (u24(*(v->y + 23u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032b3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b4: /* move    y:(addresses[6]+$18),b */
    accum[1] = sx24((int32_t)(u24(*(v->y + 24u)))); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x1032b4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b5: /* move    y:(addresses[6]+$19),b0 */
    lows[1] = (u24(*(v->y + 25u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032b5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b6: /* move    a,l:<$16 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[22] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[22] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032b6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b7: /* move    b,l:<$18 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[1]))); parallel_words[3] = (accum[1] > 0x7fffff ? 0xffffffu : accum[1] < -0x800000 ? 0u : lows[1]); md_shared_scratch_x[24] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[24] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032b7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032b8: /* do      #<$20,>$103315 */
    loop_1032b8 = 0x20u; if (loop_1032b8 == 0u) goto L_103315;
    MD_TRANSLATION_TRACE(0x1032b8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ba: /* move    x:$3,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 3u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032bau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032bb: /* move    x:$1,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 1u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032bbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032bc: /* mpy     words[3],words[2],b */
    mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032bcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032bd: /* move    l:<$b,a */
    parallel_words[0] = u24(md_shared_scratch_x[11]); parallel_words[3] = u24(md_shared_scratch_y[11]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1032bdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032be: /* asr     #$5,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 5u; lows[1] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032beu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032bf: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032bfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c0: /* add     b,a             x:$13,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 19u)); add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032c0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c1: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1032c1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c3: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032c3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c5: /* move    a10,l:<$b */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[11] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[11] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032c5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c6: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032c6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c7: /* nop */

    MD_TRANSLATION_TRACE(0x1032c7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c8: /* move    a1,words[0] */
    words[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032c8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032c9: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032c9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ca: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032cau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032cb: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032cbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032cc: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032ccu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032cd: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032cdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ce: /* add     words[3],b            l:<$16,x */
    parallel_words[0] = u24(md_shared_scratch_x[22]); parallel_words[3] = u24(md_shared_scratch_y[22]); add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1032ceu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032cf: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032cfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d0: /* move    l:<$d,a */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1032d0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d1: /* add     x,a */
    add56(accum[0], lows[0], sx24((int32_t)words[1]), words[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032d1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d2: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1032d2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d4: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032d4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d5: /* sub     x,a */
    sub56(accum[0], lows[0], sx24((int32_t)words[1]), words[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032d5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d6: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032d6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d7: /* move    x:$6,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032d7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032d8: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032d8u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032da: /* move    a10,l:<$d */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[13] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[13] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032dau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032db: /* mpy     words[2],words[0],a         x:$3,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 3u)); mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032dbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032dc: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032dcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032dd: /* move    words[0],x:$1a */
    *(md_shared_scratch_x + 26u) = sx24((int32_t)(words[0]));
    MD_TRANSLATION_TRACE(0x1032ddu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032de: /* move    a,l:<$16 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[22] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[22] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032deu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032df: /* move    x:$2,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 2u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032dfu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e0: /* mpy     words[3],words[2],b         x:$5,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 5u)); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032e0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e1: /* move    l:<$f,a */
    parallel_words[0] = u24(md_shared_scratch_x[15]); parallel_words[3] = u24(md_shared_scratch_y[15]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1032e1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e2: /* asr     #$5,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 5u; lows[1] = ((lows[2] >> 5u) | ((uint32_t)accum[2] << 19u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032e2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e3: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032e3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e4: /* add     b,a             x:$13,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 19u)); add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032e4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e5: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1032e5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e7: /* move    x:(addresses[0]+$148000),words[0] */
    words[0] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032e7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032e9: /* move    a10,l:<$f */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[15] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[15] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032e9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ea: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032eau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032eb: /* mpyr    words[2],words[0],a */
    mpy_ss32(sx24((int32_t)(words[2])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032ebu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ec: /* move    b1,x:$13 */
    *(md_shared_scratch_x + 19u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x1032ecu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ed: /* move    a1,words[0] */
    words[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032edu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ee: /* mpyr    words[0],words[3],b */
    mpy_ss32(sx24((int32_t)(words[0])), sx24((int32_t)(words[3])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; accum[1] = (int32_t)((uint32_t)accum[1] + (uint32_t)(lows[1] > 0x800000u || (lows[1] == 0x800000u && (accum[1] & 1)))); lows[1] = 0u; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032eeu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ef: /* move    x:$4,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 4u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032efu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f0: /* move    b1,words[1] */
    words[1] = (u24(accum[1])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032f0u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f1: /* mpy     words[1],words[2],b */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032f1u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f2: /* asl     #$3,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = (int32_t)(((uint32_t)accum[2] << 3u) | (lows[2] >> 21u)); lows[1] = (lows[2] << 3u) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032f2u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f3: /* add     words[3],b            l:<$18,x */
    parallel_words[0] = u24(md_shared_scratch_x[24]); parallel_words[3] = u24(md_shared_scratch_y[24]); add56(accum[1], lows[1], sx24((int32_t)(words[3])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = parallel_words[0]; words[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1032f3u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f4: /* asr     #$8,b,b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 8u; lows[1] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032f4u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f5: /* move    l:<$11,a */
    parallel_words[0] = u24(md_shared_scratch_x[17]); parallel_words[3] = u24(md_shared_scratch_y[17]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x1032f5u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f6: /* add     x,a */
    add56(accum[0], lows[0], sx24((int32_t)words[1]), words[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032f6u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f7: /* and     #>$7fff,a */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | (u24(accum[0]) & 0x7fffu));
    MD_TRANSLATION_TRACE(0x1032f7u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032f9: /* move    a1,addresses[0] */
    addresses[0] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032f9u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032fa: /* sub     x,a */
    sub56(accum[0], lows[0], sx24((int32_t)words[1]), words[0], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032fau,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032fb: /* add     b,a */
    add56(accum[0], lows[0], accum[1], lows[1], &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x1032fbu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032fc: /* move    x:$6,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 6u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032fcu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032fd: /* move    x:(addresses[0]+$148000),words[1] */
    words[1] = (u24(*(sine + addresses[0]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x1032fdu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_1032ff: /* move    a10,l:<$11 */
    parallel_words[0] = (uint32_t)(accum[0]); parallel_words[3] = lows[0]; md_shared_scratch_x[17] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[17] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x1032ffu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103300: /* mpy     words[1],words[2],a         x:$1a,b */
    parallel_words[0] = u24(*(md_shared_scratch_x + 26u)); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); accum[1] = sx24((int32_t)(parallel_words[0])); lows[1] = 0u;
    MD_TRANSLATION_TRACE(0x103300u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103301: /* asr     #$8,a,a */
    accum[2] = accum[0]; lows[2] = lows[0]; accum[0] = accum[2] >> 8u; lows[0] = ((lows[2] >> 8u) | ((uint32_t)accum[2] << 16u)) & 0xffffffu; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103301u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103302: /* move    x:$14,words[2] */
    words[2] = (u24(*(md_shared_scratch_x + 20u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103302u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103303: /* move    a,l:<$18 */
    parallel_words[0] = (uint32_t)(u24(lim24(accum[0]))); parallel_words[3] = (accum[0] > 0x7fffff ? 0xffffffu : accum[0] < -0x800000 ? 0u : lows[0]); md_shared_scratch_x[24] = sx24((int32_t)parallel_words[0]); md_shared_scratch_y[24] = sx24((int32_t)parallel_words[3]);
    MD_TRANSLATION_TRACE(0x103303u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103304: /* add     words[1],b            x:$7,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 7u)); add56(accum[1], lows[1], sx24((int32_t)(words[1])), 0u, &accum[1], &lows[1]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103304u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103305: /* move    x:$8,words[3] */
    words[3] = (u24(*(md_shared_scratch_x + 8u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103305u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103306: /* asr     b */
    accum[2] = accum[1]; lows[2] = lows[1]; accum[1] = accum[2] >> 1u; lows[1] = ((lows[2] >> 1u) | ((uint32_t)accum[2] << 23u)) & 0xffffffu; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103306u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103307: /* nop */

    MD_TRANSLATION_TRACE(0x103307u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103308: /* move    b,words[0] */
    words[0] = (u24(lim24(accum[1]))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103308u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103309: /* mpy     words[3],words[2],b         x:$a,words[3] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 10u)); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[3] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103309u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10330a: /* mpyr    words[1],words[2],a         x:$15,words[2] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 21u)); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[2] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10330au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10330b: /* move    b1,x:$14 */
    *(md_shared_scratch_x + 20u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10330bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10330c: /* mpy     words[3],words[2],b         x:$9,words[1] */
    parallel_words[0] = u24(*(md_shared_scratch_x + 9u)); mpy_ss32(sx24((int32_t)(words[3])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[1] = accum[2]; lows[1] = lows[2]; flags[3] = (accum[1] < 0); flags[0] = (accum[1] == 0 && lows[1] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10330cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10330d: /* mpyr    words[1],words[2],a         a1,words[1] */
    parallel_words[0] = u24(accum[0]); mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[2])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]); words[1] = (parallel_words[0]) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x10330du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10330e: /* move    b1,x:$15 */
    *(md_shared_scratch_x + 21u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10330eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10330f: /* add     words[1],a */
    add56(accum[0], lows[0], sx24((int32_t)(words[1])), 0u, &accum[0], &lows[0]); flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x10330fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103310: /* nop */

    MD_TRANSLATION_TRACE(0x103310u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103311: /* move    a1,words[1] */
    words[1] = (u24(accum[0])) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103311u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103312: /* mpyr    words[1],words[0],a */
    mpy_ss32(sx24((int32_t)(words[1])), sx24((int32_t)(words[0])), &accum[2], &lows[2]); accum[0] = accum[2]; lows[0] = lows[2]; accum[0] = (int32_t)((uint32_t)accum[0] + (uint32_t)(lows[0] > 0x800000u || (lows[0] == 0x800000u && (accum[0] & 1)))); lows[0] = 0u; flags[3] = (accum[0] < 0); flags[0] = (accum[0] == 0 && lows[0] == 0u); flags[1] = flags[3]; flags[2] = (!flags[0] && !flags[3]);
    MD_TRANSLATION_TRACE(0x103312u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103313: /* nop */

    MD_TRANSLATION_TRACE(0x103313u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103314: /* move    a1,y:(addresses[7])+ */
    md_write_y(v, md_shared_scratch_y, addresses[7]++, sine, u24(accum[0]));
    MD_TRANSLATION_TRACE(0x103314u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
    if (--loop_1032b8 != 0u) goto L_1032ba;
L_103315: /* move    l:<$b,a */
    parallel_words[0] = u24(md_shared_scratch_x[11]); parallel_words[3] = u24(md_shared_scratch_y[11]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103315u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103316: /* move    l:<$d,b */
    parallel_words[0] = u24(md_shared_scratch_x[13]); parallel_words[3] = u24(md_shared_scratch_y[13]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103316u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103317: /* move    a1,y:(addresses[6]+$b) */
    *(v->y + 11u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103317u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103318: /* move    a0,y:(addresses[6]+$c) */
    *(v->y + 12u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x103318u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103319: /* move    b1,y:(addresses[6]+$d) */
    *(v->y + 13u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103319u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10331a: /* move    b0,y:(addresses[6]+$e) */
    *(v->y + 14u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x10331au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10331b: /* move    l:<$f,a */
    parallel_words[0] = u24(md_shared_scratch_x[15]); parallel_words[3] = u24(md_shared_scratch_y[15]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10331bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10331c: /* move    l:<$11,b */
    parallel_words[0] = u24(md_shared_scratch_x[17]); parallel_words[3] = u24(md_shared_scratch_y[17]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x10331cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10331d: /* move    a1,y:(addresses[6]+$f) */
    *(v->y + 15u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x10331du,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10331e: /* move    a0,y:(addresses[6]+$10) */
    *(v->y + 16u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10331eu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10331f: /* move    b1,y:(addresses[6]+$11) */
    *(v->y + 17u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10331fu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103320: /* move    b0,y:(addresses[6]+$12) */
    *(v->y + 18u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x103320u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103321: /* move    x:$13,a1 */
    accum[0] = (int32_t)(((uint32_t)accum[0] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 19u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103321u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103322: /* move    x:$14,b1 */
    accum[1] = (int32_t)(((uint32_t)accum[1] & 0xff000000u) | ((u24(*(md_shared_scratch_x + 20u))) & 0xffffffu));
    MD_TRANSLATION_TRACE(0x103322u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103323: /* move    x:$15,words[1] */
    words[1] = (u24(*(md_shared_scratch_x + 21u))) & 0xffffffu;
    MD_TRANSLATION_TRACE(0x103323u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103324: /* move    a1,y:(addresses[6]+$13) */
    *(v->y + 19u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103324u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103325: /* move    b1,y:(addresses[6]+$14) */
    *(v->y + 20u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x103325u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103326: /* move    words[1],y:(addresses[6]+$15) */
    *(v->y + 21u) = sx24((int32_t)(words[1]));
    MD_TRANSLATION_TRACE(0x103326u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103327: /* move    l:<$16,a */
    parallel_words[0] = u24(md_shared_scratch_x[22]); parallel_words[3] = u24(md_shared_scratch_y[22]); accum[0] = sx24((int32_t)parallel_words[0]); lows[0] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103327u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103328: /* move    l:<$18,b */
    parallel_words[0] = u24(md_shared_scratch_x[24]); parallel_words[3] = u24(md_shared_scratch_y[24]); accum[1] = sx24((int32_t)parallel_words[0]); lows[1] = parallel_words[3];
    MD_TRANSLATION_TRACE(0x103328u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_103329: /* move    a1,y:(addresses[6]+$16) */
    *(v->y + 22u) = sx24((int32_t)(u24(accum[0])));
    MD_TRANSLATION_TRACE(0x103329u,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10332a: /* move    a0,y:(addresses[6]+$17) */
    *(v->y + 23u) = sx24((int32_t)(lows[0]));
    MD_TRANSLATION_TRACE(0x10332au,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10332b: /* move    b1,y:(addresses[6]+$18) */
    *(v->y + 24u) = sx24((int32_t)(u24(accum[1])));
    MD_TRANSLATION_TRACE(0x10332bu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10332c: /* move    b0,y:(addresses[6]+$19) */
    *(v->y + 25u) = sx24((int32_t)(lows[1]));
    MD_TRANSLATION_TRACE(0x10332cu,accum[0],lows[0],accum[1],lows[1],words[0],words[1],words[2],words[3],addresses[2],addresses[4],addresses[7]);
L_10332d: /* rts */
    md_shared_y20 = u24(md_shared_scratch_y[32]); return;
}

uint32_t efmcb_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d[8] = {0,0,0,0,0,0,0,0};
    uint32_t a0 = 0, a1 = 0, a2 = 0, a3 = 0, a4 = 0, a5 = 0, a6 = 0;
    uint32_t lhs = 0, rhs = 0;
    uint32_t cf_carry[1] = {0};
    uint32_t cf_temp[1] = {0};
C_2025e6: /* lea.l -$c(a7), a7 */
    /* local register save area */
C_2025ea: /* movem.l d[2]-d[3]/a2, (a7) */
    /* native locals replace original callee saves */
C_2025ee: /* movea.l $10(a7), a2 */
    /* resolved argument pointer */
C_2025f2: /* movea.l $14(a7), a1 */
    /* resolved argument pointer */
C_2025f6: /* move.w (a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (0u) / 2u)) & 0xffffu);
C_2025f8: /* mulu.w #$5, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x5u);
C_2025fc: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_2025fe: /* move.l d[0], d[1] */
    d[1] = d[0];
C_202600: /* asr.l #$7, d[1] */
    d[1] = (uint32_t)((int32_t)d[1] >> (((0x7u) & 63u) < 32u ? ((0x7u) & 63u) : 31u));
C_202602: /* addq.l #$4, d[1] */
    d[1] = d[1] + 0x4u;
C_202604: /* moveq #$7f, d[2] */
    d[2] = 0x0000007fu;
C_202606: /* and.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; d[0] = lhs & rhs;
C_202608: /* lea.l $24bd94.l, a0 */
    /* resolved table pointer */
C_20260e: /* move.l (a0, d[0].l * 4), d[2] */
    d[2] = *(control_24bb90 + ((516u + (d[0] * 4u + (0))) / 4u));
C_202612: /* lsl.l d[1], d[2] */
    d[2] = (((d[1]) & 63u) < 32u ? d[2] << ((d[1]) & 63u) : 0u);
C_202614: /* move.l d[2], $4(a2) */
    *(y + (4u) / 4u) = (int32_t)(d[2]);
C_202618: /* move.l d[2], d[0] */
    d[0] = d[2];
C_20261a: /* add.l d[0], d[0] */
    lhs = d[0]; rhs = d[0]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20261c: /* move.l d[2], d[1] */
    d[1] = d[2];
C_20261e: /* lsl.l #$3, d[1] */
    d[1] = (((0x3u) & 63u) < 32u ? d[1] << ((0x3u) & 63u) : 0u);
C_202620: /* sub.l d[0], d[1] */
    lhs = d[1]; rhs = d[0]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_202622: /* move.l d[1], d[0] */
    d[0] = d[1];
C_202624: /* lsl.l #$6, d[0] */
    d[0] = (((0x6u) & 63u) < 32u ? d[0] << ((0x6u) & 63u) : 0u);
C_202626: /* sub.l d[1], d[0] */
    lhs = d[0]; rhs = d[1]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_202628: /* add.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs > 0xffffffffu - rhs); d[0] = lhs + rhs;
C_20262a: /* asr.l #$8, d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((0x8u) & 63u) < 32u ? ((0x8u) & 63u) : 31u));
C_20262c: /* move.l d[0], $8(a2) */
    *(y + (8u) / 4u) = (int32_t)(d[0]);
C_202630: /* lea.l $c(a2), a0 */
    a0 = 12u;
C_202634: /* clr.l d[0] */
    d[0] = 0u;
C_202636: /* move.w $a(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (10u) / 2u)) & 0xffffu);
C_20263a: /* moveq #$9, d[1] */
    d[1] = 0x00000009u;
C_20263c: /* lsl.l d[1], d[0] */
    d[0] = (((d[1]) & 63u) < 32u ? d[0] << ((d[1]) & 63u) : 0u);
C_20263e: /* move.l d[0], (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(d[0]);
C_202640: /* cmpi.l #$ffff, d[0] */
    lhs = d[0]; rhs = 0xffffu;
C_202646: /* bgt.b $20264e */
    if ((int32_t)lhs > (int32_t)rhs) goto C_20264e;
C_202648: /* move.l #$10000, (a0) */
    *(y + ((0u) + a0) / 4u) = (int32_t)(0x10000u);
C_20264e: /* move.w $8(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (8u) / 2u)) & 0xffffu);
C_202652: /* lsl.l #$8, d[0] */
    d[0] = (((0x8u) & 63u) < 32u ? d[0] << ((0x8u) & 63u) : 0u);
C_202654: /* andi.l #$ffff00, d[0] */
    lhs = d[0]; rhs = 0xffff00u; d[0] = lhs & rhs;
C_20265a: /* move.l d[0], $10(a2) */
    *(y + (16u) / 4u) = (int32_t)(d[0]);
C_20265e: /* move.w $c(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (12u) / 2u)) & 0xffffu);
C_202662: /* mulu.w #$52, d[0] */
    d[0] = (d[0] & 0xffffu) * (0x52u);
C_202666: /* moveq #$c, d[2] */
    d[2] = 0x0000000cu;
C_202668: /* asr.l d[2], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[2]) & 63u) < 32u ? ((d[2]) & 63u) : 31u));
C_20266a: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_20266c: /* movea.l d[0], a0 */
    a0 = d[0];
C_20266e: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_202674: /* move.l $24c(a0), $14(a2) */
    *(y + (20u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_20267a: /* move.w $6(a1), d[0] */
    d[0] = (d[0] & 0xffff0000u) | ((*(p + (6u) / 2u)) & 0xffffu);
C_20267e: /* mulu.w #$c0, d[0] */
    d[0] = (d[0] & 0xffffu) * (0xc0u);
C_202682: /* move.l d[0], $18(a2) */
    *(y + (24u) / 4u) = (int32_t)(d[0]);
C_202686: /* move.w $4(a1), d[3] */
    d[3] = (d[3] & 0xffff0000u) | ((*(p + (4u) / 2u)) & 0xffffu);
C_20268a: /* mulu.w #$1a4, d[3] */
    d[3] = (d[3] & 0xffffu) * (0x1a4u);
C_20268e: /* move.l #$7ffe00, d[1] */
    d[1] = 0x7ffe00u;
C_202694: /* sub.l d[3], d[1] */
    lhs = d[1]; rhs = d[3]; cf_carry[0] = (lhs < rhs); d[1] = lhs - rhs;
C_202696: /* move.l d[1], $1c(a2) */
    *(y + (28u) / 4u) = (int32_t)(d[1]);
C_20269a: /* clr.l d[2] */
    d[2] = 0u;
C_20269c: /* move.w $2(a1), d[2] */
    d[2] = (d[2] & 0xffff0000u) | ((*(p + (2u) / 2u)) & 0xffffu);
C_2026a0: /* move.l d[2], d[0] */
    d[0] = d[2];
C_2026a2: /* lsl.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] << ((0x3u) & 63u) : 0u);
C_2026a4: /* movea.l d[0], a0 */
    a0 = d[0];
C_2026a6: /* lea.l (a0, d[2].l * 2), a0 */
    a0 = (d[2] * 2u + (0)) + a0;
C_2026aa: /* move.l a0, d[0] */
    d[0] = a0;
C_2026ac: /* lsl.l #$3, d[0] */
    d[0] = (((0x3u) & 63u) < 32u ? d[0] << ((0x3u) & 63u) : 0u);
C_2026ae: /* sub.l a0, d[0] */
    lhs = d[0]; rhs = a0; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_2026b0: /* moveq #$c, d[1] */
    d[1] = 0x0000000cu;
C_2026b2: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2026b4: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2026b6: /* movea.l d[0], a0 */
    a0 = d[0];
C_2026b8: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2026be: /* move.l $24c(a0), $20(a2) */
    *(y + (32u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((588u) + a0)) / 4u)));
C_2026c4: /* move.l d[3], $24(a2) */
    *(y + (36u) / 4u) = (int32_t)(d[3]);
C_2026c8: /* move.l d[2], d[0] */
    d[0] = d[2];
C_2026ca: /* lsl.l #$4, d[0] */
    d[0] = (((0x4u) & 63u) < 32u ? d[0] << ((0x4u) & 63u) : 0u);
C_2026cc: /* movea.l d[0], a0 */
    a0 = d[0];
C_2026ce: /* lea.l (a0, d[2].l * 4), a0 */
    a0 = (d[2] * 4u + (0)) + a0;
C_2026d2: /* move.l a0, d[0] */
    d[0] = a0;
C_2026d4: /* sub.l d[2], d[0] */
    lhs = d[0]; rhs = d[2]; cf_carry[0] = (lhs < rhs); d[0] = lhs - rhs;
C_2026d6: /* move.b #$b, d[1] */
    d[1] = (d[1] & 0xffffff00u) | ((0xbu) & 0xffu);
C_2026da: /* asr.l d[1], d[0] */
    d[0] = (uint32_t)((int32_t)d[0] >> (((d[1]) & 63u) < 32u ? ((d[1]) & 63u) : 31u));
C_2026dc: /* lsl.l #$2, d[0] */
    d[0] = (((0x2u) & 63u) < 32u ? d[0] << ((0x2u) & 63u) : 0u);
C_2026de: /* movea.l d[0], a0 */
    a0 = d[0];
C_2026e0: /* adda.l #$24bf94, a0 */
    /* resolved pointer base */
C_2026e6: /* move.l $190(a0), $28(a2) */
    *(y + (40u) / 4u) = (int32_t)(*(control_24bb90 + ((1028u + ((400u) + a0)) / 4u)));
C_2026ec: /* moveq #$b, d[0] */
    d[0] = 0x0000000bu;
C_2026ee: /* movem.l (a7), d[2]-d[3]/a2 */
    /* native locals replace original callee saves */
C_2026f2: /* lea.l $c(a7), a7 */
    /* local register save area */
C_2026f6: /* rts  */
    return d[0];
}

void efmcb_update_coeffs(const uint32_t *p, EfmcbVoice *v, uint32_t trigger) {
    int32_t coefficients[32];
    uint32_t i, count;
    for (i = 0u; i < 32u; i++) { coefficients[i] = 0; }
    coefficients[0] = (int32_t)trigger;
    count = efmcb_compute_coeffs(p, coefficients);
    for (i = 1u; i < count; i++) { v->y[i] = sx24(coefficients[i]); }
    if (trigger && v->active == 0u) { efmcb_dsp_init(v, 0); }
}
void efmcb_voice_init(EfmcbVoice *v) {
    uint32_t i;
    for (i = 0u; i < 26u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    v->prev_s24 = 0; v->pos_int = 32u; v->pos_frac = 0u; v->active = 0u; v->memory_fault = 0u;


}
void efmcb_voice_trigger(EfmcbVoice *v) {
    efmcb_dsp_trigger(v, 0);
    v->active = 1u;
}
#ifndef MD_DSP_HOST_TEST
extern uint32_t efmcb_resample_run(EfmcbVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE EfmcbVoice
#define MD_RENDER48 efmcb_render_block_48k_float
#define MD_RENDER44 efmcb_render_block_44k1
#define MD_RESAMPLE_RUN efmcb_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE
#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE EfmcbVoice
#define MD_SLOT_INIT efmcb_slot_init
#define MD_SLOT_PROCESS efmcb_slot_process
#define MD_VOICE_INIT efmcb_voice_init
#define MD_COEFFICIENTS efmcb_compute_coeffs
#define MD_CONTROL_WITH_TRIGGER efmcb_update_coeffs
#define MD_VOICE_TRIGGER efmcb_voice_trigger
#define MD_RENDER44 efmcb_render_block_44k1
#define MD_RENDER48 efmcb_render_block_48k_float
#include "../engine_slot.inc"
#endif
