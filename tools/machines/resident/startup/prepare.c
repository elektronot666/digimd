#include <stdint.h>
#include "bundle.h"
/* Cold-boot Full49 selection. Runs once at the original DSP sender, after the
 * OS has read and depacked section 7 (the retained fallback stream) and
 * before DSP reset release. The Full49 stream is stored in its own firmware
 * container section (BUNDLE_SECTION), written by the normal OS upgrade. It is
 * read through the OS's own section lookup and NOR read, LZMA-decoded against
 * the fallback stream, and hash-checked. Any failure returns the untouched
 * fallback stream, so the unit boots exactly as the previous c33 firmware. */
extern void md_sha256(const uint8_t *,uint32_t,uint8_t *);
extern uint32_t md_lzma_work_bytes(uint32_t,uint32_t);
extern uint32_t md_lzma_decode(const uint8_t *,uint32_t,const uint8_t *,uint32_t,uint8_t *,uint32_t,uint32_t,uint32_t,uint32_t,void *,uint32_t);
uint32_t md_boot_bytes=0,md_boot_status=0;
#ifdef MD_BOOT_TEST
extern void *md_allocate(uint32_t);
extern void md_release(void *);
extern uint32_t md_lookup(uint32_t,uint32_t *);
extern void md_read(uint32_t,uint32_t,void *);
extern volatile uint32_t md_test_cookies[2];
#define COOKIE (md_test_cookies[0])
#define COOKIE_INVERSE (md_test_cookies[1])
#else
#define COOKIE (*(volatile uint32_t *)BUNDLE_COOKIE_ADDRESS)
#define COOKIE_INVERSE (*(volatile uint32_t *)(BUNDLE_COOKIE_ADDRESS+4u))
#endif
static int same(const uint8_t *a,const uint8_t *b,uint32_t n) {uint32_t i,v=0;for(i=0;i<n;i++)v|=a[i]^b[i];return v==0;}
uint8_t *md_prepare_boot(uint8_t *stock,uint32_t stockbytes) {
#ifdef MD_BOOT_TEST
 void *(*allocate)(uint32_t)=md_allocate;
 void (*release)(void *)=md_release;
 uint32_t (*lookup)(uint32_t,uint32_t *)=md_lookup;
 void (*read)(uint32_t,uint32_t,void *)=md_read;
#else
 void *(*allocate)(uint32_t)=(void *(*)(uint32_t))0x401202c8u;
 void (*release)(void *)=(void (*)(void *))0x401204ceu;
 uint32_t (*lookup)(uint32_t,uint32_t *)=(uint32_t (*)(uint32_t,uint32_t *))0x4013487au;
 void (*read)(uint32_t,uint32_t,void *)=(void (*)(uint32_t,uint32_t,void *))0x401353aeu;
#endif
 uint32_t entry[4],n,i,worksize;uint8_t *stored,*image,*work,digest[32];
 md_boot_bytes=stockbytes;md_boot_status=1;
 if(!stock||stockbytes!=BUNDLE_PREFIX_BYTES+1u||stock[0]!=3u) {md_boot_status=2;return stock;}
 md_sha256(stock+1,BUNDLE_PREFIX_BYTES,digest);
 if(!same(digest,bundle_prefix_sha,32)) {md_boot_status=3;return stock;}
 if(!lookup(BUNDLE_SECTION,entry)||entry[0]!=BUNDLE_SECTION||entry[2]!=BUNDLE_STORED_BYTES) {md_boot_status=4;return stock;}
 stored=allocate(BUNDLE_STORED_BYTES);if(!stored) {md_boot_status=5;return stock;}
 read(entry[1]+0x80000u,BUNDLE_STORED_BYTES,stored);
 if(!same(stored,bundle_header,BUNDLE_HEADER_BYTES)) {md_boot_status=6;release(stored);return stock;}
 md_sha256(stored+BUNDLE_HEADER_BYTES,BUNDLE_PACKED_BYTES,digest);
 if(!same(digest,bundle_packed_sha,32)) {md_boot_status=7;release(stored);return stock;}
 worksize=md_lzma_work_bytes(BUNDLE_LC,BUNDLE_LP);work=allocate(worksize);
 if(!work) {md_boot_status=8;release(stored);return stock;}
 image=allocate(BUNDLE_IMAGE_BYTES+17u);
 if(!image) {md_boot_status=9;release(work);release(stored);return stock;}
 image[0]=3u;for(i=0;i<16;i++)image[BUNDLE_IMAGE_BYTES+1u+i]=0xa5u;
 md_boot_status=10;
 n=md_lzma_decode(stored+BUNDLE_HEADER_BYTES,BUNDLE_PACKED_BYTES,stock+1,BUNDLE_PREFIX_BYTES,image+1,BUNDLE_IMAGE_BYTES,
                  BUNDLE_LC,BUNDLE_LP,BUNDLE_PB,work,worksize);
 release(work);release(stored);
 if(n!=BUNDLE_IMAGE_BYTES) {md_boot_status=11;release(image);return stock;}
 for(i=0;i<16;i++)if(image[BUNDLE_IMAGE_BYTES+1u+i]!=0xa5u) {md_boot_status=12;release(image);return stock;}
 md_sha256(image+1,n,digest);
 if(!same(digest,bundle_image_sha,32)) {md_boot_status=13;release(image);return stock;}
 /* Same published pair c33's transition writes after a Full49 READY; it only
  * selects the 49-machine menu vector. */
 COOKIE_INVERSE=0xb6b2bbadu;COOKIE=0x494d4452u;
 md_boot_bytes=n+1u;md_boot_status=0;release(stock);return image;
}
