"""Identify the distributed source files without requiring repository history."""
import hashlib
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def source_revision():
    digest = hashlib.sha256()
    files = list(ROOT.glob('*.py'))
    for directory in ('tools', 'data'):
        files.extend(p for p in (ROOT / directory).rglob('*')
                     if p.is_file() and '__pycache__' not in p.parts)
    for path in sorted(files):
        digest.update(path.relative_to(ROOT).as_posix().encode() + b'\0')
        digest.update(hashlib.sha256(path.read_bytes()).digest())
    return digest.hexdigest()
