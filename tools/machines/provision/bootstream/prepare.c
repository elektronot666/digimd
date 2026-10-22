#include <stdint.h>
extern uint32_t md_compose_boot(const uint8_t *,uint32_t,const uint8_t *,uint32_t,uint8_t *,uint32_t);
uint32_t md_boot_bytes,md_boot_status;
static uint32_t le(const uint8_t *p) {return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
uint8_t *md_prepare_boot(uint8_t *stock,uint32_t stockbytes) {
 void *(*allocate)(uint32_t)=(void *(*)(uint32_t))0x401202c8u;
 void (*release)(void *)=(void (*)(void *))0x401204ceu;
 int32_t (*read)(uint32_t,uint32_t,void *)=(int32_t (*)(uint32_t,uint32_t,void *))0x4012c87au;
 uint8_t *asset,*composite;uint32_t n,i;static const uint8_t magic[8]={'D','M','D','S','T','O','R','E'};
 md_boot_bytes=stockbytes;md_boot_status=1;
 asset=allocate(817152u);if(!asset)return stock;
 if(read(0x580000u,512u,asset)<0) {md_boot_status=2;release(asset);return stock;}
 for(i=0;i<8;i++)if(asset[i]!=magic[i]) {md_boot_status=3;release(asset);return stock;}
 if(le(asset+8)!=1||le(asset+12)!=817100||le(asset+16)!=0x580001||le(asset+20)!=4095) {md_boot_status=4;release(asset);return stock;}
 if(read(0x580001u,817152u,asset)<0) {md_boot_status=5;release(asset);return stock;}
 composite=allocate(3u*1024u*1024u);if(!composite) {md_boot_status=6;release(asset);return stock;}
 n=md_compose_boot(stock,stockbytes,asset,817100u,composite,3u*1024u*1024u);release(asset);
 if(!n) {md_boot_status=7;release(composite);return stock;}
 md_boot_bytes=n;md_boot_status=0;release(stock);return composite;
}
