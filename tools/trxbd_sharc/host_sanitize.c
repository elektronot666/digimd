/* Host-only ASan/UBSan and per-voice bounds exercise. Never linked to SHARC. */
#define TRXBD_HOST_TEST
#include "trxbd_sharc.c"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t before[8];
    TrxBdVoice voice;
    uint32_t after[8];
} GuardedVoice;

int main(int argc,char **argv) {
    if (argc==2 && strcmp(argv[1],"--negative-control")==0) {
        volatile int *bad=(volatile int *)malloc(4*sizeof(int));
        if (!bad) return 2;
        bad[4]=42; /* The supervisor requires ASan to reject this exact write. */
        free((void *)bad);
        return 0;
    }
    unsigned blocks=argc==2?(unsigned)strtoul(argv[1],NULL,10):4096;
    if (blocks<32 || blocks>90000) return 2;
    int32_t *sine=(int32_t *)malloc(32768*sizeof(int32_t));
    GuardedVoice *voices=(GuardedVoice *)calloc(16,sizeof(GuardedVoice));
    if (!sine || !voices) return 2;
    trxbd_build_sine_table(sine);
    for (unsigned t=0;t<16;t++) {
        for (unsigned k=0;k<8;k++) voices[t].before[k]=voices[t].after[k]=0x5aa53cc3u;
        trxbd_voice_init(&voices[t].voice);
    }
    uint32_t random=0x56303u,checksum=0;
    float output[32];
    for (unsigned b=0;b<blocks;b++) {
        for (unsigned t=0;t<16;t++) {
            TrxBdVoice *voice=&voices[t].voice;
            if (b%3==0) {
                uint32_t parameters[8];
                for (unsigned k=0;k<8;k++) {
                    random=random*1664525u+1013904223u;
                    parameters[k]=random%0x3f81u;
                }
                trxbd_compute_coeffs(parameters,voice->y);
            }
            if (b==0 || b%17==t) {
                trxbd_voice_trigger(voice);
                trxbd_render_block_44k1(voice,sine);
                voice->prev_s24=voice->blk44[0];
                voice->pos_int=voice->pos_frac=0;
            }
            trxbd_render_block_48k_float(voice,sine,output);
            for (unsigned k=0;k<32;k++) {
                if (!isfinite(output[k])) return 3;
                checksum^=(uint32_t)voice->blk44[k];
            }
            for (unsigned k=0;k<8;k++)
                if (voices[t].before[k]!=0x5aa53cc3u || voices[t].after[k]!=0x5aa53cc3u) return 4;
        }
    }
    printf("{\"blocks\":%u,\"voices\":16,\"checksum\":%u,\"voice_bytes\":%zu}\n",blocks,checksum,sizeof(TrxBdVoice));
    free(voices);
    free(sine);
    return 0;
}
