# DigiMD

Project status: Final release / Unmaintained

DigiMD is provided as-is as an experimental firmware research project.

This project is complete from the original author's perspective. No further development, maintenance, updates, support, or bug fixes are planned.

The repository is available for reference, experimentation, and independent development, subject to applicable third-party licenses and rights.

No technical support is provided.

No compatibility or functionality guarantees are made.

No future releases are planned.

This firmware has not been fully validated on hardware.

Use entirely at your own risk.

Anyone interested in continuing development is welcome to pursue an independent fork where permitted by the applicable licenses.

This repository is an archive, not an actively maintained project.


Experimental firmware bringing 49 Machinedrum synthesis machines to Digitone II,
alongside its stock synthesis, sequencing, filters, modulation and effects.
Includes engine source and the tools used to translate, compile and package it.

## Create the included firmware

You need Python 3.10+ and your original Digitone II OS 1.12 `.syx` file.
No additional packages or downloads are needed for this command:

```sh
python3 build_firmware.py "/path/to/Digitone_II_OS1.12.syx"
```

On Windows, use `py -3` instead of `python3`.
The result is `build/DigiMD-FULL49-EXPERIMENTAL-dn2-1.12.syx`.
Input and output hashes are checked. The command applies the included patch;
it does not compile source edits or connect to an instrument.

Back up projects, sounds and settings before testing and retain the original
OS file for recovery. First installation uses
the startup menu: hold **FUNC** while powering on, select **OS UPGRADE**, then
send the generated file over **DIN MIDI**, following the instrument manual.
**Do not use the normal Transfer drop page while running stock OS 1.12:** the
image exceeds its normal receive buffer.

## Source development

The source can be used to study or modify the engines. It is an advanced
firmware project, not a plug-in interface for adding machines.

| Location | Purpose |
| --- | --- |
| `tools/machines/generated/` | Translated synthesis engines and shared tables |
| `tools/trxbd_sharc/`, `tools/trxsd/` | Native TRX-BD and TRX-SD engines |
| `tools/md_runtime/` | Shared synthesis arithmetic, state and resampling |
| `tools/machines/translation/` | Original-code translation and verification |
| `tools/machines/resident/` | DSP linking, dispatch and firmware loader |
| `tools/machines/control.py`, `tools/machines/registry.json` | Machine selection and control integration |
| `data/` | Machine definitions, control defaults and memory constraints |

Source translation additionally requires your original **Machinedrum OS 1.63**
firmware. Prepare the local firmware inputs and download the pinned upstream
source dependencies with:

```sh
python3 tools/prepare_development.py "/path/to/Digitone_II_OS1.12.syx" \
  --md "/path/to/Elektron_SPS1-1UW_OS1.63.syx" --fetch
```

Native development also needs Git, a C/C++ toolchain including clang, CMake,
Ninja, Rust/Cargo, the `m68k-elf` compiler/binutils and liblzma. Build the upstream
native libraries and install their Python dependencies before running native
tools. The pinned upstream projects and revisions are in `data/upstreams.json`.
The native tooling primarily targets Unix/macOS.

Translation starts with `python3 -m tools.machines.translation.recover`, then
`python3 -m tools.machines.translation.generate`. Generation overwrites the
files in `tools/machines/generated/`; edit them after generation if making
direct changes. The `translation.verify` and `translation.native` modules run
host and SHARC comparisons. `tools.machines.provision.assets` prepares the
original data bundle, and `tools.machines.resident.build` compiles the DSP image.
These tools generate their intermediate files under ignored `out/`.

The source packager is `tools.machines.resident.startup.build`. Its `--c33`
input is the `out/source-inputs/receiver.syx` created by the preparation command;
`--full49` selects a rebuilt DSP stream. Changed streams require fresh checks.
The explicit `--unqualified SHA256` option packages a stream with a matching
build receipt while marking the result unqualified. A complete source rebuild of this cleaned distribution
has not been validated; only the included patch's exact reconstruction is verified.

Machine slots and controls are currently defined for the existing 49 engines.
New algorithms or additional machine slots need matching compiler, registry,
control-path, memory and firmware integration changes and appropriate validation.

Upstream code and tools retain their respective licenses and notices. Original
firmware and external toolchain checkouts are not included. This project is
independent of Elektron.
