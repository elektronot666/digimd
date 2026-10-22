/* Host check of lzma.c on real data: decode PACKED against DICT into
 * EXPECTED's size and compare. usage: decode_test DICT PACKED EXPECTED LC LP PB */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
uint32_t md_lzma_work_bytes(uint32_t,uint32_t);
uint32_t md_lzma_decode(const uint8_t *,uint32_t,const uint8_t *,uint32_t,uint8_t *,uint32_t,uint32_t,uint32_t,uint32_t,void *,uint32_t);
static unsigned char *load(const char *path,size_t *size) {
 FILE *f=fopen(path,"rb");unsigned char *data;
 if(!f||fseek(f,0,SEEK_END)) exit(2);
 *size=(size_t)ftell(f);rewind(f);data=malloc(*size?*size:1);
 if(!data||fread(data,1,*size,f)!=*size) exit(2);
 fclose(f);return data;
}
int main(int argc,char **argv) {
 size_t ds,ps,es;unsigned char *d,*p,*e,*o;uint32_t lc,lp,pb,w,n;void *work;
 if(argc!=7) return 1;
 d=load(argv[1],&ds);p=load(argv[2],&ps);e=load(argv[3],&es);
 lc=atoi(argv[4]);lp=atoi(argv[5]);pb=atoi(argv[6]);w=md_lzma_work_bytes(lc,lp);
 work=malloc(w);o=malloc(es);
 n=md_lzma_decode(p,(uint32_t)ps,d,(uint32_t)ds,o,(uint32_t)es,lc,lp,pb,work,w);
 printf("decoded %u of %zu, work %u, %s\n",n,es,w,n==es&&!memcmp(o,e,es)?"EXACT":"MISMATCH");
 /* Negative: corrupt one packed byte in the middle; must not report an exact decode. */
 p[ps/2]^=0x40;n=md_lzma_decode(p,(uint32_t)ps,d,(uint32_t)ds,o,(uint32_t)es,lc,lp,pb,work,w);
 printf("corrupt: %s\n",n==es&&!memcmp(o,e,es)?"UNDETECTED":"rejected-or-differs");
 return 0;
}
