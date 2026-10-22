/* Host harness: runs prepare.c, lzma.c and the container's own bytes through
 * the same lookup/read semantics as OS 1.12 (0x4013487a scans the ELE3 table
 * at flash 0x80020, 0x401353ae reads flash). usage: prepare_test CONTAINER FALLBACK FULL49 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
uint8_t *md_prepare_boot(uint8_t *,uint32_t);
extern uint32_t md_boot_status,md_boot_bytes;
volatile uint32_t md_test_cookies[2];
static unsigned char *flash;static size_t flash_size;static int live,fail_alloc_at,allocations;
static uint32_t be(const unsigned char *p) {return (uint32_t)p[0]<<24|(uint32_t)p[1]<<16|(uint32_t)p[2]<<8|p[3];}
void *md_allocate(uint32_t n) {if(++allocations==fail_alloc_at)return NULL;live++;return malloc(n);}
void md_release(void *p) {if(p){live--;free(p);}}
uint32_t md_lookup(uint32_t id,uint32_t *entry) {
 uint32_t i,count=be(flash+0x1c);
 if(memcmp(flash,"ELE3",4)||be(flash+4)!=0x34)return 0;
 for(i=0;i<count;i++){const unsigned char *e=flash+0x20+16*i;
  if(be(e)==id){entry[0]=be(e);entry[1]=be(e+4);entry[2]=be(e+8);entry[3]=be(e+12);return 1;}}
 return 0;
}
void md_read(uint32_t address,uint32_t n,void *out) {
 if(address<0x80000u||address-0x80000u+n>flash_size){fprintf(stderr,"read out of container\n");exit(9);}
 memcpy(out,flash+address-0x80000u,n);
}
static unsigned char *load(const char *path,size_t *size) {
 FILE *f=fopen(path,"rb");unsigned char *data;
 if(!f||fseek(f,0,SEEK_END)) exit(2);
 *size=(size_t)ftell(f);rewind(f);data=malloc(*size);
 if(!data||fread(data,1,*size,f)!=*size) exit(2);
 fclose(f);return data;
}
static size_t fallback_size,full_size;static unsigned char *fallback,*full;
static int run(const char *name,uint32_t expect_status,int expect_full) {
 uint8_t *stock=malloc(1u<<20),*out;int ok;live=1;allocations=0;
 stock[0]=3;memcpy(stock+1,fallback,fallback_size);md_test_cookies[0]=md_test_cookies[1]=0;
 out=md_prepare_boot(stock,(uint32_t)fallback_size+1u);
 if(expect_full) ok=md_boot_status==0&&out!=stock&&md_boot_bytes==full_size+1&&out[0]==3&&!memcmp(out+1,full,full_size)&&
                    md_test_cookies[0]==0x494d4452u&&md_test_cookies[1]==0xb6b2bbadu;
 else ok=md_boot_status==expect_status&&out==stock&&md_boot_bytes==fallback_size+1&&!memcmp(out+1,fallback,fallback_size)&&
         !md_test_cookies[0]&&!md_test_cookies[1];
 ok=ok&&live==1;
 printf("%-26s status %2u live-buffers %d %s\n",name,md_boot_status,live,ok?"PASS":"FAIL");
 free(out);return ok;
}
int main(int argc,char **argv) {
 int ok=1;uint32_t i,count,offset=0;
 if(argc!=4) return 1;
 flash=load(argv[1],&flash_size);fallback=load(argv[2],&fallback_size);full=load(argv[3],&full_size);
 count=be(flash+0x1c);for(i=0;i<count;i++) if(be(flash+0x20+16*i)==9) offset=be(flash+0x20+16*i+4);
 ok&=run("valid-full49",0,1);
 flash[offset+200]^=1;ok&=run("corrupt-packed-byte",7,0);flash[offset+200]^=1;
 flash[offset+3]^=1;ok&=run("corrupt-header",6,0);flash[offset+3]^=1;
 fallback[1000]^=1;ok&=run("different-fallback",3,0);fallback[1000]^=1;
 for(i=1;i<=3;i++){char n[32];fail_alloc_at=(int)i;snprintf(n,sizeof n,"allocation-%u-fails",i);ok&=run(n,i==1?5:i==2?8:9,0);}
 fail_alloc_at=0;
 for(i=0;i<count;i++) if(be(flash+0x20+16*i)==9) flash[0x20+16*i+3]=10;
 ok&=run("section-missing",4,0);
 printf("%s\n",ok?"ALL PASS":"FAILURES");return ok?0:1;
}
