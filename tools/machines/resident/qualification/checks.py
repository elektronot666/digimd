"""Validate exact native artifacts and existing binary-bound evidence."""
import json
from pathlib import Path
from tools.machines.registry import ROOT
from tools.machines.compact.build import sha
from tools.machines.resident.native import inputs


def check_build(output):
    output=Path(output)
    proof=json.loads((output/'build-proof.json').read_text())
    assert proof['schema']=='digimd.resident.native49-build.v1'
    assert proof['status']=='PASS-build-only' and proof['machines']==49
    assert proof['mandatory_track_mono'] is True
    assert json.loads((output/'inputs.json').read_text())==proof['sources']==inputs(),'Stale native49 inputs'
    for field,name in (('dxe_sha256','profile.dxe'),('stream_sha256','section7.bin'),
                       ('packed_sha256','section7.apl'),('objects_sha256','objects.json')):
        assert proof[field]==sha(output/name),(field,'Native artifact changed')
    for name,digest in json.loads((output/'objects.json').read_text()).items():
        assert sha(ROOT/name)==digest,('Native object changed',name)
    for name,hashes in proof['actual_compilation'].items():
        for field,suffix in (('source_sha256','.c'),('assembly_sha256','.s'),('object_sha256','.doj')):
            source=output/(name+suffix)
            if name=='tables' and suffix=='.c':source=ROOT/'tools/machines/generated/control_tables.c'
            assert hashes[field]==sha(source),(name,field)
    return proof
