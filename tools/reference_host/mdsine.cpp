// SPDX-License-Identifier: AGPL-3.0-or-later
// Read all sine words after executing the original OS 1.63 DSP2 boot program.
#include "engine/VoiceEngine.h"
#include "tools/mdfw/Firmware.h"
#include "dsp56kEmu/dsp.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

int main(int argc,char** argv) {
    if(argc!=3) {
        std::cerr << "usage: digimd_mdsine ORIGINAL_MD.syx OUTPUT.raw\n";
        return 2;
    }
    try {
        const auto firmware=md::fw::loadFirmware(argv[1]);
        md::engine::VoiceEngine original(firmware); // executes boot, then the first silent block
        std::ofstream output(argv[2],std::ios::binary);
        if(!output) throw std::runtime_error("cannot open table output");
        for(uint32_t index=0;index<32768;++index) {
            const auto word=original.dsp().memory().get(dsp56k::MemArea_Y,0x148000+index)&0xffffff;
            const unsigned char bytes[4]={static_cast<unsigned char>(word),
                static_cast<unsigned char>(word>>8),static_cast<unsigned char>(word>>16),0};
            output.write(reinterpret_cast<const char*>(bytes),4);
        }
        if(!output) throw std::runtime_error("incomplete table output");
    } catch(const std::exception& error) {
        std::cerr << "mdsine: " << error.what() << '\n';
        return 1;
    }
    return 0;
}
