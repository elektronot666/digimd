"""49 identities on the guarded OS 1.12 ColdFire control/storage boundary.

Only verified engines enter MACHINE SEL. All registered identities have original
metadata for persisted/imported Sounds; unavailable synthesis is muted by DSP.
"""
import re
from pathlib import Path
from tools.machines.registry import ROOT, machines

BSS = 0x47e10000
RECORDS = (254, 255, 252, 253, 256, 257, 260, 259)


def augment(assembly, selectors=None):
    def replace(old, new, count=1):
        nonlocal assembly
        if assembly.count(old) != count:
            raise ValueError('Unexpected generic control source: ' + old)
        assembly = assembly.replace(old, new)

    # Parameter ownership is WaveTone; full identity stays in the Sound byte.
    pattern = r'    moveq   #5, %d1\n    cmp.l   %d0, %d1\n    bne.s   ([12])f\n    moveq   #1, %d0'
    def canonical(match):
        target = match[1]
        return (f'    cmpi.l  #4, %d0\n    bls.s   {target}f\n'
                f'    cmpi.l  #53, %d0\n    bhi.s   .L_md_invalid_{target}_@\n'
                f'    moveq   #1, %d0\n    bra.s   {target}f\n'
                f'.L_md_invalid_{target}_@:\n    moveq   #4, %d0')
    serial = [0]
    def unique(match):
        serial[0] += 1
        return canonical(match).replace('@', str(serial[0]))
    assembly, n = re.subn(pattern, unique, assembly)
    if n != 4:
        raise ValueError('Expected four parameter ownership seams')
    replace('    moveq   #5, %d0\n    cmp.l   %d3, %d0\n    bne.s   1f\n    moveq   #1, %d3',
            '    cmpi.l  #4, %d3\n    bls.s   1f\n    cmpi.l  #53, %d3\n    bhi.s   8f\n    moveq   #1, %d3\n    bra.s   1f\n8:  moveq   #4, %d3')
    replace('    subq.l  #5, %d1\n    bne.s   2f\n    moveq   #1, %d0\n    bra.s   2f',
            '    subq.l  #5, %d1\n    cmpi.l  #48, %d1\n    bhi.s   2f\n    move.l  %d1, %d0\n    addq.l  #1, %d0\n    bra.s   2f')
    replace('    subq.l  #5, %d1\n    bne.s   4f',
            '    subq.l  #5, %d1\n    cmpi.l  #48, %d1\n    bhi.s   4f')
    def lookup(table, reg):
        return (f'    subq.l  #1, %d0\n    lsl.l   #2, %d0\n'
                f'    lea     md_{table}_ptrs, %{reg}\n'
                f'    movea.l 0(%{reg},%d0.l), %{reg}')
    replace('1:  move.l  #trxbd_page_desc, %d0',
            '1:  bsr.w   is_trxbd\n' + lookup('page', 'a0') + '\n    move.l  %a0, %d0')
    replace('    move.l  8(%sp), %d0\n    lea     trxbd_labels, %a0',
            lookup('labels','a0') + '\n    move.l  8(%sp), %d0')
    replace('    lea     trxbd_long_names, %a0', lookup('long_names','a0'))
    replace('    lea     trxbd_ranges, %a1', lookup('ranges','a1'))
    # These replace inline instructions, so caller-scratch registers are live.
    # Preserve their actual displaced effects rather than applying a C ABI.
    def body(label, end, replacement):
        nonlocal assembly
        start=assembly.index(label+':\n')
        stop=assembly.index(end,start)
        assembly=assembly[:start]+replacement+'\n'+assembly[stop:]
    body('trxbd_long','\n| 0x4001821e','''trxbd_long:
    lea     -28(%sp), %sp
    movem.l %d0-%d1/%a0-%a1, (%sp)
    move.w  %ccr, %d0
    move.w  %d0, 16(%sp)
    movea.l 40(%a0,%d1.l), %a1
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   .L_md_long_done
'''+lookup('long_names','a0')+'''
.L_md_long_scan:
    move.l  (%a0)+, %d0
    beq.s   .L_md_long_done
    cmp.l   %d0, %d1
    beq.s   .L_md_long_found
    addq.l  #4, %a0
    bra.s   .L_md_long_scan
.L_md_long_found:
    movea.l (%a0), %a1
.L_md_long_done:
    move.l  28(%sp), 20(%sp)
    move.l  %a1, 28(%sp)
    move.w  16(%sp), %d0
    move.w  %d0, %ccr
    move.l  %d2, 24(%sp)
    movem.l (%sp), %d0-%d1/%a0-%a1
    lea     20(%sp), %sp
    rts
''')
    body('trxbd_icons','\n| 0x400dc154','''trxbd_icons:
    move.l  %d1, -(%sp)
    move.w  %ccr, %d1
    move.l  %d1, -(%sp)
    bsr.w   is_trxbd
    tst.l   %d0
    bne.s   .L_md_icons_skip
    moveq   #7, %d0
    cmp.l   %d3, %d0
    bne.s   .L_md_icons_skip
    addq.l  #2, 8(%sp)
    bra.s   .L_md_icons_done
.L_md_icons_skip:
    move.l  #0x400182c6, %d0
    move.l  %d0, 8(%sp)
.L_md_icons_done:
    move.l  (%sp)+, %d1
    move.w  %d1, %ccr
    move.l  (%sp)+, %d1
    moveq   #7, %d0
    cmp.l   %d3, %d0
    rts
''')
    # The overview is the first SYN screen actually opened by the panel. Merely
    # overriding page(type,n) leaves the WaveTone overview/graphics in use.
    # Use the same descriptor for both routes. Record0 is the native empty cell:
    # the common renderer clears it without drawing a parameter or a knob.
    # All valid MD identities use the existing mandatory MONO policy.
    pattern = r'    cmpi.l  #5, %d0\n    bne.s   ([13])f'
    assembly, n = re.subn(pattern, lambda m: '    cmpi.l  #5, %d0\n    blt.s   '+m[1]+'f\n    cmpi.l  #53, %d0\n    bgt.s   '+m[1]+'f', assembly)
    if n != 4:
        raise ValueError('Expected four original MONO guards')
    active = [m['selector'] for m in machines() if m['native']['enabled']] if selectors is None else list(selectors)
    if (len(set(active)) != len(active) or active[:2] != [5, 6] or
            any(type(s) is not int or not 5 <= s <= 53 for s in active)):
        raise ValueError('Invalid compact machine selection')
    replace('    .long   0, 2, 1, 3, 5, 4', '    .long   '+', '.join(map(str,[0,2,1,3,*active,4])))
    replace('    .long   str_mach_long, str_mach_short, 0',
            '\n'.join(f"    .long md_name_{m['selector']}, md_short_{m['selector']}, 0" for m in machines()))
    replace('    .byte   1, 0, 0xff, 0xff', '\n'.join('    .byte 1, 0, 0xff, 0xff' for m in machines()))
    lines = ['\n    .balign 4']
    for table in ('page','labels','long_names','ranges'):
        lines += ['md_'+table+'_ptrs:', '    .long ' + ', '.join('md_'+table+'_'+str(m['selector']) for m in machines())]
    for m in machines():
        s = m['selector']
        lines += ['    .balign 4', f'md_labels_{s}:']
        lines += [f'    .long {rid}, md_label_{s}_{k}' for k,rid in enumerate(RECORDS)] + ['    .long 0', f'md_long_names_{s}:']
        lines += [f'    .long {60*rid}, md_label_{s}_{k}' for k,rid in enumerate(RECORDS)] + ['    .long 0', f'md_ranges_{s}:']
        lines += [f'    .long {rid}, md_range_{s}_{k}' for k,rid in enumerate(RECORDS)] + ['    .long 0']
        lines += [f"md_range_{s}_{k}: .long 0, 32512, {p['default_raw']<<8}" for k,p in enumerate(m['syn'])]
        visible_records = [rid if p['label'] else 0 for rid, p in zip(RECORDS, m['syn'])]
        lines += [f'md_page_{s}:', f'    .long page_title_short, md_page_title_{s}',
                  '    .long '+', '.join(map(str,visible_records))+', 10', '    .balign 4',
                  f"    .long {len(m['name'])}, {len(m['name'])}, -1",
                  f'md_page_title_{s}: .asciz "{m["name"]}"',
                  f'md_name_{s}: .asciz "{m["name"]}"',
                  f'md_short_{s}: .asciz "{m["name"].replace("-", "")[:3]}"']
        lines += [f'md_label_{s}_{k}: .asciz "{p["label"]}"' for k,p in enumerate(m['syn'])]
    # Preserve established short names used in the original dual implementation.
    text = assembly + '\n'.join(lines) + '''
    .balign 2
    .globl md_syn_paint
md_syn_paint:
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   .L_md_paint_stock
    move.l  4(%sp), %d0
    cmpi.l  #252, %d0
    blt.s   .L_md_paint_stock
    cmpi.l  #257, %d0
    ble.s   .L_md_paint_circle
    cmpi.l  #259, %d0
    beq.s   .L_md_paint_circle
    cmpi.l  #260, %d0
    bne.s   .L_md_paint_stock
.L_md_paint_circle:
    | Reuse the original numeric DEC circle's drawing descriptor only.
    | Caller retains the real parameter ID/value; synthesis, labels, limits
    | and defaults continue through their original/custom accessors.
    move.l  #0x42438618, %d0
    rts
.L_md_paint_stock:
    move.l  4(%sp), %d0
    cmpi.l  #321, %d0
    jmp     0x400c2586
    .balign 2
    .globl md_syn_overview
md_syn_overview:
    bsr.w   is_trxbd
    tst.l   %d0
    beq.s   .L_md_overview_stock
    move.l  %a0, -(%sp)
    subq.l  #1, %d0
    lsl.l   #2, %d0
    lea     md_page_ptrs, %a0
    move.l  0(%a0,%d0.l), %d0
    movea.l (%sp)+, %a0
    moveq   #4, %d1
    rts
.L_md_overview_stock:
    moveq   #4, %d1
    move.l  4(%sp), %d0
    jmp     0x400c25f8
    .balign 2
    .globl md_group
md_group:
    move.l  4(%sp), %d0
    bmi.s   1f
    moveq   #3, %d1
    cmp.l   %d0, %d1
    bge.s   2f
    moveq   #4, %d1
    cmp.l   %d0, %d1
    beq.s   3f
    moveq   #53, %d1
    cmp.l   %d0, %d1
    bge.s   4f
1:  moveq   #0, %d0
    rts
2:  moveq   #1, %d0
    rts
3:  moveq   #2, %d0
    rts
4:  moveq   #3, %d0
    rts
'''
    text = text.replace('md_short_5: .asciz "TRX"', 'md_short_5: .asciz "TBD"').replace('md_short_6: .asciz "TRX"', 'md_short_6: .asciz "TSD"')
    return text.replace('0x47e02000', hex(BSS))


def build(out):
    import hashlib, json, shutil
    from digimd import load_firmware, DNII_SHA256
    from tools.build_dn2_trxbd_firmware import patch_section3
    out.mkdir(parents=True, exist_ok=True)
    (out/'meta.json').unlink(missing_ok=True)
    firmware = load_firmware(ROOT / 'firmware/Digitone_II_OS1.12.syx')
    if firmware.digest != DNII_SHA256: raise ValueError('Generic control requires exact Digitone II 1.12')
    stock = firmware.section_bytes(3)
    image, meta = patch_section3(stock, ROOT / 'firmware/Elektron_SPS1-1UW_OS1.63.syx', out, generic_machines=True)
    meta['source_sha256'] = {str(p.relative_to(ROOT)): hashlib.sha256(p.read_bytes()).hexdigest() for p in (Path(__file__), ROOT/'tools/machines/registry.json', ROOT/'tools/build_dn2_trxbd_firmware.py', ROOT/'tools/trxbd_sharc/mono_allocation.py')}
    meta['toolchain_sha256']={name:hashlib.sha256(Path(shutil.which(name)).read_bytes()).hexdigest() for name in ('m68k-elf-as','m68k-elf-ld','m68k-elf-objcopy','m68k-elf-nm')}
    meta['link_inputs_sha256']={name:hashlib.sha256((out/name).read_bytes()).hexdigest() for name in ('trxbd_cf.s','trxbd_cf.ld','trxbd_cf.elf')}
    (out / 'section3.bin').write_bytes(image)
    (out / 'meta.json').write_text(json.dumps(meta, indent=2)+'\n')
    return meta


if __name__ == '__main__':
    from pathlib import Path
    print(build(ROOT / 'out/machines/control')['sha256'])
