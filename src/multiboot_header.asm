; multiboot_header.asm
; Required to mark your kernel as Multiboot-compliant so GRUB loads it

section .text
    align 4
    dd 0x1BADB002         ; magic number
    dd 0x00000001         ; flags (bit 0: memory info)
    dd -(0x1BADB002 + 0x00000001) ; checksum

rdtsc 