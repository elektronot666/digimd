/* Split of MD_SLOT_PROCESS for the dispatcher's two-voice (SIMD) renders.
 *
 * MD_SLOT_PROCESS = MD_SLOT_PRE (parameters, coefficients, trigger) followed
 * by MD_RENDER48 (render48.inc). For pairing, the dispatcher runs each
 * voice's MD_SLOT_PRE and its 48 kHz resampling up to the point where
 * MD_RENDER48 would call MD_RENDER44, renders the two 44.1 kHz blocks at once,
 * and lets each voice finish. These functions are MD_RENDER48's statements in
 * the same order (translated engines build with MD_DISABLE_QUIET_PATH):
 *
 *   _begin   MD_SLOT_PRE, then the inactive-voice silence of MD_RENDER48.
 *            Returns 0 if the block is complete (silent), 1 to resample.
 *   _run     resampling from output sample i; stops before a due render
 *            (prev_s24 already taken) and returns i | MD_SPLIT_DUE, or 32
 *            when the block is complete.
 *   _render  the due render: MD_RENDER44, then the position step.
 *   _rendered the position step alone, after the pair render.
 *
 * Include after the engine's engine_slot.inc, with MD_RESAMPLE_RUN defined
 * (fast_relink checks that the engine's render48.inc has no quiet path).
 */
#define MD_SPLIT_DUE 0x100u
#define MD_SPLIT_FN(suffix) MD_SPLIT_CAT(MD_SLOT_PROCESS, suffix)

uint32_t MD_SPLIT_FN(_begin)(void *memory, uint32_t slot, uint32_t trig, const int32_t *sine, float *out) {
    MD_VOICE_TYPE *v;
    uint32_t i;
    v = (MD_VOICE_TYPE *)memory;
    MD_SLOT_PRE(memory, slot, trig, sine);
    if (v->active == 0u) {
        for (i = 0u; i < 32u; i++) {
            *(out + i) = 0.0f;
        }
        return 0u;
    }
    return 1u;
}

uint32_t MD_SPLIT_FN(_run)(void *memory, float *out, uint32_t i) {
    MD_VOICE_TYPE *v;
    v = (MD_VOICE_TYPE *)memory;
    while (i < 32u) {
        if (v->pos_int >= 32u) {
            v->prev_s24 = *(v->blk44 + 31u);
            return i | MD_SPLIT_DUE;
        }
        i = i + MD_RESAMPLE_RUN(v, out + i, 32u - i);
    }
    return 32u;
}

void MD_SPLIT_FN(_render)(void *memory, const int32_t *sine) {
    MD_VOICE_TYPE *v;
    v = (MD_VOICE_TYPE *)memory;
    MD_RENDER44(v, sine);
    v->pos_int = v->pos_int - 32u;
}

void MD_SPLIT_FN(_rendered)(void *memory) {
    MD_VOICE_TYPE *v;
    v = (MD_VOICE_TYPE *)memory;
    v->pos_int = v->pos_int - 32u;
}
