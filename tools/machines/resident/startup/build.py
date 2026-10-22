"""Build the permanent Full49 cold-boot firmware: one sealed Digitone II .syx.

Starts from the owner-installed c33 receiver firmware (stock audio confirmed
on the unit) and changes only:
  * section 3: the reset copier also copies a cold-boot loader to 0x47E20000,
    and 0x400cf752 jumps to it at the original DSP sender;
  * new container section 9: Full49 as a raw LZMA stream whose dictionary is
    the retained fallback stream that section 7 already carries.
Sections 2, 4, 5, 7 and 8 keep c33's stored bytes exactly. At every power-on
the loader reads section 9 through the OS's own section lookup/NOR read,
decodes, checks SHA-256 of fallback, packed and decoded data, and hands the
4.3 MB Full49 stream to the unchanged original sender. Any failure sends the
untouched fallback stream instead.

This first receiver-enabled container exceeds the stock in-OS receive buffer,
so it must be installed through the bootstrap's startup-menu OS UPGRADE over
DIN MIDI. Once running, later DigiMD containers within the 3 MiB flash window
can use Transfer. The builder enforces the flash and receiver boundaries.

    python3 -m tools.machines.resident.startup.build [--c33 PATH] [--full49 PATH]
"""
from tools.source_identity import source_revision
import argparse
import hashlib
import hmac
import json
from pathlib import Path
import struct
import re
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent
OUT = ROOT / 'out/firmware/full49-bisect1'
UI_OUT = ROOT / 'out/firmware/full49-bisect1-ui'
PREVIOUS_LOADER_WORK = ROOT / 'out/firmware/full49-usb-pacing/work'
BUNDLE = ROOT / 'out/source-inputs'
PRODUCTION_FULL49 = ROOT / 'out/machines/native49/section7.bin'
STOCK_SHA = '7e5b8a733d4ab1fdf7e5d22757f7d8eddaa3cbf71d0858874a791844c02ceb35'
C33_SHA = 'c33a4d86cb9538855592b78df653e6feaafa172e2705b508d74aaba11b0dc2f6'
C33_S3_SHA = None  # recorded in the manifest; c33 is pinned by its .syx hash
FULL49_SHA = '40b6f6256ddc5d85bb6a9798f082edcd1a073d820e6d7d540430a2b365362295'
ROUTING_FULL49_SHA = 'a42e1c8a8b9d195dca4bf2e1dd5f1e7d8c4a29f65bd4d25272f8d3081dac5864'
PACING_FULL49_SHA = 'cc75d9e4ab1340a2a6349e84370dba982b24d9556cdfd572009b5e9e60ba6e03'
FALLBACK_SHA = '55a2f960b1962cc0993d5502142acd4c34ef2d52e3c96a100a07d8bddb1ab52f'
SECTION = 9
LC, LP, PB = 0, 2, 2
MAIN = 0x40000400
HOOK = 0x400cf752
COPIER = 0x4030c980
C33_RUNTIME, C33_LONGS = 0x4030c9c0, 14574
LOADER_BASE = 0x47e20000
LOADER_LIMIT = 0x47f00000          # top of the documented free DDR area
USB_RX_BASE = 0x46800000
USB_RX_LIMIT = 0x46b10000          # 3.0625 MiB, wholly inside watched-free DDR
USB_RX_FLASH_BYTES = 0x300000      # OS container window: 0x80000..0x380000
USB_RX_MAX_PACKETS = (USB_RX_FLASH_BYTES + 8 + 100) // 101
COOKIE = 0x47e0bad0                # c33 md_full49_ready / _inverse pair
OUTPUT_NAME = 'DigiMD-BISECT1-TRANSLATED-dn2-1.12.syx'
UI_OUTPUT_NAME = 'DigiMD-BISECT1-UI-dn2-1.12.syx'
CF = ['m68k-elf-gcc', '-mcpu=54455', '-Os', '-ffreestanding', '-fno-builtin', '-fno-stack-protector',
      '-fno-zero-initialized-in-bss', '-ffunction-sections', '-fdata-sections', '-Wall', '-Wextra']


def sha(data):
    return hashlib.sha256(data).hexdigest()


def run(*cmd):
    subprocess.run([str(c) for c in cmd], check=True)


def pinned(path, digest, what):
    data = Path(path).read_bytes()
    if sha(data) != digest:
        raise SystemExit(f'{what}: SHA-256 mismatch for {path}')
    return data


def qualified_routing(path):
    """Accept a dispatcher relink only through its differential proof.

    The relinked renderer reuses every engine object of a base profile that
    passed the full original/lifecycle qualification. Its own proof shows
    bit-identical output to that base for every frame the base accepted, the
    corrected stale-voice routing (base as failing control), the load
    policies, and the original Audio Task pacing path."""
    path = Path(path).resolve()
    profile = path.parent
    build_proof = json.loads((profile / 'build-proof.json').read_text())
    routing_path = profile / 'routing-qualification.json'
    pacing_path = profile / 'pacing-proof.json'
    if not routing_path.is_file() or not pacing_path.is_file():
        raise SystemExit(f'Routing profile lacks routing/pacing proofs: {profile}')
    digest = sha(path.read_bytes())
    routing = json.loads(routing_path.read_text())
    pacing = json.loads(pacing_path.read_text())
    if build_proof.get('status') != 'PASS-build-only' or build_proof.get('stream_sha256') != digest:
        raise SystemExit('Routing build proof does not bind the selected stream')
    if routing.get('status') != 'PASS-full49-routing' or routing.get('new_stream_sha256') != digest:
        raise SystemExit('Routing renderer has not passed its differential qualification')
    if pacing.get('status') != 'PASS-Full49-backlog-ownership-correction' or pacing.get('candidate_sha256') != digest:
        raise SystemExit('Routing renderer has not passed the Audio Task pacing qualification')
    base = ROOT / build_proof['base_profile'] / 'section7.bin'
    if routing.get('old_stream_sha256') != sha(base.read_bytes()) or build_proof.get('base_stream_sha256') != sha(base.read_bytes()):
        raise SystemExit('Routing proof is not bound to its base profile')
    # The base's own engine sources were bound when it was qualified; the relink
    # re-verified every reused object against the base object manifest.
    qualified_full49(base, allow_routing=False, bind_build_sources=False)
    for name, expected in build_proof.get('sources', {}).items():
        source = ROOT / name
        if name.startswith('tools/machines/resident/') and (not source.is_file() or sha(source.read_bytes()) != expected):
            raise SystemExit(f'Routing source binding is stale: {name}')
    return path.read_bytes(), profile / 'build-proof.json', routing_path


def qualified_fast(path):
    """Accept a fast-render relink only through its proofs.

    It reuses every object of a qualified routing profile except the
    translated engines' renders (and TRX-BD's canonical scratch store). Its
    proof records the engine gates (original recordings at 44.1/48 kHz and the
    reference engines, host and sharcvm), bit-identical output to the routing
    renderer, the routing behaviour checks, the load matrix, and the original
    Audio Task pacing path."""
    path = Path(path).resolve()
    profile = path.parent
    build_proof = json.loads((profile / 'build-proof.json').read_text())
    fast = json.loads((profile / 'fast-qualification.json').read_text())
    pacing_path = profile / 'pacing-proof.json'
    if not pacing_path.is_file():
        raise SystemExit(f'Fast profile lacks its pacing proof: {profile}')
    pacing = json.loads(pacing_path.read_text())
    digest = sha(path.read_bytes())
    if build_proof.get('status') != 'PASS-build-only' or build_proof.get('stream_sha256') != digest:
        raise SystemExit('Fast build proof does not bind the selected stream')
    if fast.get('status') != 'PASS-full49-fast' or fast.get('new_stream_sha256') != digest:
        raise SystemExit('Fast renderer has not passed its qualification')
    if fast.get('engine_gates', {}).get('status') != 'PASS':
        raise SystemExit('Fast renderer engine gates did not pass')
    link = json.loads((profile / 'fast-link.json').read_text())
    if (link.get('status') != 'PASS' or link.get('stream_sha256') != digest or
            link.get('dxe_sha256') != sha((profile / 'profile.dxe').read_bytes()) or
            link.get('checker_sha256') != sha((HERE.parent / 'qualification/fast_link.py').read_bytes()) or
            link.get('placement', {}).get('status') != 'PASS'):
        raise SystemExit('Fast renderer lacks a current stream-bound on-chip link proof')
    if pacing.get('status') != 'PASS-Full49-backlog-ownership-correction' or pacing.get('candidate_sha256') != digest:
        raise SystemExit('Fast renderer has not passed the Audio Task pacing qualification')
    base = ROOT / build_proof['base_profile'] / 'section7.bin'
    if fast.get('old_stream_sha256') != sha(base.read_bytes()) or build_proof.get('base_stream_sha256') != sha(base.read_bytes()):
        raise SystemExit('Fast proof is not bound to its base profile')
    qualified_routing(base)
    for name, expected in build_proof.get('sources', {}).items():
        source = ROOT / name
        if name.startswith(('tools/machines/resident/fast', 'tools/machines/translation/fast_')) and \
                (not source.is_file() or sha(source.read_bytes()) != expected):
            raise SystemExit(f'Fast source binding is stale: {name}')
    return path.read_bytes(), profile / 'build-proof.json', profile / 'fast-qualification.json'


def qualified_full49(path, allow_routing=True, bind_build_sources=True):
    """Accept only the source-bound production renderer, never a diagnostic stream."""
    path = Path(path).resolve()
    profile = path.parent
    if allow_routing and (profile / 'fast-qualification.json').is_file():
        return qualified_fast(path)
    if allow_routing and (profile / 'routing-qualification.json').is_file():
        return qualified_routing(path)
    build_path = profile / 'build-proof.json'
    qualification_path = profile / 'qualification/proof.json'
    if not build_path.is_file() or not qualification_path.is_file():
        raise SystemExit(f'Full49 profile lacks build/qualification proofs: {profile}')
    build_proof = json.loads(build_path.read_text())
    qualification = json.loads(qualification_path.read_text())
    digest = sha(path.read_bytes())
    if build_proof.get('status') != 'PASS-build-only' or build_proof.get('stream_sha256') != digest:
        raise SystemExit('Full49 build proof does not bind the selected stream')
    if qualification.get('status') != 'PASS-native49-original-and-lifecycle' or qualification.get('stream_sha256') != digest:
        raise SystemExit('Full49 renderer has not passed production qualification')
    if qualification.get('build_proof_sha256') != sha(build_path.read_bytes()):
        raise SystemExit('Full49 qualification does not bind this build proof')
    if qualification.get('audio_pacing') != 'PASS-original-SPORT-callbacks-and-codec-ownership':
        raise SystemExit('Full49 pacing correction is not qualified')
    for name, expected in build_proof.get('sources', {}).items() if bind_build_sources else ():
        source = ROOT / name
        if not source.is_file() or sha(source.read_bytes()) != expected:
            raise SystemExit(f'Full49 source binding is stale: {name}')
    for name, expected in qualification.get('sources', {}).items():
        source = ROOT / name
        if not source.is_file() or sha(source.read_bytes()) != expected:
            raise SystemExit(f'Full49 qualification source is stale: {name}')
    for name, expected in qualification.get('receipts', {}).items():
        receipt = profile / name
        if not receipt.is_file() or sha(receipt.read_bytes()) != expected:
            raise SystemExit(f'Full49 qualification receipt is stale: {name}')
    return path.read_bytes(), build_path, qualification_path


def rebind_loader(previous, header, packed, full49, fallback, work):
    """Patch the bundle constants of a previously built loader.

    prepare.c uses the bundle only through bundle.h: four constant arrays and
    the stored/packed/image byte counts (image also as +1 and +17). In the
    -Os ColdFire build every one of these is a 32-bit literal or array. Each
    old value must occur exactly as often as in the reference build, all
    other loader bytes stay identical, and the result is re-verified by the
    host prepare test and the Unicorn loader proofs."""
    previous = Path(previous)
    old_h = (previous / 'bundle.h').read_text()
    old = {k: int(v) for k, v in re.findall(r'#define (BUNDLE_\w+) (\d+)u', old_h)}
    arrays = {k: bytes(int(x) for x in v.split(',')) for k, v in re.findall(r'static const uint8_t (\w+)\[\d+\]=\{([^}]*)\}', old_h)}
    loader = bytearray((previous / 'loader.bin').read_bytes())
    if old['BUNDLE_PREFIX_BYTES'] != len(fallback) or old['BUNDLE_HEADER_BYTES'] != len(header):
        raise SystemExit('previous loader has a different fallback/header layout')
    if arrays['bundle_prefix_sha'] != hashlib.sha256(fallback).digest():
        raise SystemExit('previous loader has a different fallback dictionary')
    stored = len(header) + len(packed)
    pairs = [(arrays['bundle_header'], bytes(header)),
             (arrays['bundle_image_sha'], hashlib.sha256(full49).digest()),
             (arrays['bundle_packed_sha'], hashlib.sha256(packed).digest())]
    for a, b in ((old['BUNDLE_STORED_BYTES'], stored), (old['BUNDLE_PACKED_BYTES'], len(packed)),
                 (old['BUNDLE_IMAGE_BYTES'], len(full49)), (old['BUNDLE_IMAGE_BYTES'] + 1, len(full49) + 1),
                 (old['BUNDLE_IMAGE_BYTES'] + 17, len(full49) + 17)):
        pairs.append((struct.pack('>I', a), struct.pack('>I', b)))
    expected = {'bundle_header': 1, 'bundle_image_sha': 2, 'bundle_packed_sha': 2}
    counts = []
    reference = bytes(loader)
    for (a, b), name in zip(pairs, ['bundle_header', 'bundle_image_sha', 'bundle_packed_sha',
                                    'stored', 'packed', 'image', 'image+1', 'image+17']):
        hits = [i for i in range(len(reference) - len(a) + 1) if reference[i:i + len(a)] == a]
        counts.append((name, len(hits)))
        for i in hits:
            loader[i:i + len(a)] = b
    want = dict(bundle_header=1, bundle_image_sha=2, bundle_packed_sha=2, stored=3, packed=3, image=4, **{'image+1': 2, 'image+17': 2})
    if dict(counts) != want:
        raise SystemExit(f'previous loader constant layout differs: {counts}')
    manifest = json.loads((previous.parent / 'manifest.json').read_text())['builder_sha256']
    for name in ('prepare.c', 'lzma.c', 'entry.s', 'usb_receiver.s', 'loader.ld'):
        key = str((HERE / name).relative_to(ROOT))
        if manifest.get(key) != sha((HERE / name).read_bytes()):
            raise SystemExit(f'{name} changed since the previous loader build; a ColdFire compiler is required')
    padded = reference + bytes(-len(reference) % 4)
    if manifest_loader_sha(previous) != sha(padded) or not (previous.parent / 'section3.bin').read_bytes().endswith(padded):
        raise SystemExit('previous loader.bin is not the loader of its released section 3')
    (work / 'loader.bin').write_bytes(bytes(loader))
    nm = shutil.which('m68k-elf-nm') or shutil.which('llvm-nm') or shutil.which('llvm-nm-18')
    rows = subprocess.check_output([nm, '-n', str(previous / 'loader.elf')], text=True).splitlines()
    symbols = {r.split()[2]: int(r.split()[0], 16) for r in rows if len(r.split()) == 3}
    shutil.copyfile(previous / 'loader.elf', work / 'loader-reference.elf')
    diff = sum(1 for x, y in zip(reference, loader) if x != y)
    (work / 'loader-rebind.json').write_text(json.dumps(dict(previous=str(previous.relative_to(ROOT)),
        previous_loader_sha256=sha(reference), loader_sha256=sha(bytes(loader)), occurrences=dict(counts),
        changed_bytes=diff), indent=2) + '\n')
    return symbols


def manifest_loader_sha(previous):
    return json.loads((Path(previous).parent / 'manifest.json').read_text())['section3']['loader_sha256']


def c_array(name, data):
    return f'static const uint8_t {name}[{len(data)}]={{' + ','.join(str(b) for b in data) + '};'


def build(c33_path, full49_path, out=None, unqualified=None, name=None, ui=True):
    """unqualified: the SHA-256 of a stream packaged WITHOUT its qualification
    proofs, on the owner's explicit request (named and marked UNQUALIFIED).
    ui: apply the ColdFire UI corrections of ui.py/ui.s (False reproduces
    the tested BISECT1 file)."""
    out = out or (UI_OUT if ui else OUT)
    firmware_name = name or (UI_OUTPUT_NAME if ui else OUTPUT_NAME)
    if unqualified and 'UNQUALIFIED' not in firmware_name:
        firmware_name = firmware_name.replace('.syx', '-UNQUALIFIED.syx')
    sys.path.insert(0, str(ROOT / 'out/external/elekloader'))
    from elekloader import syx
    from elekloader.codec import transport
    from elekloader.devices import digitone_mk2
    from digimd import decode_container, load_firmware, DNII_DEVICE
    dev = digitone_mk2.DEVICE
    out.mkdir(parents=True, exist_ok=True)
    work = out / 'work'
    work.mkdir(exist_ok=True)

    stock = syx.Syx(pinned(ROOT / 'firmware/Digitone_II_OS1.12.syx', STOCK_SHA, 'stock OS1.12'))
    c33 = syx.Syx(pinned(c33_path, C33_SHA, 'c33 firmware'))
    if unqualified:
        full49_path = Path(full49_path).resolve()
        full49 = full49_path.read_bytes()
        full49_build_proof = full49_qualification = full49_path.parent / 'build-proof.json'
        proof = json.loads(full49_build_proof.read_text())
        if proof.get('status') != 'PASS-build-only' or proof.get('stream_sha256') != sha(full49) or sha(full49) != unqualified:
            raise SystemExit('unqualified stream does not match its build proof or the given SHA-256')
        print('WARNING: packaging an UNQUALIFIED Full49 stream (no fast/routing/pacing proofs checked)', flush=True)
    else:
        full49, full49_build_proof, full49_qualification = qualified_full49(full49_path)
    if sha(full49) != (unqualified or FULL49_SHA):
        raise SystemExit(f'production Full49 SHA-256 mismatch for {full49_path}')
    if [t[0] for t in c33.table] != [5, 2, 3, 4, 7, 8]:
        raise SystemExit('unexpected c33 section order')
    for sid in (2, 4, 5, 8):
        if c33.stored[sid] != stock.stored[sid]:
            raise SystemExit(f'c33 section {sid} differs from stock')
    fallback = c33.section(7)
    if sha(fallback) != FALLBACK_SHA:
        raise SystemExit('c33 section 7 is not the retained fallback stream')
    if len(fallback) % (1 << max(LP, PB)):
        raise SystemExit('fallback length breaks the LZMA position alignment')
    main = bytearray(c33.section(3))

    # 1. Full49 against the fallback dictionary, decoded back by the same lzma.c.
    (work / 'fallback.bin').write_bytes(fallback)
    (work / 'full49.bin').write_bytes(full49)
    xz = Path('/opt/homebrew/opt/xz')
    flags = [f'-I{xz}/include', f'-L{xz}/lib'] if xz.is_dir() else []
    run('clang', '-O2', '-Wall', '-Wextra', *flags, '-o', work / 'encode', HERE / 'encode.c', '-llzma')
    run(work / 'encode', work / 'fallback.bin', work / 'full49.bin', work / 'full49.lzma', LC, LP, PB)
    packed = (work / 'full49.lzma').read_bytes()
    header = bytearray(128)
    header[:8] = b'DMD49LZ1'
    struct.pack_into('>6I', header, 8, 1, len(packed), len(full49), len(fallback), LC | LP << 8 | PB << 16, SECTION)
    header[32:64] = hashlib.sha256(full49).digest()
    header[64:96] = hashlib.sha256(packed).digest()
    header[96:128] = hashlib.sha256(fallback).digest()
    stored9 = bytes(header) + packed

    # 2. Compile and link the loader at LOADER_BASE.
    bundle = [f'#define BUNDLE_SECTION {SECTION}u', f'#define BUNDLE_HEADER_BYTES {len(header)}u',
              f'#define BUNDLE_STORED_BYTES {len(stored9)}u', f'#define BUNDLE_PACKED_BYTES {len(packed)}u',
              f'#define BUNDLE_IMAGE_BYTES {len(full49)}u', f'#define BUNDLE_PREFIX_BYTES {len(fallback)}u',
              f'#define BUNDLE_LC {LC}u', f'#define BUNDLE_LP {LP}u', f'#define BUNDLE_PB {PB}u',
              f'#define BUNDLE_COOKIE_ADDRESS 0x{COOKIE:08x}u',
              c_array('bundle_header', header), c_array('bundle_image_sha', header[32:64]),
              c_array('bundle_packed_sha', header[64:96]), c_array('bundle_prefix_sha', header[96:128])]
    (work / 'bundle.h').write_text('\n'.join(bundle) + '\n')
    sha_source = ROOT / 'tools/machines/provision/bootstream/compose.c'
    if shutil.which('m68k-elf-gcc'):
        loader_route = 'compiled'
        sources = {'prepare': HERE / 'prepare.c', 'lzma': HERE / 'lzma.c', 'sha': sha_source,
                   'support': ROOT / 'tools/machines/provision/bootstream/support.c'}
        objects = []
        for name, src in sources.items():
            run(*CF, '-I', work, '-c', src, '-o', work / f'{name}.o')
            objects.append(work / f'{name}.o')
        run('m68k-elf-as', '-mcpu=54455', '-o', work / 'entry.o', HERE / 'entry.s')
        run('m68k-elf-as', '-mcpu=54455', '-o', work / 'usb_receiver.o', HERE / 'usb_receiver.s')
        if ui:
            run('m68k-elf-as', '-mcpu=54455', '-o', work / 'ui.o', HERE / 'ui.s')
            objects.append(work / 'ui.o')
        run('m68k-elf-ld', '-nostdlib', '--gc-sections', '-T', HERE / 'loader.ld', '-o', work / 'loader.elf',
            work / 'entry.o', work / 'usb_receiver.o', *objects)
        run('m68k-elf-objcopy', '-O', 'binary', work / 'loader.elf', work / 'loader.bin')
        symbols = {r.split()[2]: int(r.split()[0], 16) for r in subprocess.check_output(
            ['m68k-elf-nm', '-n', str(work / 'loader.elf')], text=True).splitlines() if len(r.split()) == 3}
        if subprocess.check_output(['m68k-elf-nm', '-u', str(work / 'loader.elf')], text=True).strip():
            raise SystemExit('loader has undefined symbols')
    elif ui:
        raise SystemExit('the UI corrections need the ColdFire toolchain (m68k-elf-gcc)')
    else:
        # No ColdFire compiler on this host: rebind the previously built,
        # source-identical loader to the new bundle (see rebind_loader).
        loader_route = 'rebound from ' + str(PREVIOUS_LOADER_WORK.relative_to(ROOT))
        symbols = rebind_loader(PREVIOUS_LOADER_WORK, header, packed, full49, fallback, work)
    loader = (work / 'loader.bin').read_bytes()
    loader += bytes(-len(loader) % 4)
    if symbols['md_boot_entry'] != LOADER_BASE or LOADER_BASE + len(loader) > LOADER_LIMIT:
        raise SystemExit('loader placement')
    if USB_RX_BASE + 0x20 + USB_RX_MAX_PACKETS * 101 > USB_RX_LIMIT:
        raise SystemExit('USB receiver arena is too small')

    # 3. Patch c33 section 3: copier, sender hook, appended loader.
    def at(address, n):
        return bytes(main[address - MAIN:address - MAIN + n])
    if len(main) != C33_RUNTIME - MAIN + C33_LONGS * 4:
        raise SystemExit('c33 runtime is not the end of section 3')
    old_copier = bytes.fromhex('41f94030c9c043f947e00000203c000038ee22d8538066fa43f947e1000070104299538066fa4ef9400019b0')
    if at(COPIER, len(old_copier)) != old_copier or any(at(COPIER + len(old_copier), 0x40 - len(old_copier))):
        raise SystemExit('c33 reset copier differs')
    if at(0x40000538, 6) != bytes.fromhex('4eb94030c980'):
        raise SystemExit('c33 reset hook differs')
    if at(HOOK, 6) != bytes.fromhex('42a74ebafc9e') or at(0x400cf75c, 6) != bytes.fromhex('4eb947e0bab8'):
        raise SystemExit('DSP sender site differs from c33')
    if at(0x40122574, 8) != bytes.fromhex('203c4038bec04e75'):
        raise SystemExit('USB receiver state accessor differs from OS 1.12')
    if at(0x4012257c, 8) != bytes.fromhex('4fefffe848d71c1c'):
        raise SystemExit('USB receiver decoder prologue differs from OS 1.12')
    if at(0x4012260a, 6) != bytes.fromhex('487800657265'):
        raise SystemExit('USB receiver packet write site differs from OS 1.12')
    if at(0x40122688, 12) != bytes.fromhex('42aa000c14bc000125400008'):
        raise SystemExit('USB receiver start site differs from OS 1.12')
    if at(0x401226d8, 6) != bytes.fromhex('14bc00034280'):
        raise SystemExit('USB receiver completion site differs from OS 1.12')
    # The cookie pair is the one c33's own transition publishes (0x47e088e6..).
    if at(C33_RUNTIME + 0x88e6, 24) != bytes.fromhex('207cb6b2bbad23c847e0bad4203c494d445223c047e0bad0'):
        raise SystemExit('c33 Full49 cookie publication differs')
    copier_src = (HERE / 'copier.s').read_text().replace('C33_LONGS', str(C33_LONGS)).replace(
        'LOADER_BASE', f'0x{LOADER_BASE:08x}').replace('LOADER_LONGS', str(len(loader) // 4))
    copier_src = copier_src.replace('jmp 0x400019b0', f'jmp 0x{symbols["md_usb_reset"]:08x}')
    (work / 'copier.s').write_text(copier_src)
    if loader_route == 'compiled':
        run('m68k-elf-as', '-mcpu=54455', '-o', work / 'copier.o', work / 'copier.s')
        run('m68k-elf-ld', '-nostdlib', f'-Ttext=0x{COPIER:08x}', '-e', 'md_reset_copier', '-o', work / 'copier.elf', work / 'copier.o')
        run('m68k-elf-objcopy', '-O', 'binary', work / 'copier.elf', work / 'copier.bin')
    else:
        # The copier depends only on the loader length/base and the reset
        # entry; reuse the previous one only when its source text is identical.
        if (PREVIOUS_LOADER_WORK / 'copier.s').read_text() != copier_src:
            raise SystemExit('copier source differs from the previous build; a ColdFire assembler is required')
        shutil.copyfile(PREVIOUS_LOADER_WORK / 'copier.bin', work / 'copier.bin')
    copier = (work / 'copier.bin').read_bytes()
    if len(copier) > 0x40:
        raise SystemExit(f'copier is {len(copier)} bytes')
    main[COPIER - MAIN:COPIER - MAIN + len(copier)] = copier
    main[HOOK - MAIN:HOOK - MAIN + 6] = b'\x4e\xf9' + struct.pack('>I', LOADER_BASE)
    main[0x40122576 - MAIN:0x4012257a - MAIN] = struct.pack('>I', USB_RX_BASE)
    for site, symbol, size in ((0x4012257c, 'md_usb_decode_guard', 8),
                               (0x4012260a, 'md_usb_write_guard', 6),
                               (0x40122688, 'md_usb_start_guard', 12),
                               (0x401226d8, 'md_usb_end_guard', 6)):
        patch = b'\x4e\xf9' + struct.pack('>I', symbols[symbol])
        main[site - MAIN:site - MAIN + size] = patch + b'\x4e\x71' * ((size - len(patch)) // 2)
    ui_report = None
    if ui:
        from tools.machines.resident.startup import ui as ui_patch
        ui_report = ui_patch.apply(main, symbols, loader, LOADER_BASE)
    main += loader
    # Exact depacked inputs for the generic safety checker and release audit.
    (out / 'section3.bin').write_bytes(main)
    (out / 'section7.bin').write_bytes(fallback)

    # 4. Container: c33 order, patched main, section 9 appended in the table's padding.
    stored3 = syx.pack_main(bytes(main), dev)
    gaps = {}
    for stage in dev.stages():
        image, gap = syx.inplace_depack(stored3, dev, stage)
        if image != bytes(main) or gap <= 0:
            raise SystemExit(f'section 3 in-place depack fails at 0x{stage:08x}')
        gaps[f'0x{stage:08x}'] = gap
    table = [(sid, dest) for sid, _o, _l, dest in c33.table] + [(SECTION, 0)]
    if syx.TABLE_OFF + syx.ENTRY_SZ * len(table) > c33.data_start or any(c33.container[0x80:c33.data_start]):
        raise SystemExit('no free table slot')
    cont = bytearray(c33.container[:c33.data_start])
    struct.pack_into('>I', cont, syx.COUNT_OFF, len(table))
    for i, (sid, dest) in enumerate(table):
        data = stored3 if sid == 3 else stored9 if sid == SECTION else c33.stored[sid]
        struct.pack_into('>IIII', cont, syx.TABLE_OFF + syx.ENTRY_SZ * i, sid, len(cont), len(data), dest)
        cont += data
        cont += bytes(-len(cont) % 16)
    key = syx.seal_key(stock, dev)
    cont += hmac.new(key, bytes(cont), hashlib.sha256).digest()
    if dev.flash_at + len(cont) > dev.flash_limit:
        raise SystemExit(f'container {len(cont)} B overruns the flash window')
    stream = struct.pack('>II', len(cont), transport.content_checksum(bytes(cont))) + bytes(cont)
    firmware = transport.encode_syx(stream, c33.device_id, c33.framing[0], c33.framing[1])

    # 5. Independent read-back with the repository's own parser.
    path = out / firmware_name
    path.write_bytes(firmware)
    device, decoded, packets = decode_container(firmware)
    parsed = load_firmware(path, DNII_DEVICE)
    if device != DNII_DEVICE or decoded != bytes(cont):
        raise SystemExit('transport roundtrip differs')
    if parsed.section_bytes(3) != bytes(main) or parsed.section_bytes(7) != fallback:
        raise SystemExit('section read-back differs')
    s9 = [s for s in parsed.sections if s.ident == SECTION][0]
    if parsed.container[s9.offset:s9.offset + s9.length] != stored9:
        raise SystemExit('section 9 read-back differs')
    if hmac.new(key, decoded[:-32], hashlib.sha256).digest() != decoded[-32:]:
        raise SystemExit('seal does not verify')
    (work / 'container.bin').write_bytes(decoded)

    # 6. Host run of the real prepare.c/lzma.c against the container bytes.
    run('clang', '-O2', '-Wall', '-Wextra', '-DMD_BOOT_TEST', '-I', work, '-o', work / 'prepare_test',
        HERE / 'prepare_test.c', HERE / 'prepare.c', HERE / 'lzma.c', sha_source)
    run(work / 'prepare_test', work / 'container.bin', work / 'fallback.bin', work / 'full49.bin')

    received = packets * 101
    manifest = dict(
        status='BUILT-UNQUALIFIED-NOT-HARDWARE-VERIFIED' if unqualified else 'BUILT-NOT-HARDWARE-VERIFIED',
        source_commit=source_revision(),
        firmware=firmware_name, firmware_sha256=sha(firmware), firmware_bytes=len(firmware),
        inputs=dict(stock_sha256=STOCK_SHA, c33_sha256=C33_SHA, full49_stream_sha256=sha(full49),
                    fallback_stream_sha256=FALLBACK_SHA, c33_section3_sha256=sha(c33.section(3))),
        builder_sha256={str(p.relative_to(ROOT)): sha(p.read_bytes()) for p in sorted(HERE.glob('*')) if p.is_file() and p.suffix in ('.py', '.c', '.s', '.ld')},
        container_bytes=len(cont), flash_end=hex(dev.flash_at + len(cont)),
        flash_headroom_bytes=dev.flash_limit - dev.flash_at - len(cont),
        data_packets=packets, received_bytes=received, stock_in_os_receive_max=dev.receive_max,
        patched_in_os_receive_max=USB_RX_MAX_PACKETS * 101,
        stock_in_os_receive_overrun_bytes=max(0, received - dev.receive_max),
        install_route=('Install this receiver build once through startup menu (FUNC at power-on) > OS UPGRADE over DIN MIDI. '
                       'Subsequent <=3 MiB containers are accepted by the patched in-OS receiver; physical USB Transfer remains unverified.'),
        sections={str(sid): dict(stored_bytes=len(stored3 if sid == 3 else stored9 if sid == SECTION else c33.stored[sid]),
                                 source='patched c33' if sid == 3 else 'Full49 LZMA bundle' if sid == SECTION else 'c33 unchanged')
                  for sid, _ in table},
        section3=dict(raw_bytes=len(main), raw_sha256=sha(bytes(main)), inplace_gaps=gaps,
                      c33_raw_bytes=len(main) - len(loader), loader_bytes=len(loader), loader_base=hex(LOADER_BASE), copier_bytes=len(copier),
                      hook=hex(HOOK), loader_sha256=sha(loader)),
        usb_receiver=dict(base=hex(USB_RX_BASE), limit=hex(USB_RX_LIMIT), capacity_bytes=USB_RX_LIMIT - USB_RX_BASE,
                          maximum_packets=USB_RX_MAX_PACKETS, maximum_decoded_bytes=USB_RX_MAX_PACKETS * 101,
                          maximum_container_bytes=USB_RX_FLASH_BYTES,
                          guards={name: hex(symbols[name]) for name in ('md_usb_reset', 'md_usb_decode_guard', 'md_usb_start_guard', 'md_usb_write_guard', 'md_usb_end_guard')},
                          verification='checksum and HMAC routines unchanged'),
        full49=dict(raw_bytes=len(full49), raw_sha256=sha(full49), profile=str(Path(full49_path).resolve()),
                    build_proof_sha256=sha(full49_build_proof.read_bytes()),
                    qualification_sha256=sha(full49_qualification.read_bytes()),
                    diagnostic_audio_hooks=False, lzma_bytes=len(packed), lzma_sha256=sha(packed),
                    dictionary='retained fallback stream (section 7)', lc=LC, lp=LP, pb=PB),
        loader_route=loader_route,
        ui_corrections=ui_report,
        host_prepare_test='PASS (valid + 8 negative cases; see build log)',
        not_verified=['Physical USB Transfer reception and timing; this firmware still needs its first installation over startup-menu DIN MIDI.',
                      'Physical boot timing (LZMA decode + SHA-256 of ~5.9 MB on the ColdFire).',
                      'Physical audio deadline and DMA/cache timing; callback backlog correction is offline-qualified only.',
                      'Physical boot/audio with this exact candidate. The previous Full49 cold-boot image booted on hardware.'])
    (out / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    (out / 'SHA256SUMS').write_text(f'{sha(firmware)}  {firmware_name}\n')
    print(json.dumps({k: manifest[k] for k in ('firmware_sha256', 'firmware_bytes', 'container_bytes',
                      'flash_headroom_bytes', 'received_bytes', 'stock_in_os_receive_overrun_bytes')}, indent=1))
    return manifest


if __name__ == '__main__':
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    p.add_argument('--c33', type=Path, default=BUNDLE / 'receiver.syx')
    p.add_argument('--full49', type=Path, default=PRODUCTION_FULL49)
    p.add_argument('--out', type=Path)
    p.add_argument('--no-ui', action='store_true', help='leave out the ColdFire UI corrections (reproduces the tested BISECT1 file)')
    p.add_argument('--unqualified', metavar='SHA256', help="package this stream without its qualification proofs (owner's request); named and marked UNQUALIFIED")
    p.add_argument('--name', help='firmware file name (default ' + OUTPUT_NAME + ')')
    a = p.parse_args()
    build(a.c33, a.full49, a.out, a.unqualified, a.name, ui=not a.no_ui)
