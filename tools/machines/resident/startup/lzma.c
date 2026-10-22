/* Raw LZMA1 decoder with a read-only preset dictionary (the retained
 * fallback DSP stream). Follows the public-domain LzmaSpec reference
 * decoder. Freestanding: no allocation, no libc. Shared by the ColdFire
 * cold-boot loader and the host test, so the host test exercises this code.
 */
#include <stdint.h>

typedef uint16_t Prob;
typedef struct { Prob choice,choice2,low[16u<<3],mid[16u<<3],high[256]; } Len;
typedef struct {
 Prob is_match[12u<<4],is_rep[12],is_rep_g0[12],is_rep_g1[12],is_rep_g2[12],is_rep0_long[12u<<4];
 Prob slot[4][64],special[1u+114u],align[16];
 Len len,rep_len;
 Prob literal[1];
} Model;
typedef struct { const uint8_t *in;uint32_t at,size,range,code,bad; } Rc;

uint32_t md_lzma_work_bytes(uint32_t lc,uint32_t lp) {
 return (uint32_t)sizeof(Model)+(0x300u<<(lc+lp))*(uint32_t)sizeof(Prob);
}
static uint32_t next(Rc *r) { if(r->at>=r->size) {r->bad=1;return 0;} return r->in[r->at++]; }
static void normalize(Rc *r) { if(r->range<(1u<<24)) {r->range<<=8;r->code=(r->code<<8)|next(r);} }
static uint32_t bit(Rc *r,Prob *p) {
 uint32_t v=*p,bound=(r->range>>11)*v,s;
 if(r->code<bound) {v+=((1u<<11)-v)>>5;r->range=bound;s=0;}
 else {v-=v>>5;r->code-=bound;r->range-=bound;s=1;}
 *p=(Prob)v;normalize(r);return s;
}
static uint32_t direct(Rc *r,uint32_t n) {
 uint32_t result=0,t;
 do {r->range>>=1;r->code-=r->range;t=0u-(r->code>>31);r->code+=r->range&t;
  if(r->code==r->range) r->bad=1;
  normalize(r);result=(result<<1)+(t+1u);} while(--n);
 return result;
}
static uint32_t tree(Rc *r,Prob *p,uint32_t n) { uint32_t m=1,i;for(i=0;i<n;i++) m=(m<<1)+bit(r,p+m);return m-(1u<<n); }
static uint32_t reverse(Rc *r,Prob *p,uint32_t n) {
 uint32_t m=1,s=0,i,b;for(i=0;i<n;i++) {b=bit(r,p+m);m=(m<<1)+b;s|=b<<i;}return s;
}
static uint32_t length(Rc *r,Len *l,uint32_t ps) {
 if(!bit(r,&l->choice)) return tree(r,l->low+(ps<<3),3);
 if(!bit(r,&l->choice2)) return 8u+tree(r,l->mid+(ps<<3),3);
 return 16u+tree(r,l->high,8);
}
/* Returns outsize on an exact decode, 0 on any malformed/oversized input.
 * Distances may reach back through out[] into the end of dict[]. Positions
 * for pos/literal state count from the start of out[]; the builder requires
 * dictsize to be a multiple of 1<<max(lp,pb), so this equals liblzma's count. */
uint32_t md_lzma_decode(const uint8_t *in,uint32_t insize,const uint8_t *dict,uint32_t dictsize,
                        uint8_t *out,uint32_t outsize,uint32_t lc,uint32_t lp,uint32_t pb,void *work,uint32_t worksize) {
 Model *m=(Model *)work;Prob *p=(Prob *)work;Rc r;
 uint32_t i,n,pos=0,state=0,rep0=0,rep1=0,rep2=0,rep3=0,ps,len,dist,slot,direct_bits,s,mb,b,prev,distance;
 const uint32_t pmask=(1u<<pb)-1u,lmask=(1u<<lp)-1u;
 if(lc>8||lp>4||pb>4||worksize<md_lzma_work_bytes(lc,lp)||insize<5||in[0]) return 0;
 n=worksize/sizeof(Prob);for(i=0;i<n;i++) p[i]=1024u;
 r.in=in;r.at=1;r.size=insize;r.range=0xffffffffu;r.code=0;r.bad=0;
 for(i=0;i<4;i++) r.code=(r.code<<8)|next(&r);
 if(r.code==r.range) return 0;
 while(pos<outsize) {
  if(r.bad) return 0;
  ps=pos&pmask;
  if(!bit(&r,&m->is_match[(state<<4)+ps])) {
   prev=pos?out[pos-1]:dictsize?dict[dictsize-1]:0;
   p=m->literal+0x300u*(((pos&lmask)<<lc)+(prev>>(8u-lc)));s=1;
   if(state>=7) {
    distance=rep0+1u;
    if(distance<=pos) mb=out[pos-distance];
    else if(distance-pos<=dictsize) mb=dict[dictsize-(distance-pos)];
    else return 0;
    do {b=(mb>>7)&1u;mb<<=1;i=bit(&r,p+((1u+b)<<8)+s);s=(s<<1)|i;if(i!=b) break;} while(s<0x100u);
   }
   while(s<0x100u) s=(s<<1)|bit(&r,p+s);
   out[pos++]=(uint8_t)s;state=state<4?0:state<10?state-3u:state-6u;continue;
  }
  if(bit(&r,&m->is_rep[state])) {
   if(!bit(&r,&m->is_rep_g0[state])) {
    if(!bit(&r,&m->is_rep0_long[(state<<4)+ps])) {
     state=state<7?9u:11u;len=1;goto copy;
    }
   } else {
    if(!bit(&r,&m->is_rep_g1[state])) dist=rep1;
    else {if(!bit(&r,&m->is_rep_g2[state])) dist=rep2;else {dist=rep3;rep3=rep2;}rep2=rep1;}
    rep1=rep0;rep0=dist;
   }
   len=length(&r,&m->rep_len,ps);state=state<7?8u:11u;
  } else {
   rep3=rep2;rep2=rep1;rep1=rep0;len=length(&r,&m->len,ps);state=state<7?7u:10u;
   slot=tree(&r,m->slot[len<3?len:3],6);
   if(slot<4) rep0=slot;
   else {
    direct_bits=(slot>>1)-1u;rep0=(2u|(slot&1u))<<direct_bits;
    if(slot<14) rep0+=reverse(&r,m->special+rep0-slot,direct_bits);
    else {rep0+=direct(&r,direct_bits-4u)<<4;rep0+=reverse(&r,m->align,4);}
    if(rep0==0xffffffffu) return 0; /* end marker before the declared size */
   }
  }
  len+=2;
 copy:
  distance=rep0+1u;
  if(distance>pos+dictsize||len>outsize-pos) return 0;
  while(len&&distance>pos) {out[pos]=dict[dictsize-(distance-pos)];pos++;len--;}
  while(len) {out[pos]=out[pos-distance];pos++;len--;}
 }
 return r.bad?0:outsize;
}
