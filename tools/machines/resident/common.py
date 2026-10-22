"""Reproducible inputs and ADSP-21569 DDR VISA address mapping."""
from pathlib import Path
import hashlib,json,subprocess
from tools.machines.registry import ROOT
OUT=ROOT/'out/machines/resident49'
BASE=ROOT/'out/machines/translation'
PIN='2b26d3b75c53063575bc5c820fa0d38879335187'

def sha(path):return hashlib.sha256(Path(path).read_bytes()).hexdigest()

def qualified_inputs():
    from tools.machines.translation.inputs import inputs
    result={}
    rows=json.loads((BASE/'batch.json').read_text())['machines']
    for m in rows:
        if m['name'] in ('TRX-BD','TRX-SD'):continue
        p=json.loads((BASE/m['name']/'native-proof.json').read_text())
        assert p['status']=='PASS' and p['blocks']>=384 and p['resampling48000']['status']=='PASS',m['name']
        assert p['source_inputs']==inputs(m),(m['name'],'stale original parity')
        assert p['oracle_sha256']==sha(BASE/'oracle/build/mdsequence')
        result.update(p['source_inputs'])
    assert len([m for m in rows if m['name'] not in ('TRX-BD','TRX-SD')])==47
    result.update({str(p.relative_to(ROOT)):sha(p) for p in Path(__file__).parent.iterdir() if p.suffix in ('.py','.cpp')})
    for name in ('tools/trxbd_sharc/trxbd_sharc.c','tools/trxsd/trxsd.c','tools/trxsd/filter.s','tools/machines/dispatch.c','tools/machines/dispatch.h'):
        p=ROOT/name
        if p.exists():result[name]=sha(p)
    return result

def install_alias():
    from tools.trxbd_sharc import build_sharc as B
    import vm as V
    old=B.E.sw_to_byte
    def alias(pc):
        return 0x80a00000+2*(pc-0x800000) if 0x800000<=pc<0xb00000 else old(pc)
    B.E.sw_to_byte=alias;V.D.sw_to_byte=alias

def check_profile():
    assert json.loads((OUT/'inputs.json').read_text())==qualified_inputs(),'Resident profile inputs changed'
    proof=json.loads((OUT/'build-proof.json').read_text())
    assert proof['status']=='PASS-build-only' and proof['dxe_sha256']==sha(OUT/'profile.dxe')
    assert proof['inputs_sha256']==sha(OUT/'inputs.json')
    for path,digest in json.loads((OUT/'objects.json').read_text()).items():
        assert sha(ROOT/path)==digest,('Resident object changed',path)

def linker():
    original=ROOT/'out/external/selache';copy=OUT/'toolchain/source';target=OUT/'toolchain/target';binary=target/'release/seld'
    assert subprocess.check_output(['git','-C',str(original),'rev-parse','HEAD'],text=True).strip()==PIN
    assert not subprocess.check_output(['git','-C',str(original),'status','--porcelain'],text=True).strip()
    if not binary.exists():
        copy.mkdir(parents=True,exist_ok=True)
        subprocess.run(['rsync','-a','--exclude=.git','--exclude=target',str(original)+'/',str(copy)+'/'],check=True)
        p=copy/'seld/src/layout.rs';text=p.read_text();old='''    } else {
        byte_addr / 2
    }
}''';new='''    } else if (0x80a00000..0x81000000).contains(&byte_addr) {
        // ADSP-21569 data sheet Table 6: DDR VISA fetch alias.
        0x00800000 + (byte_addr - 0x80a00000) / 2
    } else {
        byte_addr / 2
    }
}'''
        assert text.count(old)==1;text=text.replace(old,new);p.write_text(text)
        with (OUT/'linker-build.log').open('w') as log:
            subprocess.run(['cargo','build','--offline','--locked','--release','--manifest-path',str(copy/'Cargo.toml'),'-p','seld','--target-dir',str(target)],check=True,stdout=log,stderr=log)
    manifest=dict(upstream_commit=PIN,source_sha256=sha(copy/'seld/src/layout.rs'),binary_sha256=sha(binary),primary_source='https://www.analog.com/media/en/technical-documentation/data-sheets/ADSP-21562-21563-21565-21566-21567-21569.pdf Table 6',limits=['Only the isolated linker address conversion is extended; compiler, assembler, VM PC masks and stack limits are unchanged.','DDR ownership, loader delivery, cache and physical timing remain unqualified.'])
    (OUT/'linker.json').write_text(json.dumps(manifest,indent=2)+'\n')
    return binary

def original_oracle():
    from tools.build_md_reference import PIN as ORIGINAL_PIN
    source=ROOT/'out/external/mpc-vst-machinedrum'
    assert subprocess.check_output(['git','-C',str(source),'rev-parse','HEAD'],text=True).strip()==ORIGINAL_PIN
    assert not subprocess.check_output(['git','-C',str(source),'status','--porcelain'],text=True).strip()
    folder=OUT/'oracle';folder.mkdir(parents=True,exist_ok=True)
    (folder/'CMakeLists.txt').write_text(f'''cmake_minimum_required(VERSION 3.22)
project(ResidentOriginal LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
add_subdirectory("{source}" upstream EXCLUDE_FROM_ALL)
add_executable(mdmultisequence "{Path(__file__).with_name('original.cpp')}")
target_include_directories(mdmultisequence PRIVATE "{source}" "{source}/libs/dsp56300/source")
target_link_libraries(mdmultisequence PRIVATE mdcore pthread ${{CMAKE_DL_LIBS}})
add_executable(mdmasksequence \"{Path(__file__).with_name('original48.cpp')}\")
target_include_directories(mdmasksequence PRIVATE \"{source}\" \"{source}/libs/dsp56300/source\")
target_link_libraries(mdmasksequence PRIVATE mdcore pthread ${{CMAKE_DL_LIBS}})
''')
    with (folder/'build.log').open('w') as log:
        subprocess.run(['cmake','-S',str(folder),'-B',str(folder/'build'),'-G','Ninja','-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS=-DDSP56K_NO_JIT_RUNTIME -DDSP56K_FORCE_INTERPRETER=1'],check=True,stdout=log,stderr=log)
        subprocess.run(['cmake','--build',str(folder/'build'),'--target','mdmultisequence','mdmasksequence','-j4'],check=True,stdout=log,stderr=log)
    return folder/'build/mdmultisequence'
