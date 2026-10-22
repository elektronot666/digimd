"""Run unchanged original-DSP gates against a checked native49 build.

Only output and profile validation are redirected. Original sources, schedules,
comparisons and the pinned original executables are preserved.
"""
import argparse
import json
from pathlib import Path
import runpy
import shutil
import sys
from unittest.mock import patch

from tools.machines.registry import ROOT
from tools.machines.compact.build import sha
from tools.machines.resident import common


def run(output,mode):
    output=Path(output).resolve();canonical=ROOT/'out/machines/resident49'
    proof=json.loads((output/'build-proof.json').read_text())
    from .checks import check_build
    from tools.machines.resident.native import inputs
    baseline_inputs=common.qualified_inputs
    full_inputs=inputs()
    def bound_inputs():
        for name,digest in full_inputs.items():assert sha(ROOT/name)==digest,('Changed native input',name)
        return full_inputs
    def check():
        with patch.object(common,'qualified_inputs',baseline_inputs):return check_build(output)
    check()
    old=common.OUT;common.OUT=output
    # Match the immutable oracle already qualified on the original fixed source.
    mask_proof=json.loads((canonical/'integration48/proof.json').read_text())
    multi_proof=json.loads((canonical/'original/proof.json').read_text())
    assert mask_proof['status']==multi_proof['status']=='PASS'
    assert mask_proof['oracle_sha256']==sha(canonical/'oracle/build/mdmasksequence')
    assert multi_proof['oracle_sha256']==sha(canonical/'oracle/build/mdmultisequence')
    for name in ('mdmasksequence','mdmultisequence'):
        dest=output/'oracle/build'/name;dest.parent.mkdir(parents=True,exist_ok=True)
        shutil.copy2(canonical/'oracle/build'/name,dest)
    source=ROOT/'tools/machines/resident'/('integration48.py' if mode=='48k' else 'original_mixed.py')
    try:
        with patch.object(common,'qualified_inputs',bound_inputs),patch.object(common,'check_profile',check),\
             patch.object(common,'original_oracle',lambda:output/'oracle/build/mdmultisequence'),\
             patch.object(sys,'argv',[str(source)]):
            if mode=='48k':
                runpy.run_module('tools.machines.resident.integration48',run_name='__main__')
                report_path=output/'integration48/proof.json'
            else:
                text=source.read_text()
                for marker,target in (("OUT=ROOT/'out/machines/resident49/original'",output/'original'),
                                      ("ELF=ROOT/'out/machines/resident49/profile.dxe'",output/'profile.dxe')):
                    assert text.count(marker)==1
                    text=text.replace(marker,marker.split('=')[0]+'=Path('+repr(str(target))+')',1)
                exec(compile(text,str(source),'exec'),{'__name__':'__main__','__file__':str(source)})
                report_path=output/'original/proof.json'
        report=json.loads(report_path.read_text());assert report['status']=='PASS'
        report.update(native49_profile=True,driver_sha256=sha(Path(__file__)),
                      profile_build_proof_sha256=sha(output/'build-proof.json'),
                      live_memory_ownership='UNQUALIFIED',firmware_activation='NOT IMPLEMENTED')
        report_path.write_text(json.dumps(report,indent=2)+'\n')
        print('PASS native49 original gate',mode,flush=True);return report
    finally:common.OUT=old


if __name__=='__main__':
    p=argparse.ArgumentParser(__doc__);p.add_argument('mode',choices=('48k','44k'))
    p.add_argument('--out',type=Path,default=ROOT/'out/machines/native49')
    a=p.parse_args();run(a.out,a.mode)
