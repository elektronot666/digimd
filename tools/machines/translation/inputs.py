"""Content binding for translation, oracle and native parity evidence."""
from pathlib import Path
from tools.machines.translation.recover import ROOT,OUT,sha


def inputs(machine):
    paths=[ROOT/machine['source'],(ROOT/machine['source']).parent/'control_tables.c',ROOT/'tools/machines/md_runtime_compat.h',ROOT/'tools/machines/engine_slot.inc']
    recovered=OUT
    paths += [recovered/name for name in ('recovery.json','P.u32','X.u32','Y.u32','raw-guard.u32','ram-namespace.json') if (recovered/name).exists()]
    paths+=list(Path(__file__).parent.glob('*.py'))+list(Path(__file__).parent.glob('*.cpp'))
    paths+=[p for p in (ROOT/'tools/md_runtime').iterdir() if p.suffix in ('.s','.h','.inc','.in')]
    return {str(p.relative_to(ROOT)):sha(p) for p in sorted(set(paths))}
