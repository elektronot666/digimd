"""Recover original instruction candidates and boot data, off the audio thread."""
import hashlib
import json
import subprocess
import shutil
from pathlib import Path
from tools.machines.registry import ROOT
from tools.build_md_reference import PIN

OUT = ROOT / 'out/machines/translation'
UPSTREAM = ROOT / 'out/external/mpc-vst-machinedrum'


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def recover(out=OUT):
    from digimd import MD_SHA256
    fw = ROOT / 'firmware/Elektron_SPS1-1UW_OS1.63.syx'
    if sha(fw) != MD_SHA256:
        raise ValueError('Require original MD OS 1.63')
    if subprocess.check_output(['git', '-C', str(UPSTREAM), 'rev-parse', 'HEAD'], text=True).strip() != PIN:
        raise ValueError('Require pinned original reference')
    if subprocess.check_output(['git', '-C', str(UPSTREAM), 'status', '--porcelain'], text=True).strip():
        raise ValueError('Original reference checkout is dirty')
    # The reference builder already overlays the fork compatibility fix without
    # changing its pinned checkout. Reuse its isolated source/library build.
    from tools.trxsd.trace_forks import build
    build('mpc')
    base = ROOT / 'out/trxsd-forks/mpc'
    wrapper = base / 'wrapper/CMakeLists.txt'
    text = wrapper.read_text()
    text += f'\nadd_executable(mddecode "{Path(__file__).with_suffix(".cpp")}")\n'
    text += f'target_include_directories(mddecode PRIVATE "{base}/source" "{UPSTREAM}/libs/dsp56300/source")\n'
    text += 'target_link_libraries(mddecode PRIVATE mdcore pthread ${CMAKE_DL_LIBS})\n'
    text += f'add_executable(mdsequence "{Path(__file__).with_name("oracle.cpp")}")\n'
    text += f'target_include_directories(mdsequence PRIVATE "{base}/source" "{UPSTREAM}/libs/dsp56300/source")\n'
    text += 'target_link_libraries(mdsequence PRIVATE mdcore pthread ${CMAKE_DL_LIBS})\n'
    wrapper.write_text(text)
    out.mkdir(parents=True, exist_ok=True)
    with (out / 'recovery-build.log').open('w') as log:
        subprocess.run(['cmake', '-S', str(base/'wrapper'), '-B', str(base/'build')], stdout=log, stderr=log, check=True)
        subprocess.run(['cmake', '--build', str(base/'build'), '--target', 'mddecode', 'mdsequence', '--parallel', '4'], stdout=log, stderr=log, check=True)
    oracle = out / 'oracle/build/mdsequence'
    oracle.parent.mkdir(parents=True, exist_ok=True)
    shutil.copyfile(base/'build/mdsequence', oracle)
    oracle.chmod(0o755)
    binary = base / 'build/mddecode'
    subprocess.run([str(binary), str(fw), str(out)], check=True)
    report = dict(schema='digimd.translation.recovery.v1', firmware_sha256=sha(fw), reference_commit=PIN,
                  decoder_sha256=sha(binary), oracle_sha256=sha(oracle), source_sha256={str(p.relative_to(ROOT)):sha(p) for p in (Path(__file__), Path(__file__).with_suffix('.cpp'))},
                  artifacts={name:sha(out/name) for name in ('decoded.tsv','P.u32','X.u32','Y.u32','raw-guard.u32','ram-namespace.json')},
                  limits=['Decoded candidates are not a control-flow proof.', 'Boot memory is oracle data, never an audio-thread emulator.'])
    (out/'recovery.json').write_text(json.dumps(report, indent=2)+'\n')
    return report


if __name__ == '__main__':
    recover()
