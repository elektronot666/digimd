/* Authentic OS 1.63 TRX-SD. Original P:$10074c..$100880, CPU $2013a6.
 * Each arithmetic truncation and persistent Y write follows the original code.
 * Source rate 44100 Hz. Shared arithmetic contract: tools/md_runtime/CONTRACT.md.
 */
#include <stdint.h>
#include "trxsd_tables.h"
#ifdef MD_DSP_HOST_TEST
#include "../trxbd_sharc/trxbd_tables.h"
#define md_decay_table trxbd_t_decay
#else
extern const uint32_t md_decay_table[438];
#endif
#include "../md_runtime/voice.h"
#include "../md_runtime/arithmetic.h"

typedef struct {
    int32_t y[30];
    MD_STREAM_FIELDS
    uint32_t last_p[8];
    uint32_t active;
} TrxSdVoice;

void trxsd_compute_coeffs(const uint32_t *p, int32_t *y) {
    uint32_t d;
    uint32_t pitch;
    pitch = (*(p + 0) >> 2) + 0x1000u;
    *(y + 7) = (int32_t)pitch;
    *(y + 11) = (int32_t)((*(p + 0) >> 5) + 0x400u);
    *(y + 1) = (int32_t)*(md_decay_table + (220u + (*(p + 1) >> 7)));
    *(y + 2) = (int32_t)*(md_decay_table + (232u + (*(p + 1) >> 8)));
    d = *(p + 4);
    *(y + 5) = sx24((int32_t)(d << 9));
    *(y + 6) = 0;
    if (d > 0x1fffu) {
        d = d - 0x2000u;
        *(y + 6) = (int32_t)(((d * d) >> 4) & 0xfffffc00u);
    }
    d = *(p + 5) << 8;
    *(y + 3) = (int32_t)(0x7fffffu - d);
    *(y + 4) = (int32_t)d;
    *(y + 8) = (int32_t)(((pitch >> 6) * ((*(p + 6) >> 9) + 0xb9u)) >> 1);
    d = *(p + 2);
    *(y + 10) = (int32_t)((d * d) >> 18);
    *(y + 12) = (int32_t)*(md_decay_table + (15u + ((*(p + 3) * 5u) >> 10)));
    *(y + 9) = (int32_t)*(trxsd_t_control_clip + ((*(p + 7) * 0x62b3u) >> 21));
}

void trxsd_voice_init(TrxSdVoice *v) {
    uint32_t i;
    for (i = 0u; i < 30u; i++) { *(v->y + i) = 0; }
    for (i = 0u; i < 32u; i++) { *(v->blk44 + i) = 0; }
    for (i = 0u; i < 8u; i++) { *(v->last_p + i) = 0xffffffffu; }
    *(v->y + 23) = 0x29a;
    *(v->y + 24) = 1;
    v->prev_s24 = 0;
    v->pos_int = 32u;
    v->pos_frac = 0u;
    v->active = 0u;
}

void trxsd_voice_trigger(TrxSdVoice *v) {
    int32_t a;
    uint32_t i;
    *(v->y + 13) = 0x140;
    *(v->y + 29) = 0x7eb852;
    *(v->y + 14) = lim24(*(v->y + 14) + 0x50c335);
    a = *(v->y + 22);
    if (a < 0) { a = -a; }
    *(v->y + 15) = lim24(0x4ccccd + a * 16);
    for (i = 16u; i <= 21u; i++) { *(v->y + i) = 0; }
    v->active = 1u;
}

/* Original 3-product MAC followed by ASL and one data-bus limiter.
 * Retain the full low24 carry through all three products and the shift. */
#ifdef MD_DSP_HOST_TEST
static int32_t trxsd_filter(int32_t older, int32_t recent, int32_t input,
                            int32_t fb2, int32_t fb1, int32_t feed) {
    int32_t h;
    uint32_t l;
    int32_t p;
    uint32_t q;
    mpy_ss32(fb2, older, &h, &l);
    mpy_ss32(fb1, recent, &p, &q);
    add56(h, l, p, q, &h, &l);
    mpy_ss32(feed, input, &p, &q);
    add56(h, l, p, q, &h, &l);
    add56(h, l, h, l, &h, &l);
    return lim24(h);
}

#else
extern void trxsd_tonal(TrxSdVoice *v, int32_t *tone, const int32_t *noise, int32_t level);
extern int32_t trxsd_filter(int32_t older, int32_t recent, int32_t input,
                            int32_t fb2, int32_t fb1, int32_t feed);
#endif

void trxsd_render_block_44k1(TrxSdVoice *v, const int32_t *sine) {
    int32_t tone[32];
    int32_t noise[32];
    int32_t *y;
    uint32_t i;
    uint32_t osc;
    int32_t ih;
    uint32_t il;
    int32_t h;
    uint32_t l;
    uint32_t sum;
    uint32_t phase;
    int32_t level;
    int32_t x;
    int32_t z;
    int32_t p;
    uint32_t q;
    int32_t fb1;
    int32_t fb2;
    int32_t feed;
    int32_t c;
    int32_t d;
    int32_t a;
    int32_t b;
    uint32_t cursor;
    y = v->y;
    for (osc = 0u; osc < 2u; osc++) {
        h = *(y + (7u + osc));
        l = 0u;
        asr56(h, l, 1u, &h, &l);
        add56(h, l, *(y + 16), 0u, &h, &l);
        asr56(h, l, 4u, &ih, &il);
        h = *(y + (18u + 2u * osc));
        l = u24(*(y + (19u + 2u * osc)));
        for (i = 0u; i < 32u; i++) {
            phase = (uint32_t)h;
            x = mpy_ss_hi(*(sine + phase), *(y + (3u + osc)));
            if (osc == 0u) { *(tone + i) = x; } else { *(noise + i) = x; }
            sum = l + il;
            h = (h + ih + (int32_t)(sum >> 24)) & 0x7fff;
            l = sum & 0xffffffu;
        }
        *(y + (18u + 2u * osc)) = h;
        *(y + (19u + 2u * osc)) = sx24((int32_t)l);
    }
    if (*(y + 17) <= *(y + 13)) {
        *(y + 16) = lim24(*(y + 10) * 2 + *(y + 16));
    } else {
        *(y + 16) = lim24(mpy_ss_hi(*(y + 12), *(y + 16)));
    }
    level = *(y + 29);
    *(y + 29) = lim24(mpy_ss_hi(*(y + 9), level));
#ifndef MD_DSP_HOST_TEST
    trxsd_tonal(v, tone, noise, level);
#else
    for (i = 0u; i < 32u; i++) {
        a = *(tone + i);
        b = *(noise + i);
        mpy_ss32(a, level, &h, &l);
        mpy_ss32(level, b, &p, &q);
        add56(h, l, p, q, &h, &l);
        h = h * 64 + (int32_t)(l >> 18);
        *(tone + i) = lim24(a + b + h);
    }
    level = *(y + 14);
    for (i = 0u; i < 32u; i++) {
        *(tone + i) = lim24(mpy_ss_hi(level, *(tone + i)));
        level = lim24(mpy_ss_hi(level, *(y + 1)));
    }
    *(y + 14) = level;
#endif
    a = *(y + 23);
    b = *(y + 24);
    for (i = 0u; i < 32u; i++) {
        z = sx24(a + b);
        *(noise + i) = z;
        a = b;
        b = z;
    }
    *(y + 23) = a;
    *(y + 24) = b;
    cursor = (uint32_t)*(y + 11);
    c = sx24((int32_t)*(trxsd_t_filter_cos + (cursor - 1024u)));
    d = sx24((int32_t)*(trxsd_t_filter_radius + (cursor - 1024u)));
    mpy_ss32(d, d, &h, &l);
    fb2 = lim24(-h - (int32_t)(l != 0u));
    mpy_ss32(d, c, &h, &l);
    add56(h, l, h, l, &h, &l);
    fb1 = lim24(h);
    feed = lim24(0x400000 - h - fb2 - (int32_t)(l != 0u));
    for (i = 0u; i < 32u; i++) {
        x = trxsd_filter(*(y + 25), *(y + 26), *(noise + i), fb2, fb1, feed);
        *(y + 25) = *(y + 26);
        *(y + 26) = x;
        z = trxsd_filter(*(y + 27), *(y + 28), x, fb2, fb1, feed);
        *(y + 27) = *(y + 28);
        *(y + 28) = z;
        *(noise + i) = z;
    }
    level = *(y + 15);
    for (i = 0u; i < 32u; i++) {
        *(noise + i) = lim24(mpy_ss_hi(level, *(noise + i)));
        level = lim24(mpy_ss_hi(level, *(y + 2)));
    }
    *(y + 15) = level;
    cursor = (uint32_t)*(y + 17);
    for (i = 0u; i < 32u; i++) {
        mpy_ss32(*(noise + i), *(y + 6), &h, &l);
        add56(h, l, *(tone + i), 0u, &h, &l);
        x = lim24(h);
        if (cursor <= 0xfe0u) {
            z = sx24((int32_t)*(trxsd_t_transient + (cursor + i)));
            mpy_ss32(z, *(y + 5), &h, &l);
            add56(h, l, x, 0u, &h, &l);
            x = lim24(h);
        }
        *(v->blk44 + i) = x;
    }
    *(y + 17) = lim24((int32_t)cursor + 32);
    *(y + 22) = *(v->blk44 + 31);
}

#ifndef MD_DSP_HOST_TEST
extern uint32_t trxsd_resample_run(TrxSdVoice *v, float *out, uint32_t count);
#endif
#define MD_VOICE_TYPE TrxSdVoice
#define MD_RENDER48 trxsd_render_block_48k_float
#define MD_RENDER44 trxsd_render_block_44k1
#define MD_RESAMPLE_RUN trxsd_resample_run
#define MD_DISABLE_QUIET_PATH
#include "../md_runtime/render48.inc"
#undef MD_DISABLE_QUIET_PATH
#undef MD_RESAMPLE_RUN
#undef MD_RENDER44
#undef MD_RENDER48
#undef MD_VOICE_TYPE

#if !defined(MD_DSP_HOST_TEST) && !defined(MD_GENERIC_ENGINE)
typedef struct {
    uint32_t magic;
    uint32_t blocks_rendered;
    TrxSdVoice voices[16];
} TrxSdState;
static const uint32_t trxsd_param_frame_offsets[8] = {
    250u, 252u, 244u, 248u, 254u, 256u, 262u, 260u
};
/* The aggregate initializer has already generated the one shared sine table. */
static void trxsd_sine_ready(int32_t *sine) { }
#define DN2_VOICE_BUF_PTRS ((uint32_t *)0x00254a60u)
#define DN2_FRAME_COPY_ADDR 0x0025c48cu
#define MD_READ_FRAME trxsd_read_frame
#define MD_INIT trxsd_init
#define MD_PROCESS trxsd_process
#define MD_STATE_TYPE TrxSdState
#define MD_VOICE_TYPE TrxSdVoice
#define MD_STATE_ADDR ((TrxSdState *)0x00304000u)
#define MD_SINE_ADDR ((int32_t *)0x80550000u)
#define MD_MAGIC 0x54525853u
#define MD_MACHINE_TYPE 6u
#define MD_BUILD_SINE trxsd_sine_ready
#define MD_VOICE_INIT trxsd_voice_init
#define MD_VOICE_TRIGGER trxsd_voice_trigger
#define MD_COEFFICIENTS trxsd_compute_coeffs
#define MD_RENDER44 trxsd_render_block_44k1
#define MD_RENDER48 trxsd_render_block_48k_float
#define MD_PARAM_OFFSETS trxsd_param_frame_offsets
#include "../md_runtime/native_engine.inc"
#endif

#ifdef MD_GENERIC_ENGINE
#define MD_VOICE_TYPE TrxSdVoice
#define MD_SLOT_INIT trxsd_slot_init
#define MD_SLOT_PROCESS trxsd_slot_process
#define MD_VOICE_INIT trxsd_voice_init
#define MD_COEFFICIENTS trxsd_compute_coeffs
#define MD_VOICE_TRIGGER trxsd_voice_trigger
#define MD_RENDER44 trxsd_render_block_44k1
#define MD_RENDER48 trxsd_render_block_48k_float
#include "../machines/engine_slot.inc"
#endif
