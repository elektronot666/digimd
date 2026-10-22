#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MD_INSTALL_TEST 1
#include "installer.c"
static uint8_t media[SECTORS*512u];static int allocations,allocation_fail,reads,read_fail,writes,write_fail,corrupt_read;static uint32_t outstanding;
void *md_allocate(uint32_t n) {allocations++;if(allocations==allocation_fail)return 0;void *p=malloc(n);assert(p);outstanding++;return p;}
void md_release(void *p) {assert(outstanding&&p);outstanding--;free(p);}
int32_t md_capacity(void) {return FIRST+SECTORS;}
static uint32_t where(uint32_t sector,uint32_t n) {assert(sector>=FIRST&&n&&n%512==0);uint32_t a=(sector-FIRST)*512;assert(a<=sizeof(media)&&n<=sizeof(media)-a);return a;}
int32_t md_read(uint32_t sector,uint32_t n,void *p) {uint32_t a=where(sector,n);reads++;if(reads==read_fail)return -1;memcpy(p,media+a,n);if(corrupt_read&&sector==FIRST+1u)((uint8_t *)p)[7]^=1;return 0;}
int32_t md_write(uint32_t sector,uint32_t n,const void *p) {uint32_t a=where(sector,n);writes++;if(writes==write_fail)return -1;memcpy(media+a,p,n);return 0;}
static void reset(void) {md_install_abort();assert(outstanding==0);memset(media,0,sizeof(media));allocations=allocation_fail=reads=read_fail=writes=write_fail=corrupt_read=0;}
static void identity(uint8_t *id) {uint32_t n=BUNDLE_PACKED_BYTES;for(int i=0;i<4;i++)id[i]=(uint8_t)(n>>(8*i));memcpy(id+4,bundle_packed_sha,32);}
static void upload(const uint8_t *packed) {for(uint32_t at=0;at<BUNDLE_PACKED_BYTES;) {uint32_t n=BUNDLE_PACKED_BYTES-at;if(n>1024)n=1024;assert(md_install_data(at,packed+at,n)==OK);if(at==0)assert(md_install_data(at,packed+at,n)==OK);at+=n;}}
int main(int argc,char **argv) {assert(argc==3);FILE *f=fopen(argv[1],"rb");assert(f);uint8_t *data=malloc(BUNDLE_PACKED_BYTES),id[36],x[1024];assert(fread(data,1,BUNDLE_PACKED_BYTES,f)==BUNDLE_PACKED_BYTES);fclose(f);identity(id);
 reset();assert(md_install_data(0,data,1)==NOT_READY);assert(md_install_commit()==NOT_READY);assert(md_install_begin(id,35)==BAD_FORMAT);
 reset();allocation_fail=1;assert(md_install_begin(id,36)==NO_MEMORY);assert(!writes&&!outstanding);
 reset();allocation_fail=2;assert(md_install_begin(id,36)==NO_MEMORY);assert(!writes&&!outstanding);
 reset();read_fail=1;assert(md_install_begin(id,36)==IO);assert(!writes&&!outstanding);
 reset();media[sizeof(media)-1]=0x42;assert(md_install_begin(id,36)==OCCUPIED);assert(!writes&&!outstanding);
 reset();assert(md_install_begin(id,36)==OK);assert(md_install_data(1,data,10)==SEQUENCE);assert(md_install_data(~0u,data,1)==BAD_FORMAT);assert(md_install_data(0,data,0)==BAD_FORMAT);upload(data);assert(md_install_commit()==OK);assert(!outstanding);assert(same(media,bundle_header,512)&&same(media+512,data,BUNDLE_PACKED_BYTES));assert(writes==3);
 reset();assert(md_install_begin(id,36)==OK);memcpy(x,data,1024);x[7]^=1;assert(md_install_data(0,x,1024)==OK);assert(md_install_data(0,data,1024)==SEQUENCE);for(uint32_t at=1024;at<BUNDLE_PACKED_BYTES;) {uint32_t n=BUNDLE_PACKED_BYTES-at;if(n>1024)n=1024;assert(md_install_data(at,data+at,n)==OK);at+=n;}assert(md_install_commit()==HASH);assert(writes==0&&!outstanding);
 reset();assert(md_install_begin(id,36)==OK);upload(data);write_fail=2;assert(md_install_commit()==IO);assert(media[7]=='P'&&!outstanding);write_fail=0;assert(md_install_begin(id,36)==OK);upload(data);assert(md_install_commit()==OK);
 reset();assert(md_install_begin(id,36)==OK);upload(data);corrupt_read=1;assert(md_install_commit()==HASH);assert(media[7]=='P'&&!outstanding);
 reset();f=fopen(argv[2],"rb");assert(f);uint8_t frame[2200];uint32_t messages=0,n=0;int byte;while((byte=fgetc(f))!=EOF) {assert(n<sizeof(frame));frame[n++]=(uint8_t)byte;if(byte==0xf7) {assert(md_install_message(frame,n)==OK);n=0;messages++;}}fclose(f);assert(!n&&same(media,bundle_header,512)&&same(media+512,data,BUNDLE_PACKED_BYTES)&&!outstanding);
 assert(md_install_message(frame,1)==BAD_FORMAT);frame[0]^=1;assert(md_install_message(frame,22)==BAD_FORMAT);
 printf("PASS importer protocol %u messages, complete packed asset exact, guards/failure/repair/CRC/ownership cases\n",messages);free(data);return 0;}
