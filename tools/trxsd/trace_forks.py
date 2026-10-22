"""Build isolated, traced original-code oracles against both pinned DSP forks.

Only a generated copy of VoiceEngine gains tracing and an API compatibility
change. The firmware instructions and pinned dependency checkouts stay intact.
"""
from pathlib import Path
import argparse
import subprocess

ROOT=Path(__file__).resolve().parents[2]
UPSTREAM=ROOT/'out/external/mpc-vst-machinedrum'

TRACE=r'''
            static FILE* trace = std::getenv("DIGIMD_TRACE") ? std::fopen(std::getenv("DIGIMD_TRACE"),"w") : nullptr;
            static unsigned traces=0;
            const auto tracepc=m_dsp->getPC().toWord();
            if(trace && tracepc>=0x10074c && tracepc<0x100881 && traces++<20000) {
                std::fprintf(trace,"%06x",tracepc);
                for(auto reg : {Reg_A,Reg_B,Reg_X,Reg_Y,Reg_SR,Reg_R0,Reg_R1,Reg_R2,Reg_R3,Reg_R4,Reg_R6,Reg_R7}) {
                    int64_t value=0;
                    m_dsp->readRegToInt(reg,value);
                    std::fprintf(trace," %014llx",static_cast<unsigned long long>(value)&0xffffffffffffffULL);
                }
                for(unsigned i=0;i<30;++i) std::fprintf(trace," %06x",m_dsp->memory().get(MemArea_Y,0x800+i)&0xffffff);
                std::fprintf(trace,"\n");
            }
'''

def build(fork):
    out=ROOT/'out/trxsd-forks'/fork
    source=out/'source';source.mkdir(parents=True,exist_ok=True)
    for p in UPSTREAM.iterdir():
        if p.name.startswith('.') or p.name=='engine': continue
        target=source/p.name
        if not target.exists():target.symlink_to(p,target_is_directory=p.is_dir())
    engine=source/'engine';engine.mkdir(exist_ok=True)
    for p in (UPSTREAM/'engine').iterdir():
        target=engine/p.name
        if p.name=='VoiceEngine.cpp':
            text=p.read_text().replace('m_dsp->exec();',TRACE+'\n            m_dsp->exec();')
            if fork=='md-mm':text=text.replace('m_dsp->setInterpreterEnabled(true);','// This fork always initializes the interpreter opcode cache.')
            target.write_text(text)
        elif not target.exists():target.symlink_to(p)
    dsp=UPSTREAM/('libs/dsp56300' if fork=='mpc' else 'libs/gearmulator-md-mm/source/dsp56300')
    wrapper=out/'wrapper';wrapper.mkdir(exist_ok=True)
    (wrapper/'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.22)
project(SDOracle LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory("{source}" upstream EXCLUDE_FROM_ALL)
add_executable(sdtrace "{ROOT}/tools/reference_host/mdsdsequence.cpp")
target_include_directories(sdtrace PRIVATE "{source}" "{dsp}/source")
target_link_libraries(sdtrace PRIVATE mdcore pthread ${{CMAKE_DL_LIBS}})
''')
    log=out/'build.log'
    with log.open('w') as f:
        subprocess.run(['cmake','-S',str(wrapper),'-B',str(out/'build'),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS=-DDSP56K_NO_JIT_RUNTIME -DDSP56K_FORCE_INTERPRETER=1',f'-DMD_DSP56300_DIR={dsp}'],stdout=f,stderr=subprocess.STDOUT,check=True)
        subprocess.run(['cmake','--build',str(out/'build'),'--target','sdtrace','--parallel','4'],stdout=f,stderr=subprocess.STDOUT,check=True)
    return out/'build/sdtrace'

if __name__=='__main__':
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('fork',choices=['mpc','md-mm']);args=p.parse_args()
    print(build(args.fork))
