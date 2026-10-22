.text
.globl md_install_rx_hook
md_install_rx_hook:
    lea -24(%sp),%sp
    movem.l %d0-%d1/%a0-%a1,(%sp)
    move.w %ccr,%d0
    move.l %d0,16(%sp)
    move.l %d2,-(%sp)
    move.l %d1,-(%sp)
    move.l %a1,-(%sp)
    jsr md_install_receive
    lea 12(%sp),%sp
    tst.l %d0
    beq.s .L_stock
    move.l #0x40121a10,%d0
    move.l %d0,24(%sp)
.L_stock:
    move.l 16(%sp),%d0
    move.w %d0,%ccr
    movem.l (%sp),%d0-%d1/%a0-%a1
    lea 24(%sp),%sp
    lea 0x40207f9e,%a3
    rts

.globl md_install_try_enter
md_install_try_enter:
    move.w %sr,%d1
    move.w #0x2700,%sr
    tst.l md_install_busy
    bne.s .L_busy
    moveq #1,%d0
    move.l %d0,md_install_busy
    bra.s .L_locked
.L_busy:
    moveq #0,%d0
.L_locked:
    move.w %d1,%sr
    rts
.globl md_install_leave
md_install_leave:
    clr.l md_install_busy
    rts
.bss
.balign 4
.globl md_install_busy
md_install_busy:
    .space 4
.text
.globl md_install_service_hook
md_install_service_hook:
    lea -64(%sp),%sp
    movem.l %d0-%d7/%a0-%a6,(%sp)
    move.w %sr,%d0
    move.l %d0,60(%sp)
    jsr md_install_service
    move.l 60(%sp),%d0
    move.w %d0,%sr
    movem.l (%sp),%d0-%d7/%a0-%a6
    lea 64(%sp),%sp
    jmp 0x40001928
