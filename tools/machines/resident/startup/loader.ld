/* Cold-boot loader: one flat image copied to 0x47E20000 at reset.
 * Everything is initialized data (built with -fno-zero-initialized-in-bss),
 * so the copier needs no separate BSS clear. */
ENTRY(md_boot_entry)
SECTIONS {
  . = 0x47E20000;
  .text : { KEEP(*(.text)) *(.text.*) }
  .rodata : { *(.rodata .rodata.*) }
  .data : { *(.data .data.*) *(.bss .bss.*) *(COMMON) }
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
