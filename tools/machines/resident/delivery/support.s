.text
.globl memcpy
memcpy:
    move.l 4(%sp),%d0
    move.l %d0,%a0
    move.l 8(%sp),%a1
    move.l 12(%sp),%d1
    beq.s .L_copy_done
.L_copy:
    move.b (%a1)+,(%a0)+
    subq.l #1,%d1
    bne.s .L_copy
.L_copy_done:
    rts
