| Replaces c33's reset copier at 0x4030c980 (called from 0x40000538, before
| the OS clears DDR from 0x402fd000). Copies the unchanged c33 runtime, then
| the cold-boot loader stored directly after it in section 3, then clears the
| same 64-byte c33 BSS and continues into the original reset path.
| Must stay within 64 bytes: c33's runtime image starts at 0x4030c9c0.
.section .text
.global md_reset_copier
md_reset_copier:
    lea 0x4030c9c0,%a0
    lea 0x47e00000,%a1
    move.l #C33_LONGS,%d0
1:  move.l (%a0)+,(%a1)+
    subq.l #1,%d0
    bne.s 1b
    lea LOADER_BASE,%a1
    move.l #LOADER_LONGS,%d0
2:  move.l (%a0)+,(%a1)+
    subq.l #1,%d0
    bne.s 2b
    lea 0x47e10000,%a1
    moveq #16,%d0
3:  clr.l (%a1)+
    subq.l #1,%d0
    bne.s 3b
    jmp 0x400019b0
