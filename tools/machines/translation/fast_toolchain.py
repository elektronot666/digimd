"""Assembler for the fast renders: selas with two encoding fixes.

The pinned selache assembler compacts every unconditional dreg <-> DM move
into the 16-bit VISA Type 3c form, which can only post-modify. A pre-modify
access `Rn = DM(Mm,Ii)` (address Ii + Mm, Ii unchanged) therefore assembled
as post-modify. The fast renders use pre-modify addressing, so they are
assembled by a copy of the toolchain (the same isolated copy that carries the
DDR linker fix, tools/machines/resident/common.linker) in which only
post-modify moves are compacted; pre-modify moves keep the Type 3 encoding
with its U bit. Production C objects keep the pinned assembler.

Second fix: `IF cond DM(imm,Ii) = Rn` / `IF cond Rn = DM(imm,Ii)` without a
compute parsed as the condition-free Type 15 form, so the condition was
silently dropped (the access always happened). Such forms are now refused;
conditional accesses use DM(Mm,Ii), whose encoding sharcvm decodes as the
stock image does. (selas's 32-bit Type 4b form is not decoded by sharcvm, so
it is not used.)
"""
import subprocess
from pathlib import Path

from tools.machines.registry import ROOT

COPY = ROOT / 'out/machines/resident49/toolchain/source'
TARGET = ROOT / 'out/machines/resident49/toolchain/target'
OLD = """            cond: 31,
            compute: None,
            ..
        } => try_type3c(write, ureg, i_reg, m_reg),"""
NEW = """            cond: 31,
            compute: None,
            // the 16-bit form always post-modifies; DM(Mm,Ii) pre-modify
            // must keep the 48/32-bit Type 3 encoding with its U bit
            post_modify: true,
        } => try_type3c(write, ureg, i_reg, m_reg),"""

COND_OLD = """            cond,
            compute,
            post_modify,
        }),
        other => Ok(other),
    }
}
"""
COND_NEW = """            cond,
            compute,
            post_modify,
        }),
        // A plain `DM(imm, Ii)` access parses as the condition-free Type 15
        // form; it has no conditional encoding, so refuse it instead of
        // silently assembling the access unconditional.
        other if cond == 31 => Ok(other),
        _ => Err(Error::Parse {
            line,
            msg: format!("no conditional encoding for: IF ... {text}"),
        }),
    }
}
"""
PATCHES = (('selinstr/src/visa_encode.rs', OLD, NEW), ('selas/src/asmparse.rs', COND_OLD, COND_NEW))


def selas():
    binary = TARGET / 'release/selas'
    if not COPY.exists():
        from tools.machines.resident import common
        common.linker()
    for path, old, new in PATCHES:
        p = COPY / path
        text = p.read_text()
        if new not in text:
            assert text.count(old) == 1, path
            p.write_text(text.replace(old, new))
            binary.unlink(missing_ok=True)
    if not binary.exists():
        subprocess.run(['cargo', 'build', '--offline', '--release', '--manifest-path', str(COPY / 'Cargo.toml'),
                        '-p', 'selas', '--target-dir', str(TARGET)], check=True)
    return binary
