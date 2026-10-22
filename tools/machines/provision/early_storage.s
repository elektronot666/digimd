.section .text
.global md_early_storage
.global md_early_storage_return
md_early_storage:
    lea -4(%sp),%sp
    move.l #0,(%sp)
    lea -64(%sp),%sp
    movem.l %d0-%d7/%a0-%a6,(%sp)
    move.w %sr,%d0
    move.l %d0,60(%sp)
    jsr 0x4012be52
md_early_storage_return:
    move.l 60(%sp),%d0
    move.w %d0,%sr
    movem.l (%sp),%d0-%d7/%a0-%a6
    lea 64(%sp),%sp
    jsr 0x400cf3f4
    jmp 0x400cf758
