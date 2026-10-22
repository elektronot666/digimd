/* Firmware-bound, chunked immutable-asset installer; no DSP callback calls this. */
#include <stdint.h>
#include <stddef.h>
#include "bundle.h"
extern void md_sha256(const uint8_t *,uint32_t,uint8_t *);
#ifdef MD_INSTALL_TEST
extern void *md_allocate(uint32_t);
extern void md_release(void *);
extern int32_t md_read(uint32_t,uint32_t,void *);
extern int32_t md_write(uint32_t,uint32_t,const void *);
extern int32_t md_capacity(void);
#else
#define md_allocate ((void *(*)(uint32_t))0x401202c8u)
#define md_release ((void (*)(void *))0x401204ceu)
#define md_read ((int32_t (*)(uint32_t,uint32_t,void *))0x4012c87au)
#define md_write ((int32_t (*)(uint32_t,uint32_t,const void *))0x4012ca60u)
#define md_capacity ((int32_t (*)(void))0x4012c49au)
#endif
#define FIRST 0x580000u
#define SECTORS 8192u
#define SCAN_BYTES 65536u
#define DATA_MAX 1024u
static uint8_t *pending;
uint32_t md_install_received,md_install_result;
enum { OK=0,BAD_FORMAT=1,NO_MEMORY=2,STORAGE=3,OCCUPIED=4,NOT_READY=5,SEQUENCE=6,HASH=7,IO=8,ALREADY=9 };
static int same(const uint8_t *a,const uint8_t *b,uint32_t n) {uint32_t v=0,i;for(i=0;i<n;i++)v|=a[i]^b[i];return v==0;}
static uint32_t le(const uint8_t *p) {return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24;}
extern void *memcpy(void *,const void *,size_t);
static void copy(uint8_t *d,const uint8_t *s,uint32_t n) {memcpy(d,s,n);}
void md_install_abort(void) {if(pending)md_release(pending);pending=0;md_install_received=0;}
static uint32_t fail(uint32_t value) {md_install_result=value;return value;}
uint32_t md_install_begin(const uint8_t *identity,uint32_t bytes) {
 uint8_t *scan;uint32_t at,i,owned,valid;int32_t capacity;
 if(bytes!=36u||le(identity)!=BUNDLE_PACKED_BYTES||!same(identity+4,bundle_packed_sha,32u))return fail(BAD_FORMAT);
 capacity=md_capacity();if(capacity<0||(uint32_t)capacity<FIRST+SECTORS)return fail(STORAGE);
 md_install_abort();scan=md_allocate(SCAN_BYTES);if(!scan)return fail(NO_MEMORY);
 if(md_read(FIRST,512u,scan)!=0) {md_release(scan);return fail(IO);}
 valid=same(scan,bundle_header,512u);owned=valid;
 if(!owned) {for(i=0;i<512u;i++)if(scan[i]!=((i==7u)?(uint8_t)'P':bundle_header[i]))break;owned=(i==512u);}
 if(!owned) {
  for(at=0;at<SECTORS;at+=SCAN_BYTES/512u) {
   if(md_read(FIRST+at,SCAN_BYTES,scan)!=0) {md_release(scan);return fail(IO);}
   for(i=0;i<SCAN_BYTES;i++)if(scan[i]!=0u&&scan[i]!=255u) {md_release(scan);return fail(OCCUPIED);}
  }
 }
 md_release(scan);pending=md_allocate(BUNDLE_READ_BYTES);if(!pending)return fail(NO_MEMORY);md_install_received=0;
 /* A matching header alone is insufficient: repair a damaged body on upload. */
 (void)valid;return fail(OK);
}
uint32_t md_install_data(uint32_t offset,const uint8_t *data,uint32_t bytes) {
 if(!pending)return fail(NOT_READY);
 if(!bytes||bytes>DATA_MAX||offset>BUNDLE_PACKED_BYTES||bytes>BUNDLE_PACKED_BYTES-offset)return fail(BAD_FORMAT);
 if(offset<md_install_received) {
  if(bytes<=md_install_received-offset&&same(pending+offset,data,bytes))return fail(OK);
  return fail(SEQUENCE);
 }
 if(offset!=md_install_received)return fail(SEQUENCE);
 copy(pending+offset,data,bytes);md_install_received+=bytes;return fail(OK);
}
uint32_t md_install_commit(void) {
 uint8_t hash[32],header[512];uint32_t i;
 if(!pending||md_install_received!=BUNDLE_PACKED_BYTES)return fail(NOT_READY);
 md_sha256(pending,BUNDLE_PACKED_BYTES,hash);if(!same(hash,bundle_packed_sha,32u)) {md_install_abort();return fail(HASH);}
 for(i=BUNDLE_PACKED_BYTES;i<BUNDLE_READ_BYTES;i++)pending[i]=0;
 copy(header,bundle_header,512u);header[7]='P';
 if(md_write(FIRST,512u,header)!=0||md_write(FIRST+1u,BUNDLE_READ_BYTES,pending)!=0||md_read(FIRST+1u,BUNDLE_READ_BYTES,pending)!=0) {md_install_abort();return fail(IO);}
 md_sha256(pending,BUNDLE_PACKED_BYTES,hash);if(!same(hash,bundle_packed_sha,32u)) {md_install_abort();return fail(HASH);}
 if(md_write(FIRST,512u,bundle_header)!=0||md_read(FIRST,512u,header)!=0||!same(header,bundle_header,512u)) {md_install_abort();return fail(IO);}
 md_install_abort();return fail(OK);
}
/* Binary command body decoded from bounded 7-bit SysEx: begin, data, commit, abort/status.
 * CRC32 covers command and raw payload; framing rejects every non-nibble byte. */
uint32_t md_install_crc32(const uint8_t *p,uint32_t n) {uint32_t c=~0u,i;while(n--) {c^=*p++;for(i=0;i<8;i++)c=(c>>1)^((0u-(c&1u))&0xedb88320u);}return ~c;}
uint32_t md_install_message(const uint8_t *p,uint32_t bytes) {
 uint8_t body[1037];uint32_t n,i,command,stored;static const uint8_t prefix[11]={0xf0,0,0x20,0x3c,0x7d,0,'D','M','D','4','9'};
 if(bytes<30u||bytes>11u+2u*sizeof(body)+1u||!same(p,prefix,11u)||p[bytes-1u]!=0xf7u||((bytes-12u)&1u))return fail(BAD_FORMAT);
 n=(bytes-12u)/2u;for(i=0;i<n;i++) {uint32_t a=p[11u+2u*i],b=p[12u+2u*i];if(a>15u||b>15u)return fail(BAD_FORMAT);body[i]=(uint8_t)(a*16u+b);}
 if(n<9u)return fail(BAD_FORMAT);
 stored=le(body+n-4u);
 if(md_install_crc32(body,n-4u)!=stored)return fail(BAD_FORMAT);
 n-=4u;command=body[0];
 if(command==1u)return md_install_begin(body+5,n-5u);
 if(command==2u) {if(n<10u)return fail(BAD_FORMAT);return md_install_data(le(body+5),body+9,n-9u);}
 if(command==3u&&n==5u)return md_install_commit();
 if(command==4u&&n==5u) {md_install_abort();return fail(OK);}
 if(command==5u&&n==5u)return md_install_result;
 return fail(BAD_FORMAT);
}
