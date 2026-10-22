.section .text
.global md_boot_entry
.global md_boot_prepared
md_boot_entry:
    lea -4(%sp),%sp
    move.l #0,(%sp)
    lea -64(%sp),%sp
    movem.l %d0-%d7/%a0-%a6,(%sp)
    move.w %sr,%d0
    move.l %d0,60(%sp)
    jsr 0x4012be52
    move.l 8(%sp),%d0
    move.l 12(%sp),%a0
    move.l %d0,-(%sp)
    move.l %a0,-(%sp)
    jsr md_prepare_boot
    lea 8(%sp),%sp
    move.l %d0,12(%sp)
    move.l md_boot_bytes,%d0
    move.l %d0,8(%sp)
md_boot_prepared:
    move.l 60(%sp),%d0
    move.w %d0,%sr
    movem.l (%sp),%d0-%d7/%a0-%a6
    lea 64(%sp),%sp
    jsr 0x400cf3f4
    jmp 0x400cf758
