"""Extend the existing guarded ColdFire interface with distinct machine 6.

Transforms generated assembly before assembly/linking. All stock binary guards,
original serialization, parameter slots and native allocation remain in force.
"""
import re
from tools.trxsd.recover import LABELS,DEFAULTS

RECORDS=(254,255,252,253,256,257,260,259)
LONG=('Pitch','Decay','Bump','Bump Envelope','Snap','Tone','Tune','Clip')

def augment(assembly):
    def replace(old,new,count=1):
        nonlocal assembly
        if assembly.count(old)!=count:raise ValueError('Unexpected dual control shape: '+old)
        assembly=assembly.replace(old,new)
    # Canonical parameter ownership is WaveTone for either custom machine.
    pattern=r'(    moveq   #5, %d1\n    cmp.l   %d0, %d1\n)    bne.s   ([12])f\n    moveq   #1, %d0'
    counter=0
    def canon(match):
        nonlocal counter
        counter+=1;label=f'.L_dual_canon_{counter}'
        return match[1]+f'    beq.s   {label}\n    moveq   #6, %d1\n    cmp.l   %d0, %d1\n    bne.s   {match[2]}f\n{label}:\n    moveq   #1, %d0'
    assembly,n=re.subn(pattern,canon,assembly)
    if n!=4:raise ValueError(f'Expected 4 canonical argument/track paths, got {n}')
    replace('    moveq   #5, %d0\n    cmp.l   %d3, %d0\n    bne.s   1f\n    moveq   #1, %d3',
        '    moveq   #5, %d0\n    cmp.l   %d3, %d0\n    beq.s   .L_dual_valid\n    moveq   #6, %d0\n    cmp.l   %d3, %d0\n    bne.s   1f\n.L_dual_valid:\n    moveq   #1, %d3')
    replace('    subq.l  #5, %d1\n    bne.s   2f\n    moveq   #1, %d0\n    bra.s   2f',
        '    subq.l  #5, %d1\n    beq.s   4f\n    subq.l  #1, %d1\n    bne.s   2f\n    moveq   #2, %d0\n    bra.s   2f\n4:  moveq   #1, %d0\n    bra.s   2f')
    replace('    subq.l  #5, %d1\n    bne.s   4f',
            '    subq.l  #5, %d1\n    cmpi.l  #1, %d1\n    bhi.s   4f')
    replace('1:  move.l  #trxbd_page_desc, %d0',
        '1:  bsr.w   is_trxbd\n    cmpi.l  #2, %d0\n    beq.s   5f\n    move.l  #trxbd_page_desc, %d0\n    bra.s   2f\n5:  move.l  #trxsd_page_desc, %d0')
    replace('    move.l  8(%sp), %d0\n    lea     trxbd_labels, %a0',
        '    cmpi.l  #2, %d0\n    beq.s   8f\n    lea     trxbd_labels, %a0\n    bra.s   7f\n8:  lea     trxsd_labels, %a0\n7:  move.l  8(%sp), %d0')
    for name,reg in [('long_names','a0'),('ranges','a1')]:
        replace(f'    lea     trxbd_{name}, %{reg}',
            f'    cmpi.l  #2, %d0\n    beq.s   8f\n    lea     trxbd_{name}, %{reg}\n    bra.s   7f\n8:  lea     trxsd_{name}, %{reg}\n7:')
    # Mandatory MONO and single-slot ownership apply to both actual Sound types.
    pattern=r'    cmpi.l  #5, %d0\n    bne.s   ([13])f'
    counter=0
    def mono(match):
        nonlocal counter
        counter+=1;label=f'.L_dual_mono_{counter}'
        return f'    cmpi.l  #5, %d0\n    beq.s   {label}\n    cmpi.l  #6, %d0\n    bne.s   {match[1]}f\n{label}:'
    assembly,n=re.subn(pattern,mono,assembly)
    if n!=4:raise ValueError(f'Expected 4 mandatory mono Sound checks, got {n}')
    replace('    .long   0, 2, 1, 3, 5, 4','    .long   0, 2, 1, 3, 5, 6, 4')
    replace('    .long   str_mach_long, str_mach_short, 0','    .long   str_mach_long, str_mach_short, 0\n    .long   sd_mach_long, sd_mach_short, 0')
    replace('    .byte   1, 0, 0xff, 0xff','    .byte   1, 0, 0xff, 0xff\n    .byte   1, 0, 0xff, 0xff')
    lines=['\n    .balign 4','trxsd_labels:']
    lines += [f'    .long {rid}, sd_short_{i}' for i,rid in enumerate(RECORDS)]+['    .long 0','trxsd_long_names:']
    lines += [f'    .long {60*rid}, sd_long_{i}' for i,rid in enumerate(RECORDS)]+['    .long 0','trxsd_ranges:']
    lines += [f'    .long {rid}, sd_range_{i}' for i,rid in enumerate(RECORDS)]+['    .long 0']
    lines += [f'sd_range_{i}: .long 0, 32512, {default<<8}' for i,default in enumerate(DEFAULTS)]
    lines += ['trxsd_page_desc:','    .long page_title_short, sd_page_title','    .long '+', '.join(map(str,RECORDS))+', 10','    .balign 4','    .long 6, 6, -1','sd_page_title: .asciz "TRX-SD"','sd_mach_long: .asciz "TRX-SD"','sd_mach_short: .asciz "TSD"']
    for i,(label,long) in enumerate(zip(LABELS,LONG)):
        lines += [f'sd_short_{i}: .asciz "{label}"',f'sd_long_{i}: .asciz "{long}"']
    return assembly+'\n'.join(lines)+'\n'
