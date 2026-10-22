/* Compile-time binding avoids indirect calls in the sample/block path.
 * MD_VOICE_TYPE exposes MD_STREAM_FIELDS; MD_RENDER44 owns original DSP state.
 * A machine may opt out of MD_TRY_QUIET48; its proof stays engine-specific. */
void MD_RENDER48(MD_VOICE_TYPE *v, const int32_t *sine, float *out_f32) {
    uint32_t i;
    uint32_t pi;
    uint32_t pf;
    int32_t *blk;
    if (v->active == 0u) {
        for (i = 0u; i < 32u; i++) {
            *(out_f32 + i) = 0.0f;
        }
        return;
    }
#ifndef MD_DISABLE_QUIET_PATH
    if (MD_TRY_QUIET48(v, out_f32) != 0u) {
        return;
    }
#endif
#ifndef MD_DSP_HOST_TEST
    /* Render only up to the next original 44.1k block boundary. The helper
     * reproduces each float operation with RND32, commits the exact rational
     * phase, and returns the number of samples written. Keep kernel calls
     * here so no future block is generated early or after the last output. */
    i = 0u;
    while (i < 32u) {
        if (v->pos_int >= 32u) {
            v->prev_s24 = *(v->blk44 + 31u);
            MD_RENDER44(v, sine);
            v->pos_int = v->pos_int - 32u;
        }
        i = i + MD_RESAMPLE_RUN(v, out_f32 + i, 32u - i);
    }
#else
    blk = v->blk44;
    /* The 44.1k kernel reads y[] and blk44[] but never these resampler
     * phase fields. Hold phase/position in local registers for all 32
     * outputs, and commit them at the exact same final state boundary.
     * This avoids roughly 62 repeated reads and stores per 48k block
     * without changing any float operation, rounding or sample order. */
    pi = v->pos_int;
    pf = v->pos_frac;
    for (i = 0u; i < 32u; i++) {
        int32_t s0;
        int32_t s1;
        float f0;
        float df;
        if (pi >= 32u) {
            v->prev_s24 = *(blk + 31u);
            MD_RENDER44(v, sine);
            pi = pi - 32u;
        }
        if (pi == 0u) {
            s0 = v->prev_s24;
        } else {
            s0 = *(blk + (pi - 1u));
        }
        s1 = *(blk + pi);
        f0 = (float)s0 * (1.0f / 8388608.0f);
        df = (float)(s1 - s0) * (float)pf * (1.0f / (8388608.0f * 160.0f));
        *(out_f32 + i) = f0 + df;

        pf = pf + 147u;
        if (pf >= 160u) {
            pf = pf - 160u;
            pi = pi + 1u;
        }
    }
    v->pos_int = pi;
    v->pos_frac = pf;
#endif
}
