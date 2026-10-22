#define MD_DSP_HOST_TEST
#include "trxsd.c"
#define MD_BUILD_SINE trxsd_test_sine
#include "../md_runtime/sine.inc"
#include <stdio.h>
int main(int argc, char **argv) {
    TrxSdVoice v;
    static int32_t sine[32768];
    uint32_t row[10];
    uint32_t out[98];
    FILE *input;
    FILE *output;
    unsigned i;
    if(argc!=3) return 2;
    input=fopen(argv[1],"rb"); output=fopen(argv[2],"wb");
    if(!input || !output) return 2;
    trxsd_test_sine(sine);trxsd_voice_init(&v);
    while(fread(row,sizeof(row),1,input)==1) {
        trxsd_compute_coeffs(row+2,v.y);
        if(row[1])trxsd_voice_trigger(&v);
        if(row[0])trxsd_render_block_44k1(&v,sine);
        out[0]=row[0];out[1]=row[1];
        for(i=0;i<32;i++)out[2+i]=(uint32_t)v.blk44[i];
        for(i=0;i<64;i++)out[34+i]=i<30?((uint32_t)v.y[i]&0xffffff):0;
        fwrite(out,sizeof(out),1,output);
    }
    fclose(input);fclose(output);return 0;
}
