/* Host-only: raw LZMA1 encode of a stream against a preset dictionary.
 * usage: encode DICT INPUT OUTPUT LC LP PB */
#include <lzma.h>
#include <stdio.h>
#include <stdlib.h>

static unsigned char *load(const char *path,size_t *size) {
 FILE *f=fopen(path,"rb");unsigned char *data;
 if(!f||fseek(f,0,SEEK_END)) exit(2);
 *size=(size_t)ftell(f);rewind(f);data=malloc(*size?*size:1);
 if(!data||fread(data,1,*size,f)!=*size) exit(2);
 fclose(f);return data;
}
int main(int argc,char **argv) {
 size_t dsize,isize,capacity,done;unsigned char *dict,*input,*output;FILE *f;
 lzma_options_lzma o;lzma_filter filters[2];
 if(argc!=7) return 1;
 dict=load(argv[1],&dsize);input=load(argv[2],&isize);
 if(lzma_lzma_preset(&o,9u|LZMA_PRESET_EXTREME)) return 3;
 o.dict_size=64u<<20;o.preset_dict=dict;o.preset_dict_size=(uint32_t)dsize;
 o.lc=(uint32_t)atoi(argv[4]);o.lp=(uint32_t)atoi(argv[5]);o.pb=(uint32_t)atoi(argv[6]);
 o.nice_len=273;o.mf=LZMA_MF_BT4;o.depth=0;
 filters[0].id=LZMA_FILTER_LZMA1;filters[0].options=&o;filters[1].id=LZMA_VLI_UNKNOWN;
 capacity=isize+isize/2+65536;output=malloc(capacity);done=0;
 if(!output||lzma_raw_buffer_encode(filters,NULL,input,isize,output,&done,capacity)!=LZMA_OK) return 4;
 f=fopen(argv[3],"wb");if(!f||fwrite(output,1,done,f)!=done) return 5;
 fclose(f);return 0;
}
