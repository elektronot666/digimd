"""Diagnostic-only original oracle adding shared RAM snapshots; golden prefix checked."""
from pathlib import Path
import shlex
import subprocess
import struct
from tools.machines.registry import ROOT
from tools.machines.compact.build import sha


def run(folder):
    folder=Path(folder).resolve();folder.mkdir(parents=True,exist_ok=True)
    canonical=ROOT/'out/machines/resident49/oracle/build'
    source=ROOT/'tools/machines/resident/original48.cpp'
    text=source.read_text();marker='put(out,dsp.dsp().memory().get(dsp56k::MemArea_Y,32));'
    assert text.count(marker)==1
    text=text.replace(marker,marker+'\n   for(unsigned k=0;k<256;k++)put(out,dsp.dsp().memory().get(dsp56k::MemArea_X,k));\n   for(unsigned k=0;k<352;k++)put(out,dsp.dsp().memory().get(dsp56k::MemArea_Y,k));')
    target=folder/'scratch.cpp';target.write_text(text);obj=folder/'scratch.o';binary=folder/'scratch-oracle'
    commands=subprocess.check_output(['ninja','-C',str(canonical),'-t','commands','mdmasksequence'],text=True).splitlines()
    compile=next(shlex.split(c) for c in commands if ' -c '+str(source) in c)
    compile[compile.index('-o')+1]=str(obj);compile[compile.index('-c')+1]=str(target)
    for flag,value in (('-MF',str(folder/'scratch.d')),('-MT',str(obj))):
        if flag in compile:compile[compile.index(flag)+1]=value
    subprocess.run(compile,cwd=canonical,check=True)
    link=shlex.split(commands[-1].removeprefix(': && ').removesuffix(' && :'))
    link=[str(obj) if s.endswith('original48.cpp.o') else s for s in link];link[link.index('-o')+1]=str(binary)
    subprocess.run(link,cwd=canonical,check=True)
    schedule=folder/'schedule.raw';reference=folder/'scratch.raw'
    subprocess.run([str(binary),str(ROOT/'firmware/Elektron_SPS1-1UW_OS1.63.syx'),str(schedule),str(reference)],check=True)
    raw=reference.read_bytes();width=16*6916+4;stride=width+608*4
    assert len(raw)%stride==0
    # Additional diagnostics must not change any byte of the original result.
    assert b''.join(raw[b:b+width] for b in range(0,len(raw),stride))==(folder/'original.raw').read_bytes()
    (folder/'scratch-source.sha256').write_text(sha(target)+'\n'+sha(binary)+'\n')
    return reference
