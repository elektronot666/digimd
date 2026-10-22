#include <stdint.h>
extern uint32_t md_install_message(const uint8_t *,uint32_t);
extern uint32_t md_install_received;
extern uint32_t md_install_crc32(const uint8_t *,uint32_t);
extern uint32_t md_install_try_enter(void);
extern void md_install_leave(void);
#define PACKET_CAP 2086u
static uint8_t packet[PACKET_CAP];
static const uint8_t wake=127u;
static volatile uint32_t packet_bytes;
static uint32_t packet_source;
static const uint8_t prefix[11]={0xf0,0,0x20,0x3c,0x7d,0,'D','M','D','4','9'};
static uint32_t sequence(const uint8_t *p,uint32_t n) {
 uint32_t i,result=0;
 if(n<21u)return 0;
 for(i=0;i<4u;i++) {uint32_t a=p[13u+2u*i],b=p[14u+2u*i];if(a>15u||b>15u)return 0;result|=(a*16u+b)<<(8u*i);}
 return result;
}
static void reply(uint32_t source,uint32_t status,uint32_t received,uint32_t seq) {
 uint8_t out[40],body[14];uint32_t i,crc;
 for(i=0;i<11;i++)out[i]=prefix[i];
 body[0]=0x40;body[1]=(uint8_t)status;
 for(i=0;i<4;i++) {body[2+i]=(uint8_t)(received>>(8u*i));body[6+i]=(uint8_t)(seq>>(8u*i));}
 crc=md_install_crc32(body,10u);
 for(i=0;i<4;i++)body[10+i]=(uint8_t)(crc>>(8u*i));
 for(i=0;i<14;i++) {out[11+2*i]=body[i]>>4;out[12+2*i]=body[i]&15u;}
 out[39]=0xf7;
 ((uint32_t (*)(const uint8_t *,uint32_t,uint32_t,uint32_t))0x401236d2u)(out,40u,0u,source);
}
/* Original USB sources 2/4. Disk, allocation and hashing run in service(). */
uint32_t md_install_receive(const uint8_t *p,uint32_t n,uint32_t source) {
 uint32_t i;
 if(n<11u)return 0;
 for(i=0;i<11u;i++)if(p[i]!=prefix[i])return 0;
 if(source!=2u&&source!=4u)return 1;
 if(n<30u||n>PACKET_CAP) {reply(source,1u,0u,sequence(p,n));return 1;}
 if(!md_install_try_enter()) {reply(source,10u,0u,sequence(p,n));return 1;}
 for(i=0;i<n;i++)packet[i]=p[i];
 packet_source=source;packet_bytes=n;
 /* The stock queue stores pointers. Type127 is rejected by the exact OS1.12
  * main-loop bound41, waking the worker without a stock event side effect. */
 ((uint32_t (*)(void *,const void *))0x40001896u)((void *)0x4059e214u,&wake);
 return 1;
}
/* Called only by the existing application message loop. */
void md_install_service(void) {
 uint32_t n=packet_bytes,status,received,source,seq;
 if(!n)return;
 source=packet_source;seq=sequence(packet,n);
 status=md_install_message(packet,n);received=md_install_received;
 packet_bytes=0u;md_install_leave();
 reply(source,status,received,seq);
}
