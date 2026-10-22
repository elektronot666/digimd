/* DIAGNOSTIC ONLY. This is deliberately NOT a functioning TRX-BD release.
 * Keep the authentic renderer, tables, FXINIT and hook ABI/layout in the image,
 * but do not execute coefficient/trigger/render code in the audio callback.
 * No instrumentation or telemetry writes; only owned custom buffers are zeroed.
 * Stock FILTER/AMP/FX/mixer and all stock voices continue through native code.
 */
#define trxbd_process trxbd_authentic_process
#include "trxbd_sharc.c"
#undef trxbd_process

void trxbd_process(void) {
    uint32_t t;
    for (t = 0u; t < 16u; t++) {
        if (read_frame_u16(148u + 2u * t) == TRXBD_MACHINE_TYPE) {
            float *out;
            uint32_t i;
            out = (float *)(*(DN2_VOICE_BUF_PTRS + t));
            for (i = 0u; i < 32u; i++) {
                *(out + i) = 0.0f;
            }
        }
    }
}
