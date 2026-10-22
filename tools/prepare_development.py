#!/usr/bin/env python3
"""Prepare local source-build inputs; optionally fetch pinned upstream tools."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from build_firmware import read_limited, reconstruct

MD_SHA256 = 'a58cd61f2efacfb07add0c643162fb4365c30e73831f2021c17b4aa3b42cabd5'
DN2_SHA256 = '7e5b8a733d4ab1fdf7e5d22757f7d8eddaa3cbf71d0858874a791844c02ceb35'


def copy_firmware(source, destination, expected):
    data = read_limited(source)
    if hashlib.sha256(data).hexdigest() != expected:
        raise ValueError(f'{source.name}: unsupported original firmware')
    if destination.exists():
        if destination.read_bytes() != data:
            raise ValueError(f'{destination.name}: existing input differs; refusing overwrite')
        return
    destination.parent.mkdir(parents=True, exist_ok=True)
    with destination.open('xb') as stream:
        stream.write(data)


def fetch_dependencies():
    dependencies = json.loads((ROOT / 'data/upstreams.json').read_text())
    external = ROOT / 'out/external'
    external.mkdir(parents=True, exist_ok=True)
    for name, dependency in dependencies.items():
        folder = external / name
        if not folder.exists():
            subprocess.run(['git', 'clone', '--no-checkout', dependency['url'], str(folder)], check=True)
            subprocess.run(['git', '-C', str(folder), 'checkout', '--detach', dependency['ref']], check=True)
        actual = subprocess.check_output(['git', '-C', str(folder), 'rev-parse', 'HEAD'], text=True).strip()
        dirty = subprocess.check_output(['git', '-C', str(folder), 'status', '--porcelain'], text=True).strip()
        if actual != dependency['ref'] or dirty:
            raise ValueError(f'{name}: existing checkout differs from the required version; left unchanged')
        subprocess.run(['git', '-C', str(folder), 'submodule', 'update', '--init', '--recursive'], check=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('firmware', type=Path, help='original Digitone II OS 1.12 .syx')
    parser.add_argument('--md', type=Path, help='original Machinedrum OS 1.63 .syx, required for translation')
    parser.add_argument('--fetch', action='store_true', help='download pinned upstream source dependencies')
    args = parser.parse_args()
    copy_firmware(args.firmware, ROOT / 'firmware/Digitone_II_OS1.12.syx', DN2_SHA256)
    if args.md:
        copy_firmware(args.md, ROOT / 'firmware/Elektron_SPS1-1UW_OS1.63.syx', MD_SHA256)
    receiver = ROOT / 'out/source-inputs/receiver.syx'
    release = ROOT / 'release/receiver'
    if receiver.exists():
        manifest = json.loads((release / 'manifest.json').read_text())
        if hashlib.sha256(read_limited(receiver)).hexdigest() != manifest['output_sha256']:
            raise ValueError('Existing receiver differs; refusing overwrite')
    else:
        reconstruct(args.firmware, receiver, release)
    if args.fetch:
        fetch_dependencies()
    print('Local source-build firmware inputs prepared. No instrument was accessed.')
    print('Native compilers, upstream Python dependencies and fresh verification outputs are also required.')


if __name__ == '__main__':
    try:
        main()
    except (OSError, ValueError, subprocess.CalledProcessError) as exc:
        print(f'Preparation failed: {exc}', file=sys.stderr)
        raise SystemExit(1)
