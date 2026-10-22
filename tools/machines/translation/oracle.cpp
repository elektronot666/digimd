// SPDX-License-Identifier: AGPL-3.0-or-later
// Original Musashi + complete DSP2 oracle, isolated from translated code.
#include "engine/MachineRunner.h"
#include "engine/VoiceEngine.h"
#include "tools/mdfw/Firmware.h"
#include "dsp56kEmu/dsp.h"
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>

static void word(std::ostream& out,uint32_t v) {
    const unsigned char b[4]={uint8_t(v),uint8_t(v>>8),uint8_t(v>>16),uint8_t(v>>24)};
    out.write(reinterpret_cast<const char*>(b),4);
}
static uint32_t read(const unsigned char* p) {
    return uint32_t(p[0])|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16)|(uint32_t(p[3])<<24);
}
int main(int argc,char** argv) {
    if(argc!=5) return 2;
    try {
        const auto id=uint8_t(std::stoi(argv[2]));
        const auto fw=md::fw::loadFirmware(argv[1]);
        auto container=md::fw::parseContainer(md::fw::parseSysex(md::fw::readFile(argv[1])));
        md::engine::MachineRunner cpu(std::move(container.sections.at(0).data));
        md::engine::VoiceEngine dsp(fw);
        std::ifstream schedule(argv[3],std::ios::binary);
        std::ofstream out(argv[4],std::ios::binary);
        std::array<unsigned char,40> row{};
        md::engine::VoiceEngine::Block audio{};
        while(schedule.read(reinterpret_cast<char*>(row.data()),row.size())) {
            std::array<uint16_t,8> p{};
            for(unsigned k=0;k<8;++k) p[k]=uint16_t(read(row.data()+8+4*k));
            const bool trigger=read(row.data()+4)!=0;
            std::array<uint32_t,32> c{};
            const auto n=cpu.compute(id,p.data(),c.data(),32,trigger);
            if(n<0 || n>32) throw std::runtime_error(cpu.faultReason());
            word(out,n);
            for(const auto v:c) word(out,v);
            c[0]=trigger?id+1:0;
            dsp.setSlot(0,c.data(),n>0?n:1);
            if(read(row.data())) {
                if(!dsp.renderBlock(audio)) throw std::runtime_error(dsp.faultReason());
            }
            for(const auto v:audio[0]) word(out,uint32_t(v));
            for(unsigned k=0;k<64;++k) word(out,dsp.dsp().memory().get(dsp56k::MemArea_Y,0x800+k));
            if(std::getenv("MD_EXTENDED_STATE")) {
                for(unsigned k=0;k<64;++k) word(out,dsp.dsp().memory().get(dsp56k::MemArea_X,0x800+k));
                for(unsigned k=0;k<1536;++k) word(out,dsp.dsp().memory().get(dsp56k::MemArea_P,0x135600+k));
                if(std::getenv("MD_SCRATCH_Y20")) word(out,dsp.dsp().memory().get(dsp56k::MemArea_Y,32));
            }
        }
        if(!schedule.eof() || !out) return 4;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
