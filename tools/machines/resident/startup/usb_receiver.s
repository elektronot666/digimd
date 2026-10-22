| Bounded main-OS Transfer receiver guards for Digitone II OS 1.12.
|
| The stock receiver state is 0x20 bytes followed by decoded 101-byte packets.
| build.py redirects the state accessor to USB_RX_BASE and replaces three exact
| instruction sequences with jumps here.  The arena is reserved from T1.3's
| watched-free ColdFire DDR and is disjoint from DigiMD's 0x47e00000 runtime.
|
| USB_RX_MAX_PACKETS is ceil((0x300000 + 8) / 101): enough for the complete
| OS flash window plus the length/checksum prefix and final packet padding.
.section .text
.equ USB_RX_MAX_PACKETS,31146
.equ USB_RX_FLASH_BYTES,0x00300000
.equ USB_RX_BASE,0x46800000

| The stock reset clears DDR only through 0x466b84d0. Initialize the relocated
| receiver after its code has been copied, before continuing the OS reset.
.global md_usb_reset
md_usb_reset:
    lea USB_RX_BASE,%a0
    moveq #8,%d0
1:  clr.l (%a0)+
    subq.l #1,%d0
    bne.s 1b
    jmp 0x400019b0

| The original decoder reads bytes 6/7/126 before proving message length.
| Check the complete stock envelope and exact wire lengths before entering it.
| Only caller-saved d0/d1/a0/a1 are touched before its original prologue.
.global md_usb_decode_guard
md_usb_decode_guard:
    move.l 12(%sp),%d0
    cmpi.l #16,%d0
    bcs.w md_usb_entry_reject
    movea.l 8(%sp),%a0
    move.l (%a0),%d1
    cmpi.l #0xf000203c,%d1
    bne.w md_usb_entry_reject
    moveq #0,%d1
    move.w 4(%a0),%d1
    cmpi.l #0x1500,%d1
    bne.w md_usb_entry_reject
    moveq #0,%d1
    move.b 6(%a0),%d1
    cmpi.l #0x7e,%d1
    beq.s 2f
    cmpi.l #0x7f,%d1
    bne.w md_usb_entry_reject
    cmpi.l #16,%d0
    bne.w md_usb_entry_reject
    bra.s 3f
2:  cmpi.l #128,%d0
    bne.w md_usb_entry_reject
3:  lea 7(%a0),%a1
    subq.l #8,%d0
4:  move.b (%a1)+,%d1
    tst.b %d1
    bmi.w md_usb_entry_reject
    subq.l #1,%d0
    bne.s 4b
    lea -24(%sp),%sp
    movem.l %d2-%d4/%a2-%a4,(%sp)
    jmp 0x40122584
md_usb_entry_reject:
    movea.l 4(%sp),%a0
    clr.b (%a0)
    moveq #6,%d0
    rts

.global md_usb_start_guard
md_usb_start_guard:
    tst.l %d0
    beq.w md_usb_reject
    cmpi.l #USB_RX_MAX_PACKETS,%d0
    bhi.w md_usb_reject
    clr.l 12(%a2)
    move.b #1,(%a2)
    move.l %d0,8(%a2)
    jmp 0x40122694

.global md_usb_write_guard
md_usb_write_guard:
    move.l 12(%a2),%d0
    cmpi.l #USB_RX_MAX_PACKETS,%d0
    bcc.w md_usb_reject
    pea 0x65
    moveq #101,%d1
    jmp 0x40122610

.global md_usb_end_guard
md_usb_end_guard:
    move.l 0x20(%a2),%d0
    cmpi.l #64,%d0
    bcs.w md_usb_reject
    cmpi.l #USB_RX_FLASH_BYTES,%d0
    bhi.w md_usb_reject
    move.l %d0,%d1
    andi.l #3,%d1
    bne.w md_usb_reject
    | Require exactly ceil((container length + 8) / 101) packets. This prevents
    | the unchanged checksum/HMAC routines from consuming stale/unreceived RAM.
    addq.l #8,%d0
    move.l 12(%a2),%d1
    moveq #101,%d2
    muls.l %d2,%d1
    sub.l %d0,%d1
    cmpi.l #100,%d1
    bhi.w md_usb_reject
    move.l 0x28(%a2),%d0
    cmpi.l #0x454c4533,%d0
    bne.w md_usb_reject
    move.b #3,(%a2)
    clr.l %d0
    jmp 0x401226de

md_usb_reject:
    jmp 0x40122654
