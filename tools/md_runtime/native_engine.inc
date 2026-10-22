/* Native pre-filter voice adapter, instantiated once per engine.
 * Required bindings are documented in CONTRACT.md. No allocator or release
 * policy lives here: DNII owns its slots and AMP lifecycle.
 * Multiple instances use distinct state regions, identifiers and entry names;
 * aggregate FXINIT/process entries call each instance once. */
#ifndef MD_OUTPUT_BUFFER
#define MD_OUTPUT_BUFFER(t) ((float *)(*(DN2_VOICE_BUF_PTRS + (t))))
#define MD_DEFAULT_OUTPUT_BUFFER
#endif
#ifndef MD_PARAM_COUNT
#define MD_PARAM_COUNT 8u
#define MD_DEFAULT_PARAM_COUNT
#endif
static uint32_t MD_READ_FRAME(uint32_t byte_off) {
    const uint32_t *f32;
    uint32_t w;
    f32 = (const uint32_t *)DN2_FRAME_COPY_ADDR;
    w = *(f32 + (byte_off >> 2));
    if ((byte_off & 2u) != 0u) {
        return (w >> 16) & 0xffffu;
    }
    return w & 0xffffu;
}

void MD_INIT(void) {
    MD_STATE_TYPE *st;
    int32_t *sine;
    uint32_t t;
    st = MD_STATE_ADDR;
    sine = MD_SINE_ADDR;
    MD_BUILD_SINE(sine);
    for (t = 0u; t < 16u; t++) {
        MD_VOICE_INIT(st->voices + t);
    }
    st->blocks_rendered = 0u;
    st->magic = MD_MAGIC;
}

void MD_PROCESS(void) {
    MD_STATE_TYPE *st;
    const int32_t *sine;
    const uint32_t *f32;
    uint32_t trig_mask;
    uint32_t t;
    st = MD_STATE_ADDR;
    sine = MD_SINE_ADDR;
    if (st->magic != MD_MAGIC) {
        /* Initialization generates 32768 sine words. It belongs in FXINIT,
         * never in a real-time callback. Latch a bad/uninitialized state as
         * silent custom voices until the normal initialization path runs. */
        for (t = 0u; t < 16u; t++) {
            uint32_t i;
            float *out;
            if (MD_READ_FRAME(148u + 2u * t) == MD_MACHINE_TYPE) {
                out = MD_OUTPUT_BUFFER(t);
                for (i = 0u; i < 32u; i++) {
                    *(out + i) = 0.0f;
                }
            }
        }
        return;
    }
    f32 = (const uint32_t *)DN2_FRAME_COPY_ADDR;
    trig_mask = (*(f32 + 8u)) >> 16;

    for (t = 0u; t < 16u; t++) {
        uint32_t mtype;
        float *vbuf;
        MD_VOICE_TYPE *v;
        uint32_t t_off;
        uint32_t p14[MD_PARAM_COUNT];
        uint32_t changed;
        uint32_t k;
        uint32_t trig;

        /* The stock record's machine field can describe the previous block
         * during machine replacement. Using it loses the first trigger on
         * entry and can retain the old type after buffer reassignment.
         * The current frame owns dispatch; a stale record must never
         * authorize a buffer write or suppress the new machine's trigger. */
        mtype = MD_READ_FRAME(148u + 2u * t);
        v = st->voices + t;
        if (mtype != MD_MACHINE_TYPE) {
            /* A voice slot can be reassigned by a sound/pattern/project load.
             * Discard synthesis and resampler history at that boundary. Never
             * clear the stock voice buffer: it already belongs to its renderer.
             * last_p also catches a selected but never triggered custom voice. */
            if (v->active != 0u || *(v->last_p + 0) != 0xffffffffu) {
                MD_VOICE_INIT(v);
            }
            continue;
        }
        vbuf = MD_OUTPUT_BUFFER(t);
        t_off = t * 146u;
        changed = 0u;
        for (k = 0u; k < MD_PARAM_COUNT; k++) {
            uint32_t f_off;
            uint32_t raw16;
            uint32_t val14;
            f_off = *(MD_PARAM_OFFSETS + k) + t_off;
            raw16 = MD_READ_FRAME(f_off);
            if (raw16 > 0x7f00u) {
                raw16 = 0x7f00u;
            }
            val14 = raw16 >> 1;
            *(p14 + k) = val14;
            if (*(v->last_p + k) != val14) {
                changed = 1u;
            }
        }
        trig = (trig_mask >> t) & 1u;
        if (changed != 0u || trig != 0u) {
            for (k = 0u; k < MD_PARAM_COUNT; k++) {
                *(v->last_p + k) = *(p14 + k);
            }
            MD_COEFFICIENTS(p14, v->y);
        }
        if (trig != 0u) {
            MD_VOICE_TRIGGER(v);
            MD_RENDER44(v, sine);
            v->prev_s24 = *(v->blk44 + 0);
            v->pos_int = 0u;
            v->pos_frac = 0u;
        }
        MD_RENDER48(v, sine, vbuf);
    }
    st->blocks_rendered = st->blocks_rendered + 1u;
}

#ifdef MD_DEFAULT_OUTPUT_BUFFER
#undef MD_OUTPUT_BUFFER
#undef MD_DEFAULT_OUTPUT_BUFFER
#endif
#ifdef MD_DEFAULT_PARAM_COUNT
#undef MD_PARAM_COUNT
#undef MD_DEFAULT_PARAM_COUNT
#endif
