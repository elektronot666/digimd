"""Shared native leaves. Resolve stream layout before assembly, never per sample."""
from pathlib import Path

RUNTIME = Path(__file__).resolve().parent


def sources():
    return sorted(p for p in RUNTIME.iterdir() if p.suffix in ('.h', '.inc', '.s', '.in') or p.name == 'build.py')


def resampler_asm(symbol: str, block_byte: int) -> str:
    if not symbol.isidentifier() or block_byte < 0 or block_byte % 4:
        raise ValueError('Require assembly identifier and word-aligned stream offset')
    # MODIFY immediate is signed 24-bit; ALU offset is a signed 32-bit constant.
    if block_byte + 136 >= 0x2000000:
        raise ValueError('Stream layout exceeds native DAG offset range')
    text = (RUNTIME / 'resample.s.in').read_text()
    for key, value in {'SYMBOL': symbol, 'BLOCK_BYTE': str(block_byte),
                       'PREVIOUS_WORD': hex((block_byte + 128) // 4),
                       'POSITION_WORD': hex((block_byte + 132) // 4)}.items():
        text = text.replace('@' + key + '@', value)
    # Local labels must also be unique when several layouts share one object.
    return text.replace('.L_resample_', f'.L_{symbol}_')


def assembly(*parts: str) -> str:
    """One object, one copy of arithmetic, caller-selected stage ordering.

    Engine stages may use SSF/SSI inline; forcing them through C calls loses
    block-level optimizations. Other engine objects link these same symbols.
    Each part is assembly text with optional section delimiters.
    """
    shared = (RUNTIME / 'arithmetic.s').read_text()
    body = '\n'.join((shared, *parts))
    body = body.replace('.SECTION/SW seg_swco;', '').replace('.ENDSEG;', '')
    return '.SECTION/SW seg_swco;\n' + body + '\n.ENDSEG;\n'


def assert_provenance(report: dict, section: bytes) -> None:
    """Reject stale native probes after any shared include/helper changes."""
    import hashlib
    root = RUNTIME.parents[1]
    if report.get('diagnostic_only'):
        raise AssertionError('Native probes require a production-source build')
    if hashlib.sha256(section).hexdigest() != report['spliced_section7_sha256']:
        raise AssertionError('Section differs from build report')
    for name, expected in report['source_sha256'].items():
        if hashlib.sha256((root / name).read_bytes()).hexdigest() != expected:
            raise AssertionError(f'Source changed after native build: {name}')
