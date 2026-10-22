// SPDX-License-Identifier: AGPL-3.0-or-later
// Scheduled oracle using the pinned Machinemodule engines, without a translated synth.
// Input rows: ten little-endian u32 words: render, trigger, eight raw14 SYN values.
// Output rows: render, trigger, 32 signed24 PCM in i32, 64 original Y words in u32.
// An update without render preserves the original DSP clock and last PCM block.
#include "engine/MachineRunner.h"
#include "engine/VoiceEngine.h"
#include "tools/mdfw/Firmware.h"
#include "dsp56kEmu/dsp.h"
#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

static uint32_t readWord(std::istream& input) {
    unsigned char bytes[4];
    if (!input.read(reinterpret_cast<char*>(bytes),4))
        throw std::runtime_error("truncated schedule row");
    return uint32_t(bytes[0]) | uint32_t(bytes[1])<<8 | uint32_t(bytes[2])<<16 | uint32_t(bytes[3])<<24;
}

static void writeWord(std::ostream& output,uint32_t word) {
    unsigned char bytes[4] = {static_cast<unsigned char>(word),static_cast<unsigned char>(word>>8),
                             static_cast<unsigned char>(word>>16),static_cast<unsigned char>(word>>24)};
    output.write(reinterpret_cast<const char*>(bytes),4);
    if (!output) throw std::runtime_error("cannot write reference");
}

int main(int argc,char** argv) {
    if (argc != 4) {
        std::cerr << "usage: digimd_mdsequence ORIGINAL_MD.syx SCHEDULE.raw OUTPUT.raw\n";
        return 2;
    }
    try {
        const auto fw = md::fw::loadFirmware(argv[1]);
        auto container = md::fw::parseContainer(md::fw::parseSysex(md::fw::readFile(argv[1])));
        md::engine::MachineRunner originalControl(std::move(container.sections.at(0).data));
        md::engine::VoiceEngine originalDSP(fw);
        std::ifstream schedule(argv[2],std::ios::binary);
        std::ofstream output(argv[3],std::ios::binary);
        if (!schedule || !output) throw std::runtime_error("cannot open schedule/output");
        md::engine::VoiceEngine::Block audio{};
        std::array<int32_t,32> lastPCM{};
        size_t rows=0,renders=0;
        while (schedule.peek() != std::char_traits<char>::eof()) {
            const auto render = readWord(schedule), trigger = readWord(schedule);
            if (render>1 || trigger>1 || (trigger && !render))
                throw std::runtime_error("invalid render/trigger flags");
            std::array<uint16_t,8> parameters{};
            for (auto& parameter:parameters) {
                const auto word = readWord(schedule);
                if (word>0x3f80) throw std::runtime_error("parameter outside raw14 range");
                parameter=static_cast<uint16_t>(word);
            }
            std::array<uint32_t,32> coefficients{};
            if (originalControl.compute(17,parameters.data(),coefficients.data(),32,trigger)!=13)
                throw std::runtime_error("original TRX-SD control failed: "+originalControl.faultReason());
            coefficients[0] = trigger ? 18 : 0;
            originalDSP.setSlot(0,coefficients.data(),13);
            if (render) {
                if (!originalDSP.renderBlock(audio))
                    throw std::runtime_error("original DSP2 failed: "+originalDSP.faultReason());
                lastPCM=audio[0];
                ++renders;
            }
            writeWord(output,render);
            writeWord(output,trigger);
            for (const auto sample:lastPCM) writeWord(output,static_cast<uint32_t>(sample));
            for (uint32_t index=0;index<64;++index)
                writeWord(output,originalDSP.dsp().memory().get(dsp56k::MemArea_Y,0x800+index)&0xffffff);
            if (++rows>1000000) throw std::runtime_error("schedule exceeds one million rows");
        }
        if (!rows) throw std::runtime_error("empty schedule");
        std::cerr << "original MD control/DSP2: " << rows << " updates, " << renders << " rendered blocks\n";
    } catch (const std::exception& error) {
        std::cerr << "mdsequence: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
