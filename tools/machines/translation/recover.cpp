// SPDX-License-Identifier: AGPL-3.0-or-later
// Offline decoder/data exporter. Never linked into native audio code.
#include "engine/VoiceEngine.h"
#include "tools/mdfw/Firmware.h"
#include "dsp56kEmu/dsp.h"
#include "dsp56kEmu/peripherals.h"
#include <fstream>
#include <iomanip>
#include <filesystem>
#include <iostream>

static void word(std::ostream& out, uint32_t v) {
    const unsigned char b[4] = {uint8_t(v), uint8_t(v>>8), uint8_t(v>>16), uint8_t(v>>24)};
    out.write(reinterpret_cast<const char*>(b), 4);
}
int main(int argc, char** argv) {
    if(argc != 3) return 2;
    try {
        const auto fw = md::fw::loadFirmware(argv[1]);
        md::engine::VoiceEngine voice(fw);
        auto& dsp = voice.dsp();
        const std::filesystem::path out(argv[2]);
        std::filesystem::create_directories(out);
        std::ofstream dis(out / "decoded.tsv");
        // Decode every possible instruction start in every loaded code record.
        // Reachability is decided later, rather than inferred from dynamic traces.
        for(const auto& r : fw.dspA.records) {
            if(r.space != md::fw::Space::P || r.addr >= 0x148000) continue;
            for(uint32_t pc=r.addr; pc<r.addr+r.words.size(); ++pc) {
                std::string text;
                const auto a=dsp.memory().get(dsp56k::MemArea_P,pc);
                const auto b=dsp.memory().get(dsp56k::MemArea_P,pc+1);
                const auto n=dsp.disassembler().disassemble(text,a,b,0,0x498d,pc);
                dis << std::hex << pc << '\t' << n << '\t' << a << '\t' << b << '\t' << text << '\n';
            }
        }
        for(unsigned space=0; space<3; ++space) {
            std::ofstream data(out / (std::string(1,"PXY"[space])+".u32"),std::ios::binary);
            for(uint32_t a=0; a<(space?0x20000:0x200000); ++a)
                word(data,dsp.memory().get(static_cast<dsp56k::EMemArea>(space),a));
            if(!data) return 4;
        }
        // Parallel moves use the ordinary RAM namespace, including addresses
        // which non-parallel instructions route to peripherals. Prove both
        // the recovered guard contents and that distinction on the original
        // interpreter before specializing those reads in generated native C.
        std::ofstream guard(out / "raw-guard.u32",std::ios::binary);
        for(uint32_t a=0x800000; a<0x900000; ++a) {
            const auto x=dsp.memory().get(dsp56k::MemArea_X,a);
            const auto y=dsp.memory().get(dsp56k::MemArea_Y,a);
            const auto p=dsp.memory().get(dsp56k::MemArea_P,a);
            if(x || y || p) throw std::runtime_error("Nonzero raw RAM guard");
            word(guard,x);
        }
        if(!guard) return 4;
        dsp.setInterpreterEnabled(true);
        dsp.memory().set(dsp56k::MemArea_P,0xc80,0xf09bd8);
        dsp.memory().set(dsp56k::MemArea_X,0xffffb8,0x123456);
        dsp.memory().set(dsp56k::MemArea_Y,0x20,0x234567);
        dsp.getPeriph(dsp56k::MemArea_X)->write(0xffffb8,0x654321);
        const auto port=dsp.getPeriph(dsp56k::MemArea_X)->read(0xffffb8,dsp56k::Movexy);
        for(auto r : {dsp56k::Reg_M3,dsp56k::Reg_M4}) dsp.writeReg(r,dsp56k::TReg24(0xffffff));
        dsp.writeReg(dsp56k::Reg_R3,dsp56k::TReg24(0xffffb8));
        dsp.writeReg(dsp56k::Reg_R4,dsp56k::TReg24(0x20));
        dsp.setPC(0xc80); dsp.execInterpreter();
        int64_t x0=0,y0=0;
        dsp.readRegToInt(dsp56k::Reg_X0,x0); dsp.readRegToInt(dsp56k::Reg_Y0,y0);
        if(x0!=0x123456 || y0!=0x234567 || port!=0x654321)
            throw std::runtime_error("Original parallel RAM namespace probe failed");
        std::ofstream proof(out / "ram-namespace.json");
        proof << "{\"status\":\"PASS\",\"opcode\":\"0xf09bd8\",\"ram_x0\":1193046,\"ram_y0\":2311527,\"peripheral\":6636321,\"zero_guard_words\":1048576}\n";
        if(!proof) return 4;
        if(!dis) return 4;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
