// SPDX-License-Identifier: AGPL-3.0-or-later
#include "engine/MachineRunner.h"
#include "engine/VoiceEngine.h"
#include "tools/mdfw/Firmware.h"
#include "dsp56kEmu/dsp.h"
#include "dsp56kEmu/assembler.h"
#include <array>
#include <fstream>
#include <iostream>
static uint32_t get(const unsigned char *p){return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);}
static void put(std::ostream& f,uint32_t v){const unsigned char p[4]={uint8_t(v),uint8_t(v>>8),uint8_t(v>>16),uint8_t(v>>24)};f.write(reinterpret_cast<const char*>(p),4);}
int main(int argc,char **argv){
 if(argc!=4)return 2;
 try {
  auto fw=md::fw::loadFirmware(argv[1]);auto c=md::fw::parseContainer(md::fw::parseSysex(md::fw::readFile(argv[1])));
  md::engine::MachineRunner cpu(std::move(c.sections.at(0).data));md::engine::VoiceEngine dsp(fw);md::engine::VoiceEngine::Block audio{},last{};
  // Gate only the host harness render seam. Original init/trigger and every
  // machine instruction remain unchanged. Skipped voices retain private state.
  dsp56k::Assembler assembler;uint32_t pc=0xc80;
  for(const auto* text : {"move y:>$142,r1","move y:(r1+$1000),a","tst a","jeq $c38","move r0,a","tst a","jmp $c22"}) {
   const auto a=assembler.assemble(text);if(!a.success())throw std::runtime_error(std::string("mask harness assemble: ")+text);
   dsp.dsp().memWriteP(pc++,a.word[0]);if(a.wordCount>1)dsp.dsp().memWriteP(pc++,a.word[1]);
  }
  dsp.dsp().memWriteP(0xc20,0x0af080);dsp.dsp().memWriteP(0xc21,0xc80);
  std::array<bool,16> render{};
  std::ifstream in(argv[2],std::ios::binary);std::ofstream out(argv[3],std::ios::binary);std::array<unsigned char,4+16*44> block{};std::array<std::array<uint32_t,32>,16> coefficients{};std::array<int,16> counts{};
  while(in.read(reinterpret_cast<char*>(block.data()),block.size())){
   cpu.poke32(0x100150c,get(block.data()));
   for(unsigned t=0;t<16;t++){
    auto *r=block.data()+4+44*t;render[t]=get(r+8)!=0;dsp.dsp().memWrite(dsp56k::MemArea_Y,0x1000+t,render[t]);const auto id=get(r);coefficients[t].fill(0);counts[t]=0;if(id==0xffffffffu)continue;
    std::array<uint16_t,8> p{};for(unsigned k=0;k<8;k++)p[k]=uint16_t(get(r+12+4*k));const bool trigger=get(r+4)!=0;
    const int n=cpu.compute(uint8_t(id),p.data(),coefficients[t].data(),32,trigger);if(n<0||n>32)throw std::runtime_error(cpu.faultReason());counts[t]=n;
    auto v=coefficients[t];v[0]=trigger?id+1:0;dsp.setSlot(t,v.data(),n>0?n:1);
   }
   if(!dsp.renderBlock(audio))throw std::runtime_error(dsp.faultReason());
   for(unsigned t=0;t<16;t++){
    put(out,counts[t]);for(auto v:coefficients[t])put(out,v);if(render[t])last[t]=audio[t];for(auto v:last[t])put(out,uint32_t(v));
    for(unsigned k=0;k<64;k++)put(out,dsp.dsp().memory().get(dsp56k::MemArea_Y,0x800+64*t+k));
    for(unsigned k=0;k<64;k++)put(out,dsp.dsp().memory().get(dsp56k::MemArea_X,0x800+64*t+k));
    for(unsigned k=0;k<1536;k++)put(out,dsp.dsp().memory().get(dsp56k::MemArea_P,0x135600+1536*t+k));
   }
   put(out,dsp.dsp().memory().get(dsp56k::MemArea_Y,32));
  }
  if(!in.eof()||!out)return 4;
 }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
