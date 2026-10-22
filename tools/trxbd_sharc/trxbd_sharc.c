/* Native SHARC+ (ADSP-21569) port of original Machinedrum OS 1.63 TRX-BD.
 *
 * Executes:
 *   1. DSP2 boot sine-table generator (P:$100068..$10008D -> 32,768 24-bit words)
 *   2. ColdFire TRX-BD coefficient routine (0x20122A..0x2013A5 -> 14 24-bit words)
 *   3. DSP56303 TRX-BD voice kernel (init P:$1003DA, trigger P:$1003E1, render P:$100408..$10050B)
 *   4. Exact 147/160 (44.1 kHz -> 48.0 kHz) rational streaming interpolation into Digitone II's
 *      pre-filter/pre-AMP voice buffer (DM 0x254a60 + 4*t).
 *
 * Written in strict 32-bit C compatible with selcc (no 64-bit types, one declaration per line,
 * no comparison initializers, no unions, simple pointer indexing).
 */
#include <stdint.h>
#include "trxbd_tables.h"
#include "../md_runtime/voice.h"

#define TRXBD_MACHINE_TYPE 5u
#define TRXBD_MAGIC 0x54525842u /* 'TRXB' */

/* TRX-BD SYN encoder slots in the ColdFire->SHARC frame copy at 0x25c48c:
 * A: PTCH -> record 254 (slot 41, offset 250)
 * B: DEC  -> record 255 (slot 42, offset 252)
 * C: RAMP -> record 252 (slot 38, offset 244)
 * D: RDEC -> record 253 (slot 40, offset 248)
 * E: STRT -> record 256 (slot 43, offset 254)
 * F: NOIS -> record 257 (slot 44, offset 256)
 * G: HARM -> record 260 (slot 47, offset 262)
 * H: CLIP -> record 259 (slot 46, offset 260)
 */
static const uint32_t trxbd_param_frame_offsets[8] = {
    250u, 252u, 244u, 248u, 254u, 256u, 262u, 260u
};

typedef struct {
    int32_t y[36];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
} TrxBdVoice;

typedef struct {
    uint32_t magic;
    uint32_t blocks_rendered;
    TrxBdVoice voices[16];
} TrxBdState;

#ifndef TRXBD_HOST_TEST
#define TRXBD_STATE_ADDR ((TrxBdState *)0x00300000u)
#define TRXBD_SINE_ADDR  ((int32_t *)0x80550000u)
#define DN2_VOICE_BUF_PTRS ((uint32_t *)0x00254a60u)
#define DN2_FRAME_COPY_ADDR 0x0025c48cu
#endif

#ifdef TRXBD_HOST_TEST
#define MD_DSP_HOST_TEST
#endif
#include "../md_runtime/arithmetic.h"
#define MD_BUILD_SINE trxbd_build_sine_table
#include "../md_runtime/sine.inc"
#undef MD_BUILD_SINE

void trxbd_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t p0;
    uint32_t p1;
    uint32_t p2;
    uint32_t p3;
    uint32_t p4;
    uint32_t p5;
    uint32_t p6;
    uint32_t p7;
    uint32_t d0;
    uint32_t d1;
    uint32_t idx;
    uint32_t pitch_w;
    uint32_t dec_w;
    p0 = *(p + 0);
    p1 = *(p + 1);
    p2 = *(p + 2);
    p3 = *(p + 3);
    p4 = *(p + 4);
    p5 = *(p + 5);
    p6 = *(p + 6);
    p7 = *(p + 7);

    d0 = (p0 >> 6) + 0x40u;
    d1 = d0 >> 7;
    idx = d0 & 0x7fu;
    pitch_w = *(trxbd_t_pitch + idx);
    *(y + 3) = sx24((int32_t)((pitch_w << d1) >> 1));

    idx = ((p1 * 0x50u) >> 12) + 120u;
    if (idx > 437u) {
        idx = 437u;
    }
    dec_w = *(trxbd_t_decay + idx);
    *(y + 1) = sx24((int32_t)dec_w);

    d0 = (p2 * p2) >> 18;
    d0 = ((d0 * p2) >> 14) + 3u;
    *(y + 4) = sx24((int32_t)d0);

    idx = (p3 * 0x46u) >> 12;
    if (idx > 437u) {
        idx = 437u;
    }
    dec_w = *(trxbd_t_decay + idx);
    *(y + 5) = sx24((int32_t)dec_w);

    *(y + 2) = sx24((int32_t)((p4 << 9) & 0x01fffe00u));

    if (p5 <= 0x1fffu) {
        *(y + 11) = sx24((int32_t)(p5 << 8));
        dec_w = *(trxbd_t_decay + 0);
        *(y + 12) = sx24((int32_t)dec_w);
    } else {
        *(y + 11) = 0x00200000;
        idx = (p5 * 0x28u - 0x50000u) >> 12;
        if (idx > 437u) {
            idx = 437u;
        }
        dec_w = *(trxbd_t_decay + idx);
        *(y + 12) = sx24((int32_t)dec_w);
    }

    *(y + 13) = sx24((int32_t)((p6 << 9) & 0x01fffe00u));

    d0 = p7 * 125u;
    *(y + 8) = sx24((int32_t)(0x001fffffu - d0));
    *(y + 9) = sx24((int32_t)(d0 + 0xffe00000u));
    d0 = (p7 * p7) >> 14;
    *(y + 10) = sx24((int32_t)(d0 * 511u));
    if (p7 <= 0x1fffu) {
        *(y + 6) = 0x007fffff;
        *(y + 7) = -0x00800000;
    } else {
        d0 = p7 * 100u;
        *(y + 6) = sx24((int32_t)(0x001c7fffu - d0));
        *(y + 7) = sx24((int32_t)(d0 + 0xffe38000u));
    }
}

void trxbd_voice_init(TrxBdVoice *v) {
    uint32_t i;
    int32_t *y;
    int32_t *b;
    uint32_t *lp;
    y = v->y;
    b = v->blk44;
    lp = v->last_p;
    for (i = 0u; i < 36u; i++) {
        *(y + i) = 0;
    }
    for (i = 0u; i < 32u; i++) {
        *(b + i) = 0;
    }
    for (i = 0u; i < 8u; i++) {
        *(lp + i) = 0xffffffffu;
    }
    *(y + 0x1c) = 0;
    *(y + 0x1d) = 1;
    v->prev_s24 = 0;
    v->pos_int = 32u;
    v->pos_frac = 0u;
    v->active = 0u;
}

void trxbd_voice_trigger(TrxBdVoice *v) {
    int32_t *y;
    int32_t b_hi;
    int32_t x0;
    int32_t x1;
    y = v->y;
    b_hi = 0x50c335 + *(y + 0x11);
    *(y + 0x12) = 0x0a;
    *(y + 0x13) = 0x7ffcb9;
    *(y + 0x11) = lim24(b_hi);
    *(y + 0x16) = 0;
    *(y + 0x17) = 0;
    *(y + 0x19) = 0;
    x0 = *(y + 0x06);
    x1 = *(y + 0x07);
    *(y + 0x1a) = x0;
    *(y + 0x1b) = x1;
    *(y + 0x14) = x0;
    *(y + 0x15) = x1;
    *(y + 0x1e) = *(y + 0x0b);
    *(y + 0x21) = *(y + 0x0d);
    *(y + 0x1f) = 0;
    *(y + 0x20) = 0;
    *(y + 0x0f) = 0x7ffcb9;
    *(y + 0x10) = 0x32;
    *(y + 0x0e) = 0x7fdbb4;
    v->active = 1u;
}

#ifndef TRXBD_DISABLE_QUIET_PATH
/* A retained native slot can outlive its audible note and keep machine 5 in
 * later frames. Do not spend a full sample kernel on an exactly quiescent
 * voice. This is NOT a level threshold or retirement: all 36 DSP state words
 * must remain identical, including PRNG/phase history for future retriggers.
 * The narrow predicate below makes the shaper's two comparisons impossible;
 * all its bounds and envelope products then remain unchanged. */
static void trxbd_quiet_phase(int32_t *y, uint32_t high_index, uint32_t shift) {
    int32_t inc_hi;
    uint32_t inc_lo;
    uint32_t low31;
    uint32_t high31;
    uint32_t low32;
    inc_hi = *(y + 3) >> shift;
    inc_lo = ((uint32_t)(*(y + 3)) & ((1u << shift) - 1u)) << (24u - shift);
    low31 = u24(*(y + (high_index + 1u))) + inc_lo * 31u;
    high31 = (uint32_t)(*(y + high_index) + inc_hi * 31 + (int32_t)(low31 >> 24)) & 0x7fffu;
    low32 = (low31 & 0xffffffu) + inc_lo;
    /* Mask BEFORE the last step, exactly as the original per-sample loop.
     * Masking the final high word would erase an observable boundary carry. */
    *(y + high_index) = sx24((int32_t)high31 + inc_hi + (int32_t)(low32 >> 24));
    *(y + (high_index + 1u)) = sx24((int32_t)(low32 & 0xffffffu));
}

static uint32_t trxbd_is_quiet(const int32_t *y) {
    if (*(y + 0x12) != 0 || *(y + 0x11) != 0 || *(y + 0x13) != 0 ||
        *(y + 0x1e) != 0 || *(y + 0x21) != 0 ||
        *(y + 0x1a) != 0x7fffff || *(y + 0x1b) != -0x800000 ||
        *(y + 0x08) < 0 || *(y + 0x09) > 0 ||
        *(y + 0x10) < 0) {
        return 0u;
    }
    return 1u;
}

static uint32_t trxbd_try_quiet_block(TrxBdVoice *v) {
    int32_t *y;
    uint32_t a;
    uint32_t b;
    uint32_t sum;
    uint32_t i;
    y = v->y;
    if (trxbd_is_quiet(y) == 0u) {
        return 0u;
    }
    a = u24(*(y + 0x1c));
    b = u24(*(y + 0x1d));
    for (i = 0u; i < 11u; i++) {
        sum = (a + b) & 0xffffffu;
        a = b;
        b = sum;
    }
    *(y + 0x1c) = sx24((int32_t)a);
    *(y + 0x1d) = sx24((int32_t)b);
    trxbd_quiet_phase(y, 0x16u, 6u);
    trxbd_quiet_phase(y, 0x1fu, 7u);
    for (i = 0u; i < 32u; i++) {
        *(v->blk44 + i) = 0;
    }
    *(y + 0x18) = 0;
    return 1u;
}
#endif

#ifndef TRXBD_HOST_TEST
extern void trxbd_transient_fade(int32_t previous, int32_t *out);
extern void trxbd_transient(int32_t *out, const int32_t *ta, const int32_t *tb, const int32_t *y, const int32_t *noise, uint32_t silent);
extern void trxbd_noise_active(int32_t *y, int32_t *noise);
extern void trxbd_phase_prepare(int32_t *y, int32_t *increments);
extern void trxbd_osc_shape(int32_t *y, const int32_t *sine, int32_t *out, const int32_t *increments);
extern int32_t trxbd_amp_clip(TrxBdVoice *v, const int32_t *shaped, const int32_t *noise, uint32_t noise_silent);
#endif

void trxbd_render_block_44k1(TrxBdVoice *v, const int32_t *sine) {
    int32_t *y;
    int32_t *out;
    int32_t noise[32];
    int32_t x_buf[32];
    uint32_t b_hi;
    uint32_t x1_u;
    uint32_t idx;
    uint32_t i;
    uint32_t noise_silent;
    int32_t b1_s;
    int32_t x0;
    int32_t x1;
    int32_t a_cnt;
    int32_t ah;
    uint32_t al;
    int32_t bh;
    uint32_t bl;
    int32_t inc_h;
    uint32_t inc_l;
    int32_t osc1_hi_inc;
    uint32_t osc1_lo_inc;
    int32_t osc1_hi;
    uint32_t osc1_lo;
    int32_t y0;
#ifndef TRXBD_HOST_TEST
    int32_t oscillator_increments[4];
#endif

    y = v->y;
    out = v->blk44;
#ifndef TRXBD_DISABLE_QUIET_PATH
    if (trxbd_try_quiet_block(v) != 0u) {
        return;
    }
#endif

    /* Parenthesize integer index sums before adding to pointers: the pinned
     * selcc loses the element scale on a second chained pointer addition. */
    /* The original MD decay coefficients in trxbd_t_decay are in
     * [0x5f29c2, 0x7fffa9] (all >=0 and <= signed24 maximum).
     * For ANY signed24 signal a and NONNEGATIVE Q23 coefficient b<=0x7fffff:
     *     -0x800000 <= floor(a*b / 0x800000) <= 0x7fffff
     * Therefore mpy_ss_hi(a,b) ALREADY fits signed24, including the most
     * negative two's-complement input, and lim24 is a mathematical no-op.
     * This holds for the noise seed/envelope, transient envelope, pitch-ramp,
     * harmonic decay and 32 per-sample amplitude-decay updates below.
     *
     * The signed25 sample sum (xv+nv) is NOT bounded this way: the
     * corresponding lim24 must stay. Likewise all clipping/shaping
     * saturations retain their original semantics.
     *
     * Source invariants: coefficients come from the original bounded
     * trxbd_t_decay table or positive trigger constants; original 24-bit
     * state writes remain sign-canonical. Exhaustive boundary/property
     * tests and host/SHARC state+PCM comparisons gate this experiment. */
     
    /* 1. Noise LFSR (P:$100408..$10041A) */
    b_hi = u24(*(y + 0x1c));
    x1_u = u24(*(y + 0x1d));
    idx = 0u;
    x0 = *(y + 0x1e);
    noise_silent = (x0 == 0) ? 1u : 0u;
    if (noise_silent != 0u) {
        /* Original noise recurrence is a'=b, b'=(a+b) mod 2^24.
         * After 10 iterations, the original code saves b10 in y[0x1c]
         * and b11=a10+b10 in y[0x1d]. Exact Fibonacci coefficients:
         * b10=55*a0+89*b0, b11=89*a0+144*b0. Every intermediate
         * multiply/add fits uint32: (89+144)*0xffffff < 2^32.
         * The zero envelope ensures every noise sample is exactly
         * zero, but future PRNG state must remain bit identical. */
        uint32_t a0;
        uint32_t b0;
        a0 = b_hi;
        b0 = x1_u;
        *(y + 0x1c) = sx24((int32_t)((55u * a0 + 89u * b0) & 0xffffffu));
        *(y + 0x1d) = sx24((int32_t)((89u * a0 + 144u * b0) & 0xffffffu));
    } else {
#ifndef TRXBD_HOST_TEST
        trxbd_noise_active(y, noise);
#else
        for (i = 0u; i < 10u; i++) {
            uint32_t x0_u;
            b_hi = (b_hi + x1_u) & 0xffffffu;
            x0_u = x1_u;
            b1_s = mpy_ss_hi(sx24((int32_t)b_hi), x0);
            *(noise + idx) = b1_s;
            *(noise + (idx + 1u)) = b1_s;
            *(noise + (idx + 2u)) = b1_s;
            idx = idx + 3u;
            x1_u = b_hi;
            b_hi = x0_u;
        }
        b_hi = (b_hi + x1_u) & 0xffffffu;
        *(y + 0x1c) = sx24((int32_t)x1_u);
        b1_s = sx24((int32_t)b_hi);
        *(y + 0x1d) = b1_s;
        b1_s = mpy_ss_hi(b1_s, x0);
        *(noise + 30u) = b1_s;
        *(noise + 31u) = b1_s;
#endif
    }

    /* 2. Noise envelope (P:$10041B..$100427).
     * Zero times any coefficient is exactly zero, including all state
     * updates. Do not read the intentionally uninitialized noise[] in the
     * other two processing paths when noise_silent is true. */
#ifdef TRXBD_HOST_TEST
    x1 = *(y + 0x0c);
    *(y + 0x1e) = (noise_silent != 0u) ? 0 : mpy_ss_hi(x1, x0);
#endif

    /* 3. Transient attack countdown (P:$10042F..$100460) */
    a_cnt = *(y + 0x12);
    if (a_cnt != 0) {
        a_cnt = a_cnt - 1;
        *(y + 0x12) = lim24(a_cnt);
        if (a_cnt == 9) {
#ifndef TRXBD_HOST_TEST
            trxbd_transient_fade(*(y + 0x18), out);
#else
            x0 = 0x733333;
            x1 = *(y + 0x18);
            for (i = 0u; i < 32u; i++) {
                int32_t s;
                s = mpy_ss_hi(x1, x0);
                *(out + i) = s;
                x1 = s;
            }
#endif
            return;
        } else {
            uint32_t off;
            off = (uint32_t)(*(y + 0x19));
            *(y + 0x19) = lim24((int32_t)(off + 32u));
#ifndef TRXBD_HOST_TEST
            trxbd_transient(out, trxbd_table_a + off, trxbd_table_b + off, y, noise, noise_silent);
#else
            x0 = *(y + 0x0f);
            x1 = *(y + 0x02);
            for (i = 0u; i < 32u; i++) {
                uint32_t ti;
                int32_t tb;
                int32_t ta;
                int32_t nv;
                ti = off + i;
                tb = *(trxbd_table_b + ti);
                ta = *(trxbd_table_a + ti);
                nv = (noise_silent != 0u) ? 0 : *(noise + i);
                mpy_ss32(tb, x1, &ah, &al);
                ah = ah * 4 + (int32_t)(al >> 22);
                al = (al << 2) & 0xffffffu;
                mpy_ss32(ta, x0, &bh, &bl);
                *(out + i) = lim24(ah + bh + (int32_t)((al + bl) >> 24) + nv);
            }
#endif
            return;
        }
    }

#ifndef TRXBD_HOST_TEST
    /* Both original increments use the same pre-decay ramp product. */
    trxbd_phase_prepare(y, oscillator_increments);
#else
    /* 4. Oscillator 1 (P:$100461..$10047A) */
    asr56(*(y + 0x03), 0u, 6u, &ah, &al);
    mpy_ss32(*(y + 0x13), *(y + 0x04), &bh, &bl);
    asr56(bh, bl, 1u, &bh, &bl);
    add56(ah, al, bh, bl, &inc_h, &inc_l);
    inc_h = sx24(inc_h);
    /* Retain the precise oscillator-1 increment and initial phase. Its
     * original sine samples are calculated inside the sample-local
     * harmonic/waveshaper loop instead of writing then rereading a
     * 32-word x_buf array. Oscillator-2 is completely independent of
     * oscillator-1 phase; no original shared y[] field is modified
     * between these two independent loops. */
    osc1_hi_inc = inc_h;
    osc1_lo_inc = inc_l;

    /* 5. Oscillator 2 & ramp decay (P:$10047B..$10049B).
     * Like oscillator 1, active oscillator 2 streams into the shaper below.
     * Preserve its independent high/low phase and exact increment here. */
    asr56(*(y + 0x03), 0u, 7u, &ah, &al);
    mpy_ss32(*(y + 0x13), *(y + 0x04), &bh, &bl);
    asr56(bh, bl, 1u, &bh, &bl);
    add56(ah, al, bh, bl, &inc_h, &inc_l);
    inc_h = sx24(inc_h);
    *(y + 0x13) = mpy_ss_hi(*(y + 0x13), *(y + 0x05));
#endif
#ifndef TRXBD_HOST_TEST
    /* Original oscillator/harmonic/shaper stages, with an exact native
     * closed-form phase update when harmonics are zero. */
    trxbd_osc_shape(y, sine, x_buf, oscillator_increments);
#else
    osc1_hi = *(y + 0x16);
    osc1_lo = u24(*(y + 0x17));
    bh = *(y + 0x1f);
    bl = u24(*(y + 0x20));
    if (*(y + 0x21) == 0) {
        /* Zero harmonics: the second oscillator produces no observable
         * samples, but its COMPLETE 48-bit phase state must still advance.
         * Collapse 32 identical phase steps into one exact closed-form
         * update. A 32-bit unsigned sum suffices since both 24-bit addends
         * are bounded: 0xFFFFFF + 31*0xFFFFFF < 2^30.
         *
         * The original loop masks the high phase to 15 bits BEFORE each
         * step. Thus the state after 31 steps is:
         * ((initial_high&0x7fff) + 31*inc_h + floor(sum31/2^24))&0x7fff.
         * Its 32nd step must occur AFTER that mask, not before it.
         * Do not collapse into one high-word sum without the last mask:
         * that would alter DSP state at some wrap boundaries. */
        uint32_t sum31;
        uint32_t sum32;
        int32_t high31;
        sum31 = bl + inc_l * 31u;
        high31 = ((bh & 0x7fff) + 31 * inc_h + (int32_t)(sum31 >> 24)) & 0x7fff;
        sum32 = (sum31 & 0xffffffu) + inc_l;
        bh = high31 + inc_h + (int32_t)(sum32 >> 24);
        bl = sum32 & 0xffffffu;
    }

    /* 6+7. Harmonic mix and asymmetric shaper (P:$10049C..$1004D3).
     * These stages are strictly sample-local apart from the shaper's
     * sequential bounds. Fusing avoids an extra 32-word write/read pass
     * through x_buf while preserving the original 24-bit arithmetic order. */
    {
        int32_t harm_decay;
        int32_t harm_level;
        int32_t shape_step;
        int32_t shape_high;
        int32_t shape_low;
        int32_t shape_start_high;
        int32_t shape_start_low;
        int32_t shape_previous_high;
        int32_t shape_previous_low;
        harm_decay = *(y + 0x0e);
        harm_level = *(y + 0x21);
        shape_step = *(y + 0x10);
        shape_high = *(y + 0x1a);
        shape_low = *(y + 0x1b);
        shape_start_high = shape_high;
        shape_start_low = shape_low;
        shape_previous_high = *(y + 0x14);
        shape_previous_low = *(y + 0x15);
        *(y + 0x1a) = lim24(shape_step + shape_high);
        *(y + 0x1b) = lim24(shape_low - shape_step);

        for (i = 0u; i < 32u; i++) {
            int32_t xv;
            int32_t yv;
            int32_t doubled;
            uint32_t osc1_idx;
            osc1_idx = (uint32_t)osc1_hi & 0x7fffu;
            osc1_hi = (osc1_hi & ~0xffffff) | (int32_t)osc1_idx;
            /* Preserve the ORIGINAL phase-before-sample semantics:
             * index at current high word, mask, advance the full 48-bit
             * phase with low-word carry, then read the indexed sine. */
            osc1_lo = osc1_lo + osc1_lo_inc;
            osc1_hi = osc1_hi + osc1_hi_inc + (int32_t)(osc1_lo >> 24);
            osc1_lo = osc1_lo & 0xffffffu;
            xv = *(sine + osc1_idx) >> 2;
            /* The original oscillator-2 sample contributes only when
             * harmonics are nonzero. Stream it without a temporary buffer;
             * the exact-zero path already advanced its full phase above. */
            /* The first oscillator is already within signed24. If the
             * original harmonic accumulator is zero, the 24-bit multiply
             * and saturation are provably the identity. The second
             * oscillator and its state STILL advance above. */
            if (harm_level != 0) {
                uint32_t r2;
                /* Preserve current index, then advance with low-word carry,
                 * and clamp the ORIGINAL doubled sine before multiplication. */
                r2 = (uint32_t)bh & 0x7fffu;
                bh = (bh & ~0xffffff) | (int32_t)r2;
                bl = bl + inc_l;
                bh = bh + inc_h + (int32_t)(bl >> 24);
                bl = bl & 0xffffffu;
                yv = lim24(*(sine + r2) * 2);
                /* Both operands are canonical signed24. Oscillator 1
                 * starts at sine>>2, in [-0x200000,0x1fffff].
                 * The second product, after >>4, lies in
                 * [-0x80000,0x80000]. Their sum is therefore always in
                 * [-0x280000,0x27ffff], strictly within signed24.
                 * The original lim24 was a provable no-op here; retain
                 * the exact multiply and signed arithmetic-shift order. */
                xv = xv + (mpy_ss_hi(yv, harm_level) >> 4);
            }
            doubled = -xv;
            if (doubled > shape_high * 2) {
                doubled = shape_previous_high - shape_step;
                shape_previous_low = shape_start_low;
                shape_previous_high = lim24(doubled);
                doubled = doubled * 2;
                /* The fold can exceed signed24. Preserve the clamp. */
                *(x_buf + i) = lim24(doubled);
            } else if (doubled < shape_low * 2) {
                doubled = shape_previous_low + shape_step;
                shape_previous_high = shape_start_high;
                shape_previous_low = lim24(doubled);
                doubled = doubled * 2;
                /* The fold can exceed signed24. Preserve the clamp. */
                *(x_buf + i) = lim24(doubled);
            } else {
                /* No fold: xv = sine>>2 + (signed24_product>>4).
                 * Its extremes lie in [-0x280000, 0x27ffff]; negating
                 * cannot leave signed24. The original lim24(doubled)
                 * was a provable no-op on this straight-through path.
                 * Keep the original fold-state transitions untouched. */
                *(x_buf + i) = doubled;
            }
        }
        *(y + 0x1f) = sx24(bh);
        *(y + 0x20) = sx24((int32_t)bl);
        *(y + 0x21) = mpy_ss_hi(harm_level, harm_decay);
        *(y + 0x14) = shape_previous_high;
        *(y + 0x15) = shape_previous_low;
        /* Original oscillator-1 phase end-state, committed after all
         * 32 waveform samples. No subsequent original stage reads these
         * two words until the next renderer invocation. */
        *(y + 0x16) = sx24(osc1_hi);
        *(y + 0x17) = sx24((int32_t)osc1_lo);
    }

#endif

    /* 8-10. Amplitude/noise, clipper and output (P:$1004D4..$10050B).
     * The clipper and output depend only on the just-produced amplitude
     * sample. Write the final sample directly instead of allocating and
     * traversing an intermediate x3_buf[32]. All intermediate lim24/
     * multiply/carry operations remain in their original sample order. */
    {
        int32_t envelope_decay;
        int32_t envelope;
        int32_t clip_high;
        int32_t clip_low;
        int32_t clip_curve;
        envelope_decay = *(y + 0x01);
        envelope = *(y + 0x11);
        clip_high = *(y + 0x08);
        clip_low = *(y + 0x09);
        clip_curve = *(y + 0x0a);
        if (envelope == 0 && clip_high > 0 && clip_low < 0) {
            /* Exact zero-envelope fast path. y[0x11] remains zero under
             * multiplication by any coefficient. With zero strictly
             * between the original clip thresholds, its clipper and
             * output samples are also exactly zero. All noise, oscillator
             * phases and waveshaper state were updated in the preceding
             * stages, as required for future note-ons/parameter changes.
             * Unlike the complete quiet-state shortcut, this remains
             * correct when other internal envelopes are still active. */
            for (i = 0u; i < 32u; i++) {
                *(out + i) = 0;
            }
        } else {
#ifndef TRXBD_HOST_TEST
        /* Original stages 8-10, with identical sample/truncation order.
         * The native helper holds coefficients and envelope in registers,
         * preserves the pointer C ABI, and returns the final envelope. */
        envelope = trxbd_amp_clip(v, x_buf, noise, noise_silent);
#else
        for (i = 0u; i < 32u; i++) {
            int32_t xv;
            int32_t nv;
            int32_t clipped;
            xv = *(x_buf + i);
            nv = (noise_silent != 0u) ? 0 : *(noise + i);
            /* xv+nv is signed 25-bit: never round the add to 24-bit. */
            xv = lim24(mpy_ss_hi(envelope, xv + nv));
            envelope = mpy_ss_hi(envelope, envelope_decay);
            ah = xv;
            al = 0u;
            if (xv >= clip_high) {
                int32_t excess;
                excess = lim24(xv - clip_high);
                mpy_ss32(excess, clip_high, &ah, &al);
                ah = ah + clip_high;
            } else if (xv <= clip_low) {
                int32_t excess;
                excess = lim24(xv - clip_low);
                mpy_ss32(excess, clip_high, &ah, &al);
                ah = ah + clip_low;
            }
            if (clip_curve == 0) {
                /* mpy_ss32(0,y0) returns hi=lo=0. The first product's
                 * low 24 bits alone cannot carry into the high word.
                 * Thus the original summation is exactly lim24(ah). */
                clipped = lim24(ah);
            } else {
                y0 = lim24(ah);
                mpy_ss32(clip_curve, y0, &bh, &bl);
                clipped = lim24(ah + bh + (int32_t)((al + bl) >> 24));
            }
            /* The sum and double fit signed32, including both limits. */
            *(out + i) = lim24((clipped + xv) * 2);
        }
#endif
        }
        *(y + 0x11) = envelope;
    }
    *(y + 0x18) = *(out + 31u);
}

#ifndef TRXBD_DISABLE_QUIET_PATH
static uint32_t trxbd_try_quiet_48k(TrxBdVoice *v, float *out_f32) {
    uint32_t i;
    uint32_t pi;
    uint32_t pf;
    uint32_t last;
    pi = v->pos_int;
    pf = v->pos_frac;
    if (v->prev_s24 != 0 || pi > 32u || pf >= 160u || trxbd_is_quiet(v->y) == 0u) {
        return 0u;
    }
    for (i = 0u; i < 32u; i++) {
        if (*(v->blk44 + i) != 0) {
            return 0u;
        }
    }
    /* Original rendering occurs BEFORE each sample, so test the position
     * before sample 31. 31*147 = 28*160+77; 32*147 = 29*160+64.
     * Keep a crossing after the last sample pending until the next callback. */
    last = pi + 28u;
    if (pf >= 83u) {
        last = last + 1u;
    }
    pi = pi + 29u;
    pf = pf + 64u;
    if (pf >= 160u) {
        pf = pf - 160u;
        pi = pi + 1u;
    }
    if (last >= 32u) {
        trxbd_try_quiet_block(v);
        pi = pi - 32u;
    }
    v->pos_int = pi;
    v->pos_frac = pf;
    for (i = 0u; i < 32u; i++) {
        *(out_f32 + i) = 0.0f;
    }
    return 1u;
}
#endif

#ifndef TRXBD_HOST_TEST
extern uint32_t trxbd_resample_run(TrxBdVoice *v, float *out, uint32_t count);
#endif

#define MD_VOICE_TYPE TrxBdVoice
#define MD_RENDER48 trxbd_render_block_48k_float
#define MD_RENDER44 trxbd_render_block_44k1
#define MD_RESAMPLE_RUN trxbd_resample_run
#define MD_TRY_QUIET48 trxbd_try_quiet_48k
#ifdef TRXBD_DISABLE_QUIET_PATH
#define MD_DISABLE_QUIET_PATH
#endif
#include "../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_TRY_QUIET48
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE

#if !defined(TRXBD_HOST_TEST) && !defined(MD_GENERIC_ENGINE)
#define MD_READ_FRAME read_frame_u16
#define MD_INIT trxbd_init
#define MD_PROCESS trxbd_process
#define MD_STATE_TYPE TrxBdState
#define MD_VOICE_TYPE TrxBdVoice
#define MD_STATE_ADDR TRXBD_STATE_ADDR
#define MD_SINE_ADDR TRXBD_SINE_ADDR
#define MD_MAGIC TRXBD_MAGIC
#define MD_MACHINE_TYPE TRXBD_MACHINE_TYPE
#define MD_BUILD_SINE trxbd_build_sine_table
#define MD_VOICE_INIT trxbd_voice_init
#define MD_VOICE_TRIGGER trxbd_voice_trigger
#define MD_COEFFICIENTS trxbd_compute_coeffs
#define MD_RENDER44 trxbd_render_block_44k1
#define MD_RENDER48 trxbd_render_block_48k_float
#define MD_PARAM_OFFSETS trxbd_param_frame_offsets
#include "../md_runtime/native_engine.inc"
#undef MD_READ_FRAME
#undef MD_INIT
#undef MD_PROCESS
#undef MD_STATE_TYPE
#undef MD_VOICE_TYPE
#undef MD_STATE_ADDR
#undef MD_SINE_ADDR
#undef MD_MAGIC
#undef MD_MACHINE_TYPE
#undef MD_BUILD_SINE
#undef MD_VOICE_INIT
#undef MD_VOICE_TRIGGER
#undef MD_COEFFICIENTS
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_PARAM_OFFSETS
#endif

#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxBdVoice
#define MD_SLOT_INIT trxbd_slot_init
#define MD_SLOT_PROCESS trxbd_slot_process
#define MD_VOICE_INIT trxbd_voice_init
#define MD_COEFFICIENTS trxbd_compute_coeffs
#define MD_VOICE_TRIGGER trxbd_voice_trigger
#define MD_RENDER44 trxbd_render_block_44k1
#define MD_RENDER48 trxbd_render_block_48k_float
#include "../machines/engine_slot.inc"
#endif
