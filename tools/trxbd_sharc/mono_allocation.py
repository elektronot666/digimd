"""Machine-specific MONO allocation policy for authentic Machinedrum voices.

No stock Sound, track VoiceConfig, free mask or shared allocator array is
changed. Machine 5 uses the original MONO policy and one physical voice per
allocation. Each Sound is checked directly; active-UI identity is not authority.
"""
import struct

MAIN_BASE=0x40000400
GUARDS=(
    (0x4002a6a2,bytes.fromhex('4fefff8c48d77cfc'),'native allocator prologue'),
    (0x4002a80c,bytes.fromhex('7d3058002e394059db04'),'native unison count and timestamp'),
    (0x401380fe,bytes.fromhex('7201b2ad0146'),'event MONO-priority mode test'),
    (0x4004b6c8,bytes.fromhex('204020280146'),'track play-mode getter'),
    (0x4004b710,bytes.fromhex('2040b4a80146'),'track play-mode setter compare'),
    (0x4002a9da,bytes.fromhex('d3fc40288314b5d1'),'MONO Sound-identity reuse filter'),
)


def augment(assembly):
    # Keep Sound creation/selection and UI queries consistent with the policy.
    for old,new in (
        ('    move.l  %a0, 0x47e02000\n    rts\n\n| Helper:',
         '    bsr.w   trxbd_force_mono\n    move.l  %a0, 0x47e02000\n    rts\n\n| Helper:'),
        ('1:  move.l  %a0, 0x47e02000\n    move.l  (%sp)+, %d1',
         '1:  bsr.w   trxbd_force_mono\n    move.l  %a0, 0x47e02000\n    move.l  (%sp)+, %d1')):
        if assembly.count(old)!=1:raise ValueError('Unexpected Sound hook shape')
        assembly=assembly.replace(old,new)
    return assembly+'''
| a0 = Sound*. Preserve d0/a0, never use the active UI Sound for dispatch.
    .globl trxbd_force_mono
trxbd_force_mono:
    move.l  %d0, -(%sp)
    tst.l   %a0
    beq.s   1f
    mvs.b   222(%a0), %d0
    cmpi.l  #5, %d0
    bne.s   1f
    moveq   #2, %d0
    move.l  %d0, 326(%a0)
1:  move.l  (%sp)+, %d0
    rts

| Replay displaced prologue, then enforce mode even for old saved presets.
    .globl trxbd_mono_allocator
trxbd_mono_allocator:
    lea     -116(%sp), %sp
    movem.l %d2-%d7/%a2-%a6, (%sp)
    move.l  %a0, -(%sp)
    movea.l 156(%sp), %a0
    bsr.w   trxbd_force_mono
    movea.l (%sp)+, %a0
    jmp     0x4002a6aa

| Local result only: custom voices never expand a native unison allocation.
    .globl trxbd_mono_count
trxbd_mono_count:
    mvs.b   0(%a0,%d5.l), %d6
    move.l  %d0, -(%sp)
    move.l  %a0, -(%sp)
    movea.l 164(%sp), %a0
    tst.l   %a0
    beq.s   1f
    mvs.b   222(%a0), %d0
    cmpi.l  #5, %d0
    bne.s   1f
    moveq   #1, %d6
1:  movea.l (%sp)+, %a0
    move.l  (%sp)+, %d0
    move.l  0x4059db04, %d7
    rts

| Preserve the original cmp flags and d1=1 for subsequent event branches.
    .globl trxbd_mono_event
trxbd_mono_event:
    move.l  %a0, -(%sp)
    movea.l %a5, %a0
    bsr.w   trxbd_force_mono
    movea.l (%sp)+, %a0
    moveq   #1, %d1
    cmp.l   326(%a5), %d1
    rts

| The existing setup page must also read/write MONO, never display a working
| POLY choice that the allocation guard would subsequently override.
    .globl trxbd_mono_get
trxbd_mono_get:
    movea.l %d0, %a0
    bsr.w   trxbd_force_mono
    move.l  326(%a0), %d0
    rts

    .globl trxbd_mono_set
trxbd_mono_set:
    movea.l %d0, %a0
    move.l  %d0, -(%sp)
    mvs.b   222(%a0), %d0
    cmpi.l  #5, %d0
    bne.s   1f
    moveq   #2, %d2
1:  move.l  (%sp)+, %d0
    cmp.l   326(%a0), %d2
    rts

| Native MONO normally reuses only voices with the identical Sound pointer.
| A sound lock can supply another object on the same track. For TRX-BD, reuse
| the track's last owned physical voice instead. The stock candidate mask
| supplies same-track rows. A changed reservation must not allocate a second
| synthesis voice while the previous one still belongs to this track. Include
| that existing voice in the LOCAL permission intersection (d5), preserving
| its synthesis state. Shared reservations/free masks are never changed here.
| If last was stolen, the original fallback allocator chooses a new one.
    .globl trxbd_mono_reuse
trxbd_mono_reuse:
    adda.l  #0x40288314, %a1
    move.l  %d0, -(%sp)
    move.l  %a0, -(%sp)
    mvs.b   222(%a2), %d0
    cmpi.l  #5, %d0
    bne.s   3f
    move.l  132(%sp), %d0
    mulu.w  #1036, %d0
    lea     0x4028884d, %a0
    mvs.b   0(%a0,%d0.l), %d0
    cmp.l   %d0, %d3
    beq.s   2f
    movea.l (%sp)+, %a0
    move.l  (%sp)+, %d0
    cmpi.l  #-1, %d3
    rts
2:  bset.l  %d3, %d5
    movea.l (%sp)+, %a0
    move.l  (%sp)+, %d0
    cmpa.l  %a2, %a2
    rts
3:  movea.l (%sp)+, %a0
    move.l  (%sp)+, %d0
    cmpa.l  (%a1), %a2
    rts
'''


def patch(write_va,stock_s3,symbols):
    for address,expected,label in GUARDS:
        off=address-MAIN_BASE
        if stock_s3[off:off+len(expected)]!=expected:
            raise ValueError('Unexpected OS1.12 '+label)
    write_va(0x4002a6a2,b'\x4e\xf9'+struct.pack('>I',symbols['trxbd_mono_allocator'])+b'\x4e\x71')
    write_va(0x4002a80c,b'\x4e\xb9'+struct.pack('>I',symbols['trxbd_mono_count'])+b'\x4e\x71'*2)
    write_va(0x401380fe,b'\x4e\xb9'+struct.pack('>I',symbols['trxbd_mono_event']))
    write_va(0x4004b6c8,b'\x4e\xb9'+struct.pack('>I',symbols['trxbd_mono_get']))
    write_va(0x4004b710,b'\x4e\xb9'+struct.pack('>I',symbols['trxbd_mono_set']))
    write_va(0x4002a9da,b'\x4e\xb9'+struct.pack('>I',symbols['trxbd_mono_reuse'])+b'\x4e\x71')
