/* Startup-only prototype. Destination must be unpublished, exclusive storage.
 * Caller must verify manifest SHA256 before invoking this bounded decoder.
 * This module contains no file I/O and is never an audio callback loader. */
#include <stdint.h>
typedef struct { uint32_t ready; uint32_t *source; uint32_t base; uint32_t words; } MdAssetView;
typedef struct { uint32_t *destination; uint32_t capacity_words; MdAssetView *view; } MdAssetRequest;
static uint32_t read_le32(const uint8_t *p) { return (uint32_t)p[0] | ((uint32_t)p[1]<<8) | ((uint32_t)p[2]<<16) | ((uint32_t)p[3]<<24); }
static uint32_t payload_crc32(const uint8_t *p, uint32_t n) {
    uint32_t crc, i, bit;
    crc=0xffffffffu;
    for(i=0u;i<n;i++) {
        crc ^= p[i];
        for(bit=0u;bit<8u;bit++) { crc=(crc>>1)^((0u-(crc&1u))&0xedb88320u); }
    }
    return crc^0xffffffffu;
}
uint32_t md_asset_load(const uint8_t *asset, uint32_t bytes, MdAssetRequest *request) {
    uint32_t count, cursor, remaining, kind, i, value, position;
    uint32_t *destination;
    if (request->view->ready != 0u) return 1u;
    if (bytes<88u || request->capacity_words<0x1e0000u) return 2u;
    if (asset[0]!='D' || asset[1]!='M' || asset[2]!='D' || asset[3]!='A' || asset[4]!='S' || asset[5]!='0' || asset[6]!='0' || asset[7]!='1') return 3u;
    if (read_le32(asset+8)!=0x20000u || read_le32(asset+12)!=0x1e0000u || read_le32(asset+16)!=bytes-88u) return 4u;
    if (read_le32(asset+20)!=0x4affa6d3u || payload_crc32(asset+88u,bytes-88u)!=0x4affa6d3u) return 5u;
    destination=request->destination;cursor=88u;position=0u;
    while(cursor<bytes) {
        if (bytes-cursor<5u) return 6u;
        kind=asset[cursor];count=read_le32(asset+cursor+1u);cursor+=5u;
        remaining=0x1e0000u-position;
        if (count==0u || count>remaining) return 7u;
        if (kind==0u) {
            for(i=0u;i<count;i++) destination[position+i]=0u;
        } else if (kind==1u) {
            if (count>(bytes-cursor)/3u) return 8u;
            for(i=0u;i<count;i++) {
                value=asset[cursor] | ((uint32_t)asset[cursor+1u]<<8) | ((uint32_t)asset[cursor+2u]<<16);
                destination[position+i]=value;cursor+=3u;
            }
        } else return 9u;
        position+=count;
    }
    if (position!=0x1e0000u) return 10u;
    request->view->source=destination;request->view->base=0x20000u;request->view->words=position;
    request->view->ready=0x4d444153u;
    return 0u;
}
