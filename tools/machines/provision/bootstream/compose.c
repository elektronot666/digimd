/* Experimental startup-only boot stream construction. No runtime overlays. */
#include <stdint.h>
static uint32_t le32(const uint8_t *p) { return p[0]|(uint32_t)p[1]<<8|(uint32_t)p[2]<<16|(uint32_t)p[3]<<24; }
static void put32(uint8_t *p,uint32_t v) { p[0]=v;p[1]=v>>8;p[2]=v>>16;p[3]=v>>24; }
static uint32_t rr(uint32_t x,unsigned n) { return (x>>n)|(x<<(32-n)); }
static const uint32_t K[64]={
0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2};
void md_sha256(const uint8_t *data,uint32_t length,uint8_t *digest) {
 uint32_t h[8]={0x6a09e667,0xbb67ae85,0x3c6ef372,0xa54ff53a,0x510e527f,0x9b05688c,0x1f83d9ab,0x5be0cd19};
 uint32_t w[64],v[8],b,i,j,n,a,t,u;uint8_t block[64];
 n=(length+9+63)/64;
 for(b=0;b<n;b++) {
  for(i=0;i<64;i++) { a=b*64+i;block[i]=a<length?data[a]:a==length?128:0; }
  if(b==n-1) { put32(block+56,0);a=length>>29;block[56]=a>>24;block[57]=a>>16;block[58]=a>>8;block[59]=a;a=length<<3;block[60]=a>>24;block[61]=a>>16;block[62]=a>>8;block[63]=a; }
  for(i=0;i<16;i++) w[i]=(uint32_t)block[i*4]<<24|(uint32_t)block[i*4+1]<<16|(uint32_t)block[i*4+2]<<8|block[i*4+3];
  for(i=16;i<64;i++) w[i]=w[i-16]+(rr(w[i-15],7)^rr(w[i-15],18)^(w[i-15]>>3))+w[i-7]+(rr(w[i-2],17)^rr(w[i-2],19)^(w[i-2]>>10));
  for(i=0;i<8;i++) v[i]=h[i];
  for(i=0;i<64;i++) { t=v[7]+(rr(v[4],6)^rr(v[4],11)^rr(v[4],25))+((v[4]&v[5])^(~v[4]&v[6]))+K[i]+w[i];u=(rr(v[0],2)^rr(v[0],13)^rr(v[0],22))+((v[0]&v[1])^(v[0]&v[2])^(v[1]&v[2]));for(j=7;j>0;j--) v[j]=v[j-1];v[4]+=t;v[0]=t+u; }
  for(i=0;i<8;i++) h[i]+=v[i];
 }
 for(i=0;i<8;i++) { digest[i*4]=h[i]>>24;digest[i*4+1]=h[i]>>16;digest[i*4+2]=h[i]>>8;digest[i*4+3]=h[i]; }
}
static int same(const uint8_t *a,const uint8_t *b,uint32_t n) { uint32_t i,v=0;for(i=0;i<n;i++) v|=a[i]^b[i];return v==0; }
static void copy(uint8_t *a,const uint8_t *b,uint32_t n) { uint32_t i;for(i=0;i<n;i++) a[i]=b[i]; }
static void checksum(uint8_t *h) { uint32_t i;uint8_t x=0;h[2]=0;for(i=0;i<16;i++) x^=h[i];h[2]=x; }
static void header(uint8_t *h,uint32_t flags,uint32_t target,uint32_t count,uint32_t arg) { put32(h,0xad000001u|flags);put32(h+4,target);put32(h+8,count);put32(h+12,arg);checksum(h); }
uint32_t md_compose_boot(const uint8_t *stock,uint32_t stockbytes,const uint8_t *asset,uint32_t assetbytes,uint8_t *out,uint32_t capacity) {
 static const uint8_t expected[32]={0xc2,0xbf,0x8d,0x34,0x9f,0x94,0xdd,0xef,0xe6,0xa6,0x96,0x7c,0xa3,0xc0,0x09,0x45,0xcb,0xcf,0x3c,0x78,0x7f,0x16,0x57,0xc3,0x2f,0xcf,0xbb,0x22,0xab,0x1f,0xa9,0x83};
 uint8_t digest[32],x;uint32_t i,j,p,lastfirst=0,final=0,code,count,target,extra=0,words=0,size,write;
 if(assetbytes!=817100u||stockbytes<17||stockbytes>1048576u||stock[0]!=3) return 0;
 md_sha256(asset,assetbytes,digest);if(!same(digest,expected,32)) return 0;
 for(p=1;p<stockbytes;) {
  if(stockbytes-p<16) return 0;
  code=le32(stock+p);count=le32(stock+p+8);target=le32(stock+p+4);x=0;for(i=0;i<16;i++) x^=stock[p+i];
  if(x||(code>>24)!=0xad||(code&15)!=1) return 0;
  if(code&(1u<<14)) lastfirst=p;
  if(code&(1u<<15)) {if(p+16!=stockbytes) return 0;final=p;break;}
  if(!(code&((1u<<11)|(1u<<12)|(1u<<14)))) {if(target>0xffffffffu-count||(target<0x80d80000u&&target+count>0x80600000u)) return 0;}
  size=(code&((1u<<8)|(1u<<11)|(1u<<12)|(1u<<14)))?0:count;
  if(size>stockbytes-p-16) return 0;p+=16+size;
 }
 if(!final||!lastfirst) return 0;
 for(p=88;p<assetbytes;) {if(assetbytes-p<5) return 0;code=asset[p];count=le32(asset+p+1);p+=5;if(code>1||!count||count>0x1e0000u-words)return 0;words+=count;size=code?count*3:0;if(size>assetbytes-p)return 0;p+=size;extra+=16+(code?count*4:0);}
 if(words!=0x1e0000u||extra>capacity||stockbytes>capacity-extra) return 0;
 copy(out,stock,final);write=final;words=0;
 for(p=88;p<assetbytes;) {code=asset[p];count=le32(asset+p+1);p+=5;header(out+write,code?0:1u<<8,0x80600000u+words*4,count*4,0);write+=16;
  if(code) for(i=0;i<count;i++) {j=(uint32_t)asset[p]|(uint32_t)asset[p+1]<<8|(uint32_t)asset[p+2]<<16;p+=3;put32(out+write,j);write+=4;}
  words+=count;
 }
 copy(out+write,stock+final,16);write+=16;put32(out+lastfirst+12,le32(stock+lastfirst+12)+extra);checksum(out+lastfirst);return write;
}
