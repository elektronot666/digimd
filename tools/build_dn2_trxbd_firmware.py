#!/usr/bin/env python3
"""Build and verify the Digitone II OS 1.12 TRX-BD firmware candidate (.syx).

Combines:
  - Section 3 (ColdFire Main OS 1.12): registers machine type 5 ("TRX-BD" / "TBD")
    in MACHINE SEL, maps its 8 SYN controls (PTCH, DEC, RAMP, RDEC, STRT, NOIS,
    HARM, CLIP) with authentic Machinedrum OS 1.63 defaults (64, 64, 0, 0, 0, 0, 0, 0)
    and 0..127 ranges, enables p-locks/LFOs and +Drive kit/project persistence,
    enforces native MONO/single-voice allocation for TRX-BD, and embeds the
    relocated Machinedrum OS 1.63 ColdFire 0x20122A routine.
  - Section 7 (SHARC+ ADSP-21569 DSP): splices the native SHARC+ TRX-BD voice
    synthesis kernel (P:$1003DA..$10050B + boot sine table P:$100068..$10008D +
    ColdFire 0x20122A coefficient calculation) into the pre-filter/pre-AMP voice loop.
  - Sections 2, 4, 5, 8: preserved byte-for-byte from stock Digitone II OS 1.12.
"""
from __future__ import annotations

import argparse
import hashlib
import hmac
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
if str(ROOT) not in sys.path:
    sys.path.insert(0, str(ROOT))

from tools.source_identity import source_revision

from digimd import DNII_DEVICE, DNII_SHA256, MD_DEVICE, MD_SHA256, load_firmware
from digimd_md_cf_transplant import (
    MD_MAIN_BASE,
    RELOCATIONS,
    SOURCE_CODE_END,
    SOURCE_DATA_BEGIN,
    SOURCE_DATA_END,
    SOURCE_ENTRY,
)
from tools.trxbd_sharc.build_sharc import build_section7, verify_in_sharcvm

ELEKLOADER_ROOT = ROOT / "out/external/elekloader"
DN2_CFW_ROOT = ROOT / "out/external/dn2-cfw"

MAIN_BASE = 0x40000400
STOCK_END = 0x4030C980
BOOT_VA = STOCK_END
RUN_LOAD_VA = STOCK_END + 0x40
RUN_VA = 0x47E00000
BSS_VA = 0x47E02000
BSS_WORDS = 16

NEW_TYPE = 5
CLONE_TYPE = 1  # WaveTone parameter slot owner

# The 8 WaveTone parameter record IDs mapped to TRX-BD SYN encoders A..H:
#   A: PTCH -> 254 (slot 41, frame offset 250)
#   B: DEC  -> 255 (slot 42, frame offset 252)
#   C: RAMP -> 252 (slot 38, frame offset 244)
#   D: RDEC -> 253 (slot 40, frame offset 248)
#   E: STRT -> 256 (slot 43, frame offset 254)
#   F: NOIS -> 257 (slot 44, frame offset 256)
#   G: HARM -> 260 (slot 47, frame offset 262)
#   H: CLIP -> 259 (slot 46, frame offset 260)
TRXBD_SYN_RECORDS = (
    (254, "PTCH", "Pitch", 0, 0x7F00, 0x4000),
    (255, "DEC", "Decay", 0, 0x7F00, 0x4000),
    (252, "RAMP", "Ramp", 0, 0x7F00, 0x0000),
    (253, "RDEC", "Ramp Decay", 0, 0x7F00, 0x0000),
    (256, "STRT", "Start", 0, 0x7F00, 0x0000),
    (257, "NOIS", "Noise", 0, 0x7F00, 0x0000),
    (260, "HARM", "Harmonics", 0, 0x7F00, 0x0000),
    (259, "CLIP", "Clip", 0, 0x7F00, 0x0000),
)

GUARDS_112 = (
    (0x40000538, "4eb9400019b0", "OS entry VBR init call (boot copier hook)"),
    (0x401DE168, "0000000000000002000000010000000300000004", "stock 5-entry MACHINE SEL list"),
    (0x4004D8E0, "48780014", "MACHINE SEL list alloc (20 bytes)"),
    (0x4004D900, "4879401de17c", "MACHINE SEL list copy end"),
    (0x4004D906, "41e80014", "MACHINE SEL list capacity (20 bytes)"),
    (0x4004D90A, "4879401de168", "MACHINE SEL list copy begin"),
    (0x40059274, "202f00046d147203b2806c0a123c0004b280670a600470014e7542004e7570024e75", "group(type)"),
    (0x400DC496, "7204", "long name bound"),
    (0x400DC4A6, "41f9401f7c04", "long name table base"),
    (0x400DC4BC, "7204", "short name bound"),
    (0x400DC4CE, "0680401f7c08", "short name table base+4"),
    (0x400DC4E2, "7204", "third column bound"),
    (0x400DC4F4, "0680401f7c0c", "third column table base+8"),
    (0x400DC2C0, "7204", "attr byte bound"),
    (0x400DC2CA, "41f9401f7d40", "attr table base"),
    (0x400DC2E4, "41f9401f7d42", "attr table+2 base (MIDI check)"),
    (0x400DC300, "7404", "attr track-mask bound"),
    (0x400DC316, "41f9401f7d42", "attr track-mask base+2"),
    (0x400DD3EA, "7406", "stored-sound LOAD machine bound (moveq #6,d2)"),
    (0x400DC18E, "2f027264206f0008", "param_set_slot_to_id prologue"),
    (0x400DC196, "202f000c226f0010", "param_set_slot_to_id hook site"),
    (0x400DC19E, "b288", "param_set_slot_to_id resume"),
    (0x4004B7F2, "2f0a246f000820522f0a206800284e90588f4a80671420522f0a206800284e90588f2040712800de600270ff245f4e75", "getMachineType(track)"),
    (0x4004B816, "712800de6002", "getMachineType(track) sound+0xDE read"),
    (0x4005A4AC, "47f94004b7f2", "IDENTITY: MACHINE SEL current machine marker"),
    (0x40071912, "4eb94004b7f2", "IDENTITY: track machine name display"),
    (0x4002DB2C, "4eb94004b7f2", "IDENTITY: re-commit of track machine type"),
    (0x400D5BD6, "4eb94004b7f2", "IDENTITY: track state message machine type"),
    (0x400D68C2, "4eb94004b7f2", "IDENTITY: incoming machine type compare"),
    (0x40071EB6, "4eb94004b7f2", "IDENTITY: CLEAR TRK PRESET"),
    (0x40031394, "4ef94004b7f2", "IDENTITY: getMachineType(model, track) tail jump"),
    (0x40036C24, "772800de2f024eb9400dbe4c", "VALID_SITE: parameter ownership check"),
    (0x400C25F2, "7204202f0004", "SYN overview descriptor(type)"),
    (0x400C257C, "202f00040c8000000141", "runtime parameter drawing descriptor(id)"),
    (0x400C2636, "7204202f0004", "SYN page count(type)"),
    (0x400C2652, "2f027404222f0008202f000c", "SYN page(type, n)"),
    (0x40064622, "4eb9400372da", "SYN knob short label call"),
    (0x400365F2, "2f3018282f02", "parameter long name push"),
    (0x4001821E, "7007b083660000a2", "SYN page WaveTone oscillator icon check"),
    (0x400DC154, "2f02222f00080c8100000141", "limits_getter(id)"),
    (0x40046C20, "204020280008", "getActiveTrack return read"),
    (0x4004CC90, "508f2040114200de", "machine setter sound+0xDE store"),
)


def _cow_string(label: str, text: str) -> str:
    n = len(text.encode("ascii"))
    return f"""    .balign 4
    .long {n}, {n}, -1
{label}:
    .asciz "{text}"
"""


def generate_coldfire_asm(stock_s3: bytes, md_sec0: bytes) -> str:
    """Generate the ColdFire V4 (.boot + .run) assembly for Digitone II OS 1.12."""
    stock_names = stock_s3[0x401F7C04 - MAIN_BASE : 0x401F7C04 - MAIN_BASE + 60]
    stock_attrs = stock_s3[0x401F7D40 - MAIN_BASE : 0x401F7D40 - MAIN_BASE + 20]

    names_hex = ", ".join(f"0x{b:02x}" for b in stock_names)
    attrs_hex = ", ".join(f"0x{b:02x}" for b in stock_attrs)

    c0 = SOURCE_ENTRY - MD_MAIN_BASE
    md_code = md_sec0[c0 : SOURCE_CODE_END - MD_MAIN_BASE]
    # Replace the three 32-bit table operands (at offsets 0x26, 0x44, 0xb6) with linker symbols
    o1 = 0x201250 - SOURCE_ENTRY
    o2 = 0x20126E - SOURCE_ENTRY
    o3 = 0x2012E0 - SOURCE_ENTRY
    c_part0 = ", ".join(f"0x{b:02x}" for b in md_code[:o1])
    c_part1 = ", ".join(f"0x{b:02x}" for b in md_code[o1 + 4 : o2])
    c_part2 = ", ".join(f"0x{b:02x}" for b in md_code[o2 + 4 : o3])
    c_part3 = ", ".join(f"0x{b:02x}" for b in md_code[o3 + 4 :])

    # 128 pitch words + 438 decay words = 566 words = 2264 bytes
    md_tables = md_sec0[SOURCE_DATA_BEGIN - MD_MAIN_BASE : SOURCE_DATA_BEGIN - MD_MAIN_BASE + 566 * 4]
    md_tables_hex = ", ".join(f"0x{b:02x}" for b in md_tables)

    label_entries = []
    long_entries = []
    range_entries = []
    range_triples = []
    short_strs = []
    long_strs = []

    for rid, short_lbl, long_lbl, mn, mx, df in TRXBD_SYN_RECORDS:
        label_entries.append(f"    .long {rid}, str_short_{rid}")
        long_entries.append(f"    .long {60 * rid}, str_long_{rid}")
        range_entries.append(f"    .long {rid}, range_{rid}")
        range_triples.append(f"range_{rid}: .long {mn}, {mx}, {df}")
        short_strs.append(f'str_short_{rid}: .asciz "{short_lbl}"')
        long_strs.append(f'str_long_{rid}: .asciz "{long_lbl}"')

    rec_ids = ", ".join(str(r[0]) for r in TRXBD_SYN_RECORDS)
    nl = """
"""

    assembly = f"""
| DigiMD TRX-BD ColdFire V4 patch for Digitone II OS 1.12
| Assembled with m68k-elf-as -mcpu=54455

    .section .boot, "ax"
    .globl boot_entry
boot_entry:
    lea     {RUN_LOAD_VA:#010x}, %a0
    lea     {RUN_VA:#010x}, %a1
    move.l  #__run_words, %d0
1:  move.l  (%a0)+, (%a1)+
    subq.l  #1, %d0
    bne.s   1b
    lea     {BSS_VA:#010x}, %a1
    moveq   #{BSS_WORDS}, %d0
2:  clr.l   (%a1)+
    subq.l  #1, %d0
    bne.s   2b
    jmp     0x400019b0

    .section .run, "ax"
    .globl slot_shim
    .globl canon_arg
    .globl canon_track
    .globl raw_track
    .globl canon_valid
    .globl track_hook
    .globl setter_hook
    .globl is_trxbd
    .globl trxbd_count
    .globl trxbd_page
    .globl trxbd_label
    .globl trxbd_long
    .globl trxbd_icons
    .globl trxbd_range
    .globl trxbd_machine_list
    .globl trxbd_name_table
    .globl trxbd_attr_table
    .globl md_trxbd_cf_blob

| 0x400dc196: param_set_slot_to_id(slot, machine_type, filter_type)
slot_shim:
    move.l  12(%sp), %d0
    movea.l 16(%sp), %a1
    moveq   #{NEW_TYPE}, %d1
    cmp.l   %d0, %d1
    bne.s   1f
    moveq   #{CLONE_TYPE}, %d0
1:  moveq   #100, %d1
    jmp     0x400dc19e

| 0x400c25f2: overview descriptor(type)
canon_arg:
    move.l  8(%sp), %d0
    moveq   #{NEW_TYPE}, %d1
    cmp.l   %d0, %d1
    bne.s   1f
    moveq   #{CLONE_TYPE}, %d0
1:  moveq   #4, %d1
    rts

| 0x4004b816: getMachineType(track) canonicalized (5 -> 1)
canon_track:
    mvs.b   222(%a0), %d0
    moveq   #{NEW_TYPE}, %d1
    cmp.l   %d0, %d1
    bne.s   1f
    moveq   #{CLONE_TYPE}, %d0
1:  jmp     0x4004b81e

| Raw getMachineType(track) returning real sound+0xDE (5 for TRX-BD)
raw_track:
    move.l  %a2, -(%sp)
    movea.l 8(%sp), %a2
    movea.l (%a2), %a0
    move.l  %a2, -(%sp)
    movea.l 40(%a0), %a0
    jsr     (%a0)
    addq.l  #4, %sp
    tst.l   %d0
    beq.s   1f
    movea.l (%a2), %a0
    move.l  %a2, -(%sp)
    movea.l 40(%a0), %a0
    jsr     (%a0)
    addq.l  #4, %sp
    movea.l %d0, %a0
    mvs.b   222(%a0), %d0
    bra.s   2f
1:  moveq   #-1, %d0
2:  movea.l (%sp)+, %a2
    rts

| 0x40036c24: parameter ownership test (isParameterValidForSound)
canon_valid:
    mvs.b   222(%a0), %d3
    moveq   #{NEW_TYPE}, %d0
    cmp.l   %d3, %d0
    bne.s   1f
    moveq   #{CLONE_TYPE}, %d3
1:  movea.l (%sp)+, %a1
    move.l  %d2, -(%sp)
    jmp     (%a1)

| 0x40046c20: getActiveTrack hook -> updates active_sound_ptr in .bss
track_hook:
    movea.l %d0, %a0
    move.l  8(%a0), %d0
    move.l  %d1, -(%sp)
    movea.l 0x800052a0, %a0
    tst.l   %a0
    beq.s   1f
    move.l  %d0, %d1
    andi.l  #15, %d1
    mulu.w  #1163, %d1
    adda.l  %d1, %a0
    lea     52(%a0), %a0
1:  move.l  %a0, {BSS_VA:#010x}
    move.l  (%sp)+, %d1
    rts

| 0x4004cc92: machine setter hook -> updates sound+0xDE and active_sound_ptr
setter_hook:
    movea.l %d0, %a0
    move.b  %d2, 222(%a0)
    move.l  %a0, {BSS_VA:#010x}
    rts

| Helper: d0 = 1 if active sound has machine_type == 5, else 0. Preserves d1, a0.
is_trxbd:
    move.l  %a0, -(%sp)
    move.l  %d1, -(%sp)
    movea.l {BSS_VA:#010x}, %a0
    tst.l   %a0
    bne.s   1f
    movea.l 0x800052a0, %a0
    tst.l   %a0
    beq.s   3f
    moveq   #0, %d1
    move.b  0x42432a6c, %d1
    andi.l  #15, %d1
    mulu.w  #1163, %d1
    adda.l  %d1, %a0
    lea     52(%a0), %a0
1:  mvs.b   222(%a0), %d1
    moveq   #0, %d0
    subq.l  #{NEW_TYPE}, %d1
    bne.s   2f
    moveq   #1, %d0
    bra.s   2f
3:  moveq   #0, %d0
2:  move.l  (%sp)+, %d1
    movea.l (%sp)+, %a0
    rts

| 0x400c2636: SYN page count(type) -> 1 page for TRX-BD
trxbd_count:
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   1f
    addq.l  #4, %sp
    moveq   #1, %d0
    rts
1:  move.l  8(%sp), %d0
    moveq   #{NEW_TYPE}, %d1
    cmp.l   %d0, %d1
    bne.s   2f
    moveq   #{CLONE_TYPE}, %d0
2:  moveq   #4, %d1
    rts

| 0x400c2656: SYN page descriptor(type, n)
trxbd_page:
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   3f
    move.l  16(%sp), %d0
    tst.l   %d0
    beq.s   1f
    move.l  #0x42433bd4, %d0
    bra.s   2f
1:  move.l  #trxbd_page_desc, %d0
2:  addq.l  #4, %sp
    move.l  (%sp)+, %d2
    rts
3:  move.l  12(%sp), %d1
    move.l  16(%sp), %d0
    subq.l  #{NEW_TYPE}, %d1
    bne.s   4f
    moveq   #{CLONE_TYPE}, %d1
    rts
4:  addq.l  #{NEW_TYPE}, %d1
    rts

| 0x40064622: SYN knob short label
trxbd_label:
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   9f
    move.l  8(%sp), %d0
    lea     trxbd_labels, %a0
1:  move.l  (%a0)+, %d1
    beq.s   9f
    cmp.l   %d0, %d1
    beq.s   2f
    addq.l  #4, %a0
    bra.s   1b
2:  move.l  (%a0), %d0
    rts
9:  jmp     0x400372da

| 0x400365f2: parameter long name
trxbd_long:
    movea.l 40(%a0,%d1.l), %a1
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   9f
    lea     trxbd_long_names, %a0
1:  move.l  (%a0)+, %d0
    beq.s   9f
    cmp.l   %d0, %d1
    beq.s   2f
    addq.l  #4, %a0
    bra.s   1b
2:  movea.l (%a0), %a1
9:  movea.l (%sp)+, %a0
    move.l  %a1, -(%sp)
    move.l  %d2, -(%sp)
    jmp     (%a0)

| 0x4001821e: skip WaveTone oscillator icons on TRX-BD SYN page
trxbd_icons:
    bsr.w   is_trxbd
    tst.l   %d0
    bne.s   1f
    moveq   #7, %d0
    cmp.l   %d3, %d0
    beq.s   2f
1:  move.l  #0x400182c6, (%sp)
2:  rts

| 0x400dc154: limits_getter(id) -> {{min, max, default}} for TRX-BD controls
trxbd_range:
    move.l  8(%sp), %d1
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   9f
    lea     trxbd_ranges, %a1
1:  move.l  (%a1)+, %d0
    beq.s   9f
    cmp.l   %d0, %d1
    beq.s   2f
    addq.l  #4, %a1
    bra.s   1b
2:  movea.l (%a1), %a1
    move.l  (%a1)+, (%a0)
    move.l  (%a1)+, 4(%a0)
    move.l  (%a1), 8(%a0)
    move.l  %a0, %d0
    addq.l  #4, %sp
    rts
9:  movea.l (%sp)+, %a1
    move.l  %d2, -(%sp)
    move.l  8(%sp), %d1
    jmp     (%a1)

    .balign 4
trxbd_labels:
{nl.join(label_entries)}
    .long   0

trxbd_long_names:
{nl.join(long_entries)}
    .long   0

trxbd_ranges:
{nl.join(range_entries)}
    .long   0

{nl.join(range_triples)}

    .balign 4
trxbd_page_desc:
    .long   page_title_short, page_title_long
    .long   {rec_ids}, 10

{_cow_string("page_title_short", "SYN 1")}
{_cow_string("page_title_long", "TRX-BD")}

{nl.join(short_strs)}
{nl.join(long_strs)}

    .balign 4
trxbd_machine_list:
    .long   0, 2, 1, 3, {NEW_TYPE}, 4

    .balign 4
trxbd_name_table:
    .byte   {names_hex}
    .long   str_mach_long, str_mach_short, 0

    .balign 4
trxbd_attr_table:
    .byte   {attrs_hex}
    .byte   1, 0, 0xff, 0xff

str_mach_long:
    .asciz  "TRX-BD"
str_mach_short:
    .asciz  "TBD"

    .balign 4
    .globl md_trxbd_cf_code
    .globl md_trxbd_cf_tables
md_trxbd_cf_code:
    .byte   {c_part0}
    .long   md_trxbd_cf_tables
    .byte   {c_part1}
    .long   md_trxbd_cf_tables + 0x200
    .byte   {c_part2}
    .long   md_trxbd_cf_tables + 0x200
    .byte   {c_part3}

    .balign 4
md_trxbd_cf_tables:
    .byte   {md_tables_hex}
    .balign 4
"""
    from tools.trxbd_sharc.mono_allocation import augment
    return augment(assembly)


def compile_coldfire_payload(stock_s3: bytes, md_sec0: bytes, work_dir: Path, *, assembly_transform=None, bss_va=BSS_VA) -> tuple[bytes, bytes, dict[str, int]]:
    """Assemble and link .boot and .run for ColdFire V4 (MCF5441x)."""
    asm_src = generate_coldfire_asm(stock_s3, md_sec0)
    if assembly_transform is not None:
        asm_src = assembly_transform(asm_src)
    asm_path = work_dir / "trxbd_cf.s"
    obj_path = work_dir / "trxbd_cf.o"
    elf_path = work_dir / "trxbd_cf.elf"
    ld_path = work_dir / "trxbd_cf.ld"
    asm_path.write_text(asm_src)

    ld_script = f"""
SECTIONS {{
  .boot {BOOT_VA:#010x} : {{
    *(.boot)
  }}
  .run {RUN_VA:#010x} : AT({RUN_LOAD_VA:#010x}) {{
    __run_start = .;
    *(.run)
    . = ALIGN(4);
    __run_end = .;
  }}
  __run_words = (__run_end - __run_start) >> 2;
}}
"""
    ld_path.write_text(ld_script)

    subprocess.run(["m68k-elf-as", "-mcpu=54455", str(asm_path), "-o", str(obj_path)], check=True)
    subprocess.run(["m68k-elf-ld", "-T", str(ld_path), str(obj_path), "-o", str(elf_path)], check=True)

    boot_bin = work_dir / "boot.bin"
    run_bin = work_dir / "run.bin"
    subprocess.run(["m68k-elf-objcopy", "-O", "binary", "--only-section=.boot", str(elf_path), str(boot_bin)], check=True)
    subprocess.run(["m68k-elf-objcopy", "-O", "binary", "--only-section=.run", str(elf_path), str(run_bin)], check=True)

    nm_out = subprocess.run(["m68k-elf-nm", "-n", str(elf_path)], check=True, capture_output=True, text=True).stdout
    syms: dict[str, int] = {}
    for line in nm_out.splitlines():
        parts = line.split()
        if len(parts) >= 3:
            syms[parts[2]] = int(parts[0], 16)

    boot_bytes = boot_bin.read_bytes()
    run_bytes = run_bin.read_bytes()
    if len(boot_bytes) > 0x40:
        raise RuntimeError(f".boot exceeds 64 bytes ({len(boot_bytes)})")
    if len(run_bytes) > bss_va - RUN_VA:
        raise RuntimeError(f".run overlaps .bss ({len(run_bytes)} bytes)")
    return boot_bytes, run_bytes, syms


def patch_section3(stock_s3: bytes, md_path: Path, work_dir: Path, *, assembly_transform=None, dual_engine=False, generic_machines=False, machine_selectors=None) -> tuple[bytes, dict[str, object]]:
    """Verify all 41 guards and apply the TRX-BD ColdFire patch to Section 3."""
    if machine_selectors is not None and not generic_machines:
        raise ValueError('Machine selection requires generic control')
    if generic_machines:
        if assembly_transform is not None:
            raise ValueError('Generic control requires tracked production sources')
        from tools.machines.control import augment, BSS
        assembly_transform = augment if machine_selectors is None else lambda assembly: augment(assembly, machine_selectors)
    elif dual_engine:
        if assembly_transform is not None:
            raise ValueError('Dual control requires tracked production transformation')
        from tools.trxsd.control import augment
        assembly_transform = augment
    last_machine = 53 if generic_machines else 6 if dual_engine else NEW_TYPE
    list_bytes = 28 if dual_engine or generic_machines else 24
    if generic_machines:
        from tools.machines.registry import machines
        list_bytes = 4*(5+(sum(m['native']['enabled'] for m in machines()) if machine_selectors is None else len(machine_selectors)))
    if len(stock_s3) != STOCK_END - MAIN_BASE:
        raise ValueError(f"Stock Section 3 length mismatch: {len(stock_s3)}")
    for va, hex_want, desc in GUARDS_112:
        want = bytes.fromhex(hex_want)
        got = stock_s3[va - MAIN_BASE : va - MAIN_BASE + len(want)]
        if got != want:
            raise ValueError(f"Guard failed at 0x{va:08x} ({desc}): want {want.hex()}, got {got.hex()}")

    md_fw = load_firmware(md_path, MD_DEVICE)
    if generic_machines and md_fw.digest != MD_SHA256:
        raise ValueError('Generic control requires original Machinedrum OS 1.63')
    if assembly_transform is None:
        boot_bytes, run_bytes, syms = compile_coldfire_payload(stock_s3, md_fw.section_bytes(0), work_dir)
    else:
        boot_bytes, run_bytes, syms = compile_coldfire_payload(
            stock_s3, md_fw.section_bytes(0), work_dir, assembly_transform=assembly_transform,
            bss_va=BSS if generic_machines else BSS_VA)

    out = bytearray(stock_s3)
    stock_writes = []

    def write_va(va: int, payload: bytes) -> None:
        off = va - MAIN_BASE
        if generic_machines:
            from tools.trxbd_sharc.mono_allocation import GUARDS as mono_guards
            guarded=[(a,a+len(bytes.fromhex(h))) for a,h,_ in GUARDS_112]
            guarded += [(a,a+len(data)) for a,data,_ in mono_guards]
            if not any(a<=va and va+len(payload)<=b for a,b in guarded):
                raise ValueError(f'Unguarded stock write at {va:#x} ({len(payload)} bytes)')
            stock_writes.append(dict(address=hex(va),bytes=len(payload),before=bytes(out[off:off+len(payload)]).hex(),after=payload.hex()))
        out[off : off + len(payload)] = payload

    # 1. Boot copier hook at 0x40000538 -> jsr boot_entry (0x4030c980)
    write_va(0x40000538, b"\x4e\xb9" + struct.pack(">I", syms["boot_entry"]))

    # 2. MACHINE SEL list (6 entries -> 24 bytes) & group(type)
    list_va = syms["trxbd_machine_list"]
    write_va(0x4004D8E0 + 2, struct.pack(">H", list_bytes))
    write_va(0x4004D900 + 2, struct.pack(">I", list_va + list_bytes))
    write_va(0x4004D906 + 2, struct.pack(">H", list_bytes))
    write_va(0x4004D90A + 2, struct.pack(">I", list_va))
    group_code = bytes.fromhex(
        "202f00046d127203b2806c107204b280670e7206b2806c0c42804e7570014e7570024e7570034e75"
    )
    if generic_machines:
        # The original function is 34 bytes; the old 40-byte inline extension
        # overwrites the live function at 0x40059298. Use an out-of-line body.
        write_va(0x40059274,b'\x4e\xf9'+struct.pack('>I',syms['md_group']))
    else:
        write_va(0x40059274, group_code)

    # 3. 6-row machine name table
    names_va = syms["trxbd_name_table"]
    for bound_va, ptr_va, col_off in (
        (0x400DC496, 0x400DC4A6, 0),
        (0x400DC4BC, 0x400DC4CE, 4),
        (0x400DC4E2, 0x400DC4F4, 8),
    ):
        write_va(bound_va + 1, bytes([last_machine]))
        write_va(ptr_va + 2, struct.pack(">I", names_va + col_off))

    # 4. 6-row machine attribute table
    attrs_va = syms["trxbd_attr_table"]
    write_va(0x400DC2C0 + 1, bytes([last_machine]))
    write_va(0x400DC300 + 1, bytes([last_machine]))
    for ptr_va, col_off in ((0x400DC2CA, 0), (0x400DC2E4, 2), (0x400DC316, 2)):
        write_va(ptr_va + 2, struct.pack(">I", attrs_va + col_off))

    # 5. Stored-sound LOAD bound (0x400dd3ea: moveq #6,d2 -> moveq #7,d2)
    write_va(0x400DD3EA + 1, bytes([last_machine + 2]))

    # 6. param_set_slot_to_id hook (0x400dc196: jmp slot_shim; nop)
    write_va(0x400DC196, b"\x4e\xf9" + struct.pack(">I", syms["slot_shim"]) + b"\x4e\x71")

    # 7. getMachineType(track) canonicalized + 7 identity sites to raw_track
    write_va(0x4004B816, b"\x4e\xf9" + struct.pack(">I", syms["canon_track"]))
    for id_va in (
        0x4005A4AC,
        0x40071912,
        0x4002DB2C,
        0x400D5BD6,
        0x400D68C2,
        0x40071EB6,
        0x40031394,
    ):
        write_va(id_va + 2, struct.pack(">I", syms["raw_track"]))

    # 8. Parameter ownership check (0x40036c24: jsr canon_valid)
    write_va(0x40036C24, b"\x4e\xb9" + struct.pack(">I", syms["canon_valid"]))

    # 9. Active sound tracking hooks (0x40046c20 & 0x4004cc92)
    write_va(0x40046C20, b"\x4e\xb9" + struct.pack(">I", syms["track_hook"]))
    write_va(0x4004CC92, b"\x4e\xb9" + struct.pack(">I", syms["setter_hook"]))

    # 10. SYN overview, page count, page descriptor, short labels, long names, icon skip, and ranges
    if generic_machines:
        write_va(0x400C25F2, b"\x4e\xf9" + struct.pack(">I", syms["md_syn_overview"]))
        write_va(0x400C257C, b"\x4e\xf9" + struct.pack(">I", syms["md_syn_paint"]))
    else:
        write_va(0x400C25F2, b"\x4e\xb9" + struct.pack(">I", syms["canon_arg"]))
    write_va(0x400C2636, b"\x4e\xb9" + struct.pack(">I", syms["trxbd_count"]))
    write_va(0x400C2656, b"\x4e\xb9" + struct.pack(">I", syms["trxbd_page"]) + b"\x4e\x71")
    write_va(0x40064622, b"\x4e\xb9" + struct.pack(">I", syms["trxbd_label"]))
    write_va(0x400365F2, b"\x4e\xb9" + struct.pack(">I", syms["trxbd_long"]))
    write_va(0x4001821E, b"\x4e\xb9" + struct.pack(">I", syms["trxbd_icons"]) + b"\x4e\x71")
    write_va(0x400DC154, b"\x4e\xb9" + struct.pack(">I", syms["trxbd_range"]))

    from tools.trxbd_sharc.mono_allocation import patch as patch_mono
    patch_mono(write_va, stock_s3, syms)

    # Append .boot at 0x4030c980 (padded to 0x40 bytes) + .run at 0x4030c9c0
    out.extend(boot_bytes.ljust(0x40, b"\x00"))
    out.extend(run_bytes)

    meta = {
        "stock_s3_len": len(stock_s3),
        "dual_engine": dual_engine,
        "generic_machines": generic_machines,
        "patched_s3_len": len(out),
        "boot_va": hex(BOOT_VA),
        "run_load_va": hex(RUN_LOAD_VA),
        "run_va": hex(RUN_VA),
        "bss_va": hex(BSS if generic_machines else BSS_VA),
        "boot_bytes": len(boot_bytes),
        "run_bytes": len(run_bytes),
        "symbols": {k: hex(v) for k, v in syms.items()},
        "guarded_stock_writes": stock_writes,
        "sha256": hashlib.sha256(out).hexdigest(),
    }
    return bytes(out), meta


def verify_coldfire_in_unicorn(patched_s3: bytes, syms: dict[str, int], *, expected_syn_title="TRX-BD") -> dict[str, object]:
    """Run Unicorn ColdFire V4E functional tests on the patched Section 3 image."""
    venv_py = ROOT / "out/external/digikit/.venv/bin/python"
    if not venv_py.exists():
        raise RuntimeError(f"Missing digikit virtualenv python at {venv_py}")

    test_script = f"""
import json, struct, sys
from unicorn import Uc, UC_ARCH_M68K, UC_MODE_BIG_ENDIAN
from unicorn.m68k_const import (
    UC_CPU_M68K_CFV4E, UC_M68K_REG_A0, UC_M68K_REG_A7,
    UC_M68K_REG_D0, UC_M68K_REG_D1, UC_M68K_REG_D2, UC_M68K_REG_PC
)

s3 = open({str(
    Path(tempfile.gettempdir()) / "digimd_patched_s3.bin"
)!r}, "rb").read()
syms = {json.dumps(syms)}

uc = Uc(UC_ARCH_M68K, UC_MODE_BIG_ENDIAN)
uc.ctl_set_cpu_model(UC_CPU_M68K_CFV4E)
uc.mem_map(0x40000000, 0x00400000)
uc.mem_map(0x42400000, 0x00100000)
uc.mem_map(0x42C00000, 0x00100000)
uc.mem_map(0x47E00000, 0x00100000)
uc.mem_map(0x80000000, 0x00100000)
uc.mem_write(0x40000400, s3)
# Populate dummy WaveTone slot->row entries at 0x42c65d18 + 40*4 (type 1)
for i in range(40):
    uc.mem_write(0x42C65D18 + (1 * 40 + i) * 4, struct.pack(">I", 238 + (i % 25)))

STOP = 0x40000400
STACK = 0x47EFF000

def call_c(fn_va, *args, a0=None):
    sp = STACK
    for arg in reversed(args):
        sp -= 4
        uc.mem_write(sp, struct.pack(">I", arg & 0xFFFFFFFF))
    sp -= 4
    uc.mem_write(sp, struct.pack(">I", STOP))
    uc.reg_write(UC_M68K_REG_A7, sp)
    if a0 is not None:
        uc.reg_write(UC_M68K_REG_A0, a0)
    uc.emu_start(fn_va, STOP, count=50000)
    return uc.reg_read(UC_M68K_REG_D0)

def read_cstr(va):
    raw = bytes(uc.mem_read(va, 64))
    return raw[:raw.index(0)].decode("ascii")

# 1. Execute boot_entry up to 0x400019b0 to copy .run to 0x47e00000 and zero .bss
uc.reg_write(UC_M68K_REG_A7, STACK)
uc.emu_start(syms["boot_entry"], 0x400019B0, count=50000)

# 2. Verify group(type) at 0x40059274
groups = [call_c(0x40059274, t) for t in range(7)]
assert groups == [1, 1, 1, 1, 2, 3, 3], f"Unexpected groups: {{groups}}"

# 3. Verify machine long and short names (0x400dc496 & 0x400dc4bc)
long_5 = read_cstr(call_c(0x400DC496, 5))
short_5 = read_cstr(call_c(0x400DC4BC, 5))
assert long_5 == "TRX-BD", f"Expected TRX-BD, got {{long_5}}"
assert short_5 == "TBD", f"Expected TBD, got {{short_5}}"

# 4. Verify machine attributes & track permission (0x400dc2c0 & 0x400dc2fe)
attr_5 = call_c(0x400DC2C0, 5)
perm_5 = [call_c(0x400DC2FE, 5, t) for t in range(16)]
assert attr_5 == 1, f"Expected attr 1, got {{attr_5}}"
assert all(p == 1 for p in perm_5), f"Expected all 16 tracks allowed: {{perm_5}}"

# 5. Verify param_set_slot_to_id (0x400dc18e) maps type 5 identically to WaveTone (type 1)
slots_1 = [call_c(0x400DC18E, s, 1, 0) for s in range(25, 65)]
slots_5 = [call_c(0x400DC18E, s, 5, 0) for s in range(25, 65)]
assert slots_5 == slots_1 and any(slots_5), "slot_to_id mismatch between type 5 and WaveTone"

# 6. Set up a fake Kit at 0x80010000 and pointer at 0x800052a0, with track 0 = TRX-BD (5) and track 1 = FM Tone (0)
KIT_ADDR = 0x80010000
uc.mem_write(0x800052A0, struct.pack(">I", KIT_ADDR))
uc.mem_write(KIT_ADDR + 52 + 0 * 1163 + 0xDE, bytes([5]))
uc.mem_write(KIT_ADDR + 52 + 1 * 1163 + 0xDE, bytes([0]))

# Set active_sound_ptr to track 0's Sound*
SOUND0 = KIT_ADDR + 52
SOUND1 = KIT_ADDR + 52 + 1163
uc.mem_write(0x47E02000, struct.pack(">I", SOUND0))

# 7. Verify SYN page count (0x400c2636) and descriptor (0x400c2652) on TRX-BD track
cnt_trx = call_c(0x400C2636, 1)
desc_va = call_c(0x400C2652, 1, 0)
desc_words = struct.unpack(">11I", bytes(uc.mem_read(desc_va, 44)))
assert cnt_trx == 1, f"Expected 1 SYN page for TRX-BD, got {{cnt_trx}}"
assert desc_words[2:10] == (254, 255, 252, 253, 256, 257, 260, 259), f"Unexpected knob records: {{desc_words[2:10]}}"
assert desc_words[10] == 10, f"Expected level record 10, got {{desc_words[10]}}"
assert read_cstr(desc_words[0]) == "SYN 1"
assert read_cstr(desc_words[1]) == {expected_syn_title!r}

# 8. Verify short labels (trxbd_label) and ranges/defaults (0x400dc154) on TRX-BD track
OUT_TRIPLE = 0x47EFE000
got_labels = []
got_defaults = []
Got_max = []
for rid in desc_words[2:10]:
    lbl_ptr = call_c(syms["trxbd_label"], 0, rid)
    got_labels.append(read_cstr(lbl_ptr))
    call_c(0x400DC154, rid, a0=OUT_TRIPLE)
    mn, mx, df = struct.unpack(">iii", bytes(uc.mem_read(OUT_TRIPLE, 12)))
    got_defaults.append(df >> 8)
    Got_max.append(mx >> 8)

assert got_labels == ["PTCH", "DEC", "RAMP", "RDEC", "STRT", "NOIS", "HARM", "CLIP"], f"Bad labels: {{got_labels}}"
assert got_defaults == [64, 64, 0, 0, 0, 0, 0, 0], f"Bad defaults: {{got_defaults}}"
assert Got_max == [127] * 8, f"Bad maxes: {{Got_max}}"

# 9. Switch active_sound_ptr to track 1 (stock FM Tone / WaveTone) and verify stock labels/ranges are untouched!
uc.mem_write(0x47E02000, struct.pack(">I", SOUND1))
stock_lbl_254 = read_cstr(call_c(syms["trxbd_label"], 0, 254))
call_c(0x400DC154, 254, a0=OUT_TRIPLE)
_, _, stock_df_254 = struct.unpack(">iii", bytes(uc.mem_read(OUT_TRIPLE, 12)))
assert stock_lbl_254 == "HOLD", f"Stock label corrupted: {{stock_lbl_254}}"
assert (stock_df_254 >> 8) == 127, f"Stock default corrupted: {{stock_df_254 >> 8}}"

# 10. Execute relocated Machinedrum OS 1.63 ColdFire routine md_trxbd_cf_code natively on MCF5441x (CFV4E)!
CF_PARAMS = 0x47EFE100
CF_OUT = 0x47EFE200
syn_defaults = [64, 64, 0, 0, 0, 0, 0, 0]
uc.mem_write(CF_PARAMS, struct.pack(">8H", *(v << 7 for v in syn_defaults)))
uc.mem_write(CF_OUT, b"\\x00" * 64)
cnt_md = call_c(syms["md_trxbd_cf_code"], CF_OUT, CF_PARAMS)
words_md = [w & 0xFFFFFF for w in struct.unpack(">14I", bytes(uc.mem_read(CF_OUT, 56)))]
want_md = [0x000000, 0x7FF275, 0x000000, 0x000B54, 0x000003, 0x5F29C2, 0x7FFFFF,
           0x800000, 0x1FFFFF, 0xE00000, 0x000000, 0x000000, 0x5F29C2, 0x000000]
assert cnt_md == 14 and words_md == want_md, f"Relocated MD ColdFire mismatch: cnt={{cnt_md}} words={{[hex(x) for x in words_md]}}"

print(json.dumps({{
    "unicorn_coldfire_verified": True,
    "groups": groups,
    "machine_5_names": [long_5, short_5],
    "syn_page_count": cnt_trx,
    "syn_page_title": read_cstr(desc_words[1]),
    "syn_knob_records": list(desc_words[2:10]),
    "syn_labels": got_labels,
    "syn_defaults": got_defaults,
    "syn_max": Got_max,
    "stock_isolation_verified": True,
    "relocated_md_cf_executed_on_cfv4e": True,
    "relocated_md_cf_words_hex": [f"{{w:06x}}" for w in words_md],
}}))
"""
    tmp_s3 = Path(tempfile.gettempdir()) / "digimd_patched_s3.bin"
    tmp_py = Path(tempfile.gettempdir()) / "digimd_verify_cf_uc.py"
    tmp_s3.write_bytes(patched_s3)
    tmp_py.write_text(test_script)
    res = subprocess.run([str(venv_py), str(tmp_py)], capture_output=True, text=True)
    if res.returncode != 0:
        raise RuntimeError(f"Unicorn ColdFire test failed:\\n{res.stdout}\\n{res.stderr}")
    return json.loads(res.stdout.strip())


def pack_and_seal_syx(
    stock_syx_path: Path,
    patched_s3: bytes,
    patched_s7: bytes,
    version_tag: str = "1.12",
) -> tuple[bytes, dict[str, object]]:
    """Pack Section 3 (dense) and Section 7 (aplib) into a signed Digitone II .syx."""
    if str(ELEKLOADER_ROOT) not in sys.path:
        sys.path.insert(0, str(ELEKLOADER_ROOT))
    from elekloader import syx
    from elekloader.codec import aplib, dense, elz, transport
    from elekloader.devices import digitone_mk2

    dev = digitone_mk2.DEVICE
    stock = syx.Syx.load(str(stock_syx_path))
    if stock.sha256 != DNII_SHA256:
        raise ValueError("Stock Digitone II OS 1.12 SHA-256 mismatch")

    # Pack Section 3 with dense (enforcing floor at 0x40200000 stage)
    stored_s3 = syx.pack_main(patched_s3, dev)
    # Verify both in-place depack stages (0x40400000 and 0x40200000)
    depacked_404, gap_404 = syx.inplace_depack(stored_s3, dev, 0x40400000)
    depacked_402, gap_402 = syx.inplace_depack(stored_s3, dev, 0x40200000)
    if depacked_404 != patched_s3 or depacked_402 != patched_s3:
        raise RuntimeError("Section 3 in-place depack mismatch")
    if gap_404 <= 0 or gap_402 <= 0:
        raise RuntimeError(f"Section 3 in-place depack margin <= 0: {gap_404}, {gap_402}")

    # Pack Section 7 with aplib and verify elz.depack_section roundtrip
    stored_s7 = aplib.pack_section(patched_s7)
    if elz.depack_section(stored_s7) != patched_s7:
        raise RuntimeError("Section 7 depack mismatch")

    key = syx.seal_key(stock, dev)
    header = bytearray(stock.header)
    v_bytes = version_tag.encode("ascii")
    if len(v_bytes) != 4:
        raise ValueError("version_tag must be 4 ASCII chars")
    header[0x14:0x18] = v_bytes

    cont = bytearray(stock.container[: stock.data_start])
    cont[: syx.COUNT_OFF] = header
    for i, (sid, _off, _clen, dest) in enumerate(stock.table):
        if sid == 3:
            data = stored_s3
        elif sid == 7:
            data = stored_s7
        else:
            data = stock.stored[sid]
        struct.pack_into(">IIII", cont, syx.TABLE_OFF + syx.ENTRY_SZ * i, sid, len(cont), len(data), dest)
        cont += data
        cont += bytes(-len(cont) % 16)

    cont += hmac.new(key, bytes(cont), hashlib.sha256).digest()
    stream = struct.pack(">II", len(cont), transport.content_checksum(bytes(cont))) + bytes(cont)
    syx_bytes = transport.encode_syx(stream, stock.device_id, stock.framing[0], stock.framing[1])
    # The two framing messages are16 bytes each, not128-byte data packets.
    # len(syx)//128 already excludes them; subtracting two again overstates
    # receive headroom by202 bytes. Count actual independently decoded packets.
    from digimd import decode_container
    decoded_device, decoded_container, packet_count = decode_container(syx_bytes)
    if decoded_device != DNII_DEVICE or decoded_container != bytes(cont):
        raise RuntimeError('Firmware transport roundtrip differs from intended container')
    actual_received = packet_count * 101

    info = {
        "syx_sha256": hashlib.sha256(syx_bytes).hexdigest(),
        "syx_bytes": len(syx_bytes),
        "container_bytes": len(cont),
        "stored_s3_bytes": len(stored_s3),
        "stored_s7_bytes": len(stored_s7),
        "inplace_gap_0x40400000": gap_404,
        "inplace_gap_0x40200000": gap_402,
        "flash_end": hex(dev.flash_at + len(cont)),
        "flash_headroom_bytes": dev.flash_limit - (dev.flash_at + len(cont)),
        "actual_data_packets": packet_count,
        "actual_received_bytes": actual_received,
        "receive_headroom_bytes": dev.receive_max - actual_received,
    }
    return syx_bytes, info


def verified_metadata(decomp_dir: Path, stock_path: Path) -> dict:
    """Bind function annotations to this exact stock image and exported files."""
    provenance = json.loads((decomp_dir / 'provenance.json').read_text())
    stock = load_firmware(stock_path, DNII_DEVICE)
    if (stock.digest != DNII_SHA256 or provenance['stock_sha256'] != stock.digest or
            provenance['section3_sha256'] != hashlib.sha256(stock.section_bytes(3)).hexdigest()):
        raise ValueError('Ghidra metadata does not match stock firmware')
    for filename, key in (('functions.tsv', 'functions_tsv_sha256'), ('xrefs.sqlite', 'xrefs_sqlite_sha256')):
        if hashlib.sha256((decomp_dir / filename).read_bytes()).hexdigest() != provenance[key]:
            raise ValueError(f'Ghidra metadata integrity mismatch: {filename}')
    return provenance


def run_brickcheck(
    syx_path: Path,
    stock_path: Path,
    s3_path: Path,
    s7_path: Path,
    out_dir: Path,
    *,
    emu_boot: bool = False,
    upgrade_watch: bool | None = None,
    dsp_blocks: int = 128,
    warm_snapshot: Path | None = None,
    decomp_dir: Path | None = None,
) -> dict[str, object]:
    """Run dn2-cfw's brickcheck suite against the candidate .syx."""
    venv_py = ROOT / "out/external/digikit/.venv/bin/python"
    bc_dir = DN2_CFW_ROOT / "tools/brickcheck"
    vendor_dir = DN2_CFW_ROOT / "vendor"
    vendor_dir.mkdir(exist_ok=True)
    for name in ("digikit", "elekloader", "selache"):
        link = vendor_dir / name
        if not link.exists():
            link.symlink_to(ROOT / "out/external" / name)

    argv = [
        str(syx_path.resolve()),
        "--stock",
        str(stock_path.resolve()),
        "--intended-main",
        str(s3_path.resolve()),
        "--intended-dsp",
        str(s7_path.resolve()),
        "--dsp-blocks",
        str(dsp_blocks),
        "--out",
        str(out_dir.resolve()),
    ]
    if not emu_boot:
        argv.append("--no-emu-boot")
    if not (emu_boot if upgrade_watch is None else upgrade_watch):
        argv.append("--no-upgrade-watch")
    warm_snapshot = warm_snapshot or ROOT / 'out/firmware/stock_boot/warm.snap'
    if warm_snapshot.is_file():
        argv.extend(["--warm-snap", str(warm_snapshot.resolve())])
    if decomp_dir is None and (ROOT / 'out/decomp/main_os/provenance.json').is_file():
        decomp_dir = ROOT / 'out/decomp/main_os'
    if decomp_dir is not None:
        verified_metadata(decomp_dir, stock_path)
        argv.extend(["--decomp", str(decomp_dir.resolve())])

    runner_code = f"""
import os, struct, sys
sys.path.insert(0, {str(bc_dir)!r})
sys.path.insert(0, {str(ROOT / 'out/external/digikit')!r})
sys.path.insert(0, {str(ROOT / 'out/external/digikit/tools')!r})

sys.path.insert(0, {str(ROOT)!r})
from tools.dn2_isa_adapter import install
from tools.build_dn2_trxbd_firmware import upgrade_watch_complete
install()
if os.environ.get('DIGIMD_UPGRADE_WATCH') == '1':
    # Upstream dt2.container.container retains final SysEx packet padding.
    # The real receiver passes its declared container length to the writer.
    # Stage exactly those bytes so the watch exercises the candidate's length,
    # rather than a larger length invented from transport padding.
    from dt2 import container as writer_container
    from digimd import decode_container
    def exact_writer_container(path):
        with open(path, 'rb') as firmware:
            return decode_container(firmware.read())[1]
    writer_container.container = exact_writer_container
    from emu import live
    original_live_init = live.LiveEmulator.__init__
    def writer_live_init(self, *args, **kwargs):
        # The writer harness drives bare emu_start, not timer steps. GUI idle
        # skipping would stop that run in a wait loop before any programming.
        kwargs['idle_skip'] = False
        original_live_init(self, *args, **kwargs)
    live.LiveEmulator.__init__ = writer_live_init
    original_live_build = live.build
    def writer_live_build(*args, **kwargs):
        # Match the stock warm snapshot's emucheck storage topology.
        kwargs['slc'] = True
        return original_live_build(*args, **kwargs)
    live.build = writer_live_build
    original_live_setup = live.LiveEmulator._setup
    def writer_live_setup(self):
        result = original_live_setup(self)
        machine, profile = result[0], result[4]
        from unicorn import UC_HOOK_CODE
        def enter_writer(uc, pc, size, data):
            # Invoke the bare writer in the SPI mutex owner's task context.
            # A snapshot can park a different task; running there would wait
            # forever for the frozen owner. Keep the mutex and driver intact.
            owner = struct.unpack('>I', uc.mem_read(0x446244b8, 4))[0]
            if owner:
                if profile.current_tcb is None:
                    raise RuntimeError('Cannot resolve current task for writer fixture')
                uc.mem_write(profile.current_tcb, struct.pack('>I', owner))
                print(f'upgrade fixture: invoke writer as SPI owner {{owner:#x}}', flush=True)
        machine.uc.hook_add(UC_HOOK_CODE, enter_writer, begin=0x40139104, end=0x40139104)
        return result
    live.LiveEmulator._setup = writer_live_setup
import brickcheck
import bc_upgrade
bc_upgrade.STOCK_SYX = {str(stock_path.resolve())!r}
original_upgrade_run = bc_upgrade.run
def checked_upgrade_run(*args, **kwargs):
    old_flag = os.environ.get('DIGIMD_UPGRADE_WATCH')
    os.environ['DIGIMD_UPGRADE_WATCH'] = '1'
    try:
        report = original_upgrade_run(*args, **kwargs)
    finally:
        if old_flag is None:
            os.environ.pop('DIGIMD_UPGRADE_WATCH', None)
        else:
            os.environ['DIGIMD_UPGRADE_WATCH'] = old_flag
    report['coverage_complete'] = upgrade_watch_complete(report)
    if not report['coverage_complete'] and not report.get('error'):
        report['error'] = 'upgrade writer did not reach program completion with observed erase/program wire transactions'
    return report
bc_upgrade.run = checked_upgrade_run
if {upgrade_watch is True and not emu_boot!r}:
    # Upstream couples check 12 to the long GUI boot check even though the
    # writer uses an independent stock snapshot. Run its original assertions
    # when explicitly requested; check 8 remains an honest SKIP.
    original_checks = brickcheck.Run.go
    def checks_with_independent_upgrade(self):
        original_checks(self)
        self.checks[12] = brickcheck.Check(12)
        self.timed(12, self.c12)
    brickcheck.Run.go = checks_with_independent_upgrade
sys.exit(brickcheck.main({argv!r}))
"""
    # Check 8 starts fresh Python processes for both the boot ladder and
    # emucheck. Inherit the identical ISA adapter there, too, without editing
    # the vendored emulator or altering any of brickcheck's assertions.
    with tempfile.TemporaryDirectory(prefix="digimd-brickcheck-") as tmp:
        runner_py = Path(tmp) / "run_brickcheck.py"
        runner_py.write_text(runner_code)
        (Path(tmp) / "sitecustomize.py").write_text(runner_code.split("import brickcheck\n")[0])
        env = dict(os.environ)
        env["PYTHONPATH"] = os.pathsep.join(filter(None, (tmp, env.get("PYTHONPATH"))))
        rep_path = out_dir / "report.json"
        rep_path.unlink(missing_ok=True)
        result = subprocess.run([str(venv_py), "-u", str(runner_py)],
                                cwd=str(DN2_CFW_ROOT), env=env, check=False)
    rep_path = out_dir / "report.json"
    if not rep_path.exists():
        raise RuntimeError(f"brickcheck did not produce report.json (exit {result.returncode})")
    report = json.loads(rep_path.read_text())
    if result.returncode not in (0, 3) or report.get("verdict") not in ("accept", "incomplete"):
        raise RuntimeError(f"brickcheck refused the candidate (exit {result.returncode}); see {rep_path}")
    return report


def upgrade_watch_complete(report: dict) -> bool:
    """Reject vacuous flash-watch acceptance after an early emulator stop."""
    verdict = report.get('verdict', {})
    return (report.get('stopped_at') == '0x401391c8'
            and verdict.get('program_count', 0) > 0
            and verdict.get('erase_count', 0) > 0
            and any(int(t['cmd'], 16) in (0x02, 0x12) for t in report.get('txns', []))
            and any(int(t['cmd'], 16) in (0x20, 0x21, 0x52, 0xD8, 0xDC) for t in report.get('txns', [])))


def prepare_stock_upgrade_snapshot(stock_path: Path) -> Path:
    """Create brickcheck 12's stock warm snapshot locally, without hardware."""
    if hashlib.sha256(stock_path.read_bytes()).hexdigest() != DNII_SHA256:
        raise ValueError("Stock Digitone II OS 1.12 SHA-256 mismatch")
    out = ROOT / 'out/firmware/stock_boot'
    out.mkdir(parents=True, exist_ok=True)
    sections = ROOT / 'out/external/digikit/out/sections/dn2-1.12'
    bootstrap = f"""
import sys
sys.path.insert(0, {str(ROOT / 'out/external/digikit')!r})
sys.path.insert(0, {str(ROOT / 'out/external/digikit/tools')!r})
sys.path.insert(0, {str(ROOT)!r})
from tools.dn2_isa_adapter import install
install()
"""
    code = bootstrap + f"""
import os
from emu import extract, checkpoint
from emu.longrun import spin
from emucheck import setup
extract.extract({str(stock_path.resolve())!r}, {str(sections)!r})
os.environ['DT2_SECTIONS'] = {str(sections)!r}
checkpoint.make([400_000_000], {str(out / 'cold')!r}, {str(stock_path.resolve())!r})
m, ev, st, pc, inq, at, timers, profile = setup({str(out / 'cold400M.snap')!r}, {str(stock_path.resolve())!r})
pc, done, stop = spin(m, pc, 400_000_000, chunk=10_000_000, pits=timers)
if stop != 'limit':
    raise RuntimeError(f'Stock warming stopped early: {{stop}}')
checkpoint.save_longrun(m, ev, timers, {str(out / 'warm.snap.tmp')!r}, extra={{'n': 400_000_000 + done}})
os.replace({str(out / 'warm.snap.tmp')!r}, {str(out / 'warm.snap')!r})
"""
    subprocess.run([str(ROOT / 'out/external/digikit/.venv/bin/python'), '-u', '-c', code], check=True)
    return out / 'warm.snap'


def build_firmware(
    md_path: Path,
    dn2_path: Path,
    out_dir: Path,
    *,
    run_emu_boot: bool = False,
    warm_snapshot: Path | None = None,
    decomp_dir: Path | None = None,
    diagnostic_silence: bool = False,
    dual_engine: bool = False,
) -> dict[str, object]:
    """End-to-end build and verification of out/firmware/digimd-trxbd-dn2-1.12.syx."""
    if dual_engine and diagnostic_silence:
        raise ValueError('Dual production engines cannot use the diagnostic renderer')
    md_fw = load_firmware(md_path, MD_DEVICE)
    dn2_fw = load_firmware(dn2_path, DNII_DEVICE)
    if md_fw.digest != MD_SHA256:
        raise ValueError("Machinedrum OS 1.63 SHA-256 mismatch")
    if dn2_fw.digest != DNII_SHA256:
        raise ValueError("Digitone II OS 1.12 SHA-256 mismatch")

    if diagnostic_silence and out_dir.exists():
        raise ValueError('Diagnostic output must be a new directory; preserve previous artifacts')
    out_dir.mkdir(parents=True, exist_ok=True)
    # A failed rebuild must not leave a prior candidate's success manifest.
    (out_dir / 'manifest.json').unlink(missing_ok=True)
    work_dir = out_dir / "build"
    work_dir.mkdir(parents=True, exist_ok=True)

    # 1. Build Section 7 (SHARC+ DSP)
    sharc_dir = out_dir / 'sharc-build' if diagnostic_silence or dual_engine else ROOT / "out/trxbd_sharc"
    sharc_dir.mkdir(parents=True, exist_ok=True)
    s7_report = (build_section7(dn2_path, sharc_dir, diagnostic_silence=True)
                 if diagnostic_silence else build_section7(dn2_path, sharc_dir, dual_engine=True)
                 if dual_engine else build_section7(dn2_path, sharc_dir))
    s7_bytes = (sharc_dir / "section7_trxbd.bin").read_bytes()
    s7_path = out_dir / "section7_trxbd.bin"
    s7_path.write_bytes(s7_bytes)

    # 2. Build Section 3 (ColdFire Main OS 1.12)
    stock_s3 = dn2_fw.section_bytes(3)
    patched_s3, s3_meta = (patch_section3(stock_s3, md_path, work_dir, dual_engine=True)
                           if dual_engine else patch_section3(stock_s3, md_path, work_dir))
    s3_path = out_dir / "section3_main_os_trxbd.bin"
    s3_path.write_bytes(patched_s3)

    # 3. Verify ColdFire Section 3 in Unicorn
    uc_report = verify_coldfire_in_unicorn(
        patched_s3,
        {k: int(v, 16) if isinstance(v, str) else v for k, v in s3_meta["symbols"].items()},
    )

    # 4. Pack and HMAC-seal the .syx image
    syx_bytes, syx_info = pack_and_seal_syx(dn2_path, patched_s3, s7_bytes)
    syx_path = out_dir / ("DIAGNOSTIC-silence-custom-dn2-1.12.syx" if diagnostic_silence
                          else "EXPERIMENTAL-digimd-trxbd-trxsd-dn2-1.12.syx" if dual_engine
                          else "digimd-trxbd-dn2-1.12.syx")
    syx_path.write_bytes(syx_bytes)

    # 5. Run brickcheck
    bc_dir = out_dir / "brickcheck"
    bc_report = run_brickcheck(syx_path, dn2_path, s3_path, s7_path, bc_dir,
                              emu_boot=run_emu_boot, warm_snapshot=warm_snapshot, decomp_dir=decomp_dir)

    if decomp_dir is None and (ROOT / 'out/decomp/main_os/provenance.json').is_file():
        decomp_dir = ROOT / 'out/decomp/main_os'
    manifest = {
        "check10_metadata": verified_metadata(decomp_dir, dn2_path) if decomp_dir is not None else None,
        "schema": "digimd.dn2-trxbd-firmware.v1",
        "diagnostic_only": diagnostic_silence,
        "dual_engine": dual_engine,
        "experimental": dual_engine,
        "diagnostic_mode": "silence-custom-renderer" if diagnostic_silence else None,
        "source_commit": source_revision(),
        "source_dirty": None,
        "builder_sha256": hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        "mono_allocation_source_sha256": hashlib.sha256(
            (ROOT/'tools/trxbd_sharc/mono_allocation.py').read_bytes()).hexdigest(),
        "artifact": str(syx_path),
        "source_firmware_sha256": {"machinedrum_1.63": md_fw.digest, "digitone_ii_1.12": dn2_fw.digest},
        "emulated_cold_boot_requested": run_emu_boot,
        "hardware_tested": False,
        "syx_info": syx_info,
        "section3_coldfire": s3_meta,
        "unicorn_coldfire_verification": uc_report,
        "section7_sharc": s7_report,
        "brickcheck_summary": {
            "verdict": bc_report.get("verdict"),
            "checks": {
                k: {"status": v.get("status"), "summary": v.get("summary") or v.get("name")}
                for k, v in bc_report.get("checks", {}).items()
            },
        },
    }
    (out_dir / "manifest.json").write_text(json.dumps(manifest, indent=2))
    return manifest


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--md", type=Path, default=ROOT / "firmware/Elektron_SPS1-1UW_OS1.63.syx")
    ap.add_argument("--dn2", type=Path, default=ROOT / "firmware/Digitone_II_OS1.12.syx")
    ap.add_argument("--out-dir", type=Path, default=ROOT / "out/firmware")
    ap.add_argument("--emu-boot", action="store_true", help="Run full 1000M-instruction emulated cold boot in brickcheck")
    ap.add_argument("--prepare-upgrade-snapshot", action="store_true", help="First create the stock 800M-instruction snapshot for brickcheck 12")
    ap.add_argument("--warm-snap", type=Path, help="Existing stock warm snapshot for brickcheck 12")
    ap.add_argument("--decomp", type=Path, help="Ghidra function metadata directory for brickcheck 10")
    ap.add_argument("--diagnostic-silence", action="store_true",
                    help="DIAGNOSTIC ONLY: retain control/init/hook but zero custom buffers; requires a new --out-dir")
    ap.add_argument("--dual-engine", action="store_true", help="EXPERIMENTAL authentic TRX-BD plus TRX-SD")
    args = ap.parse_args()

    warm_snapshot = prepare_stock_upgrade_snapshot(args.dn2) if args.prepare_upgrade_snapshot else args.warm_snap
    manifest = build_firmware(args.md, args.dn2, args.out_dir, run_emu_boot=args.emu_boot,
                              warm_snapshot=warm_snapshot, decomp_dir=args.decomp,
                              diagnostic_silence=args.diagnostic_silence, dual_engine=args.dual_engine)
    print("FIRMWARE BUILT:", manifest["artifact"])
    print("SYX SHA256:", manifest["syx_info"]["syx_sha256"])
    print("BRICKCHECK VERDICT:", manifest["brickcheck_summary"]["verdict"])
    for cid, cinfo in manifest["brickcheck_summary"]["checks"].items():
        print(f"  brickcheck {cid}: {cinfo['status']} — {cinfo['summary']}")
    return 0 if manifest["brickcheck_summary"]["verdict"] == "accept" else 3


if __name__ == "__main__":
    raise SystemExit(main())
