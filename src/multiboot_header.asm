; multiboot_header.asm
; Required to mark your kernel as Multiboot-compliant so GRUB loads it

section .text
    align 4
    dd 0x1BADB002
    dd 0x05
    dd -(0x1BADB002 + 0x05)

dd 0
dd 0
dd 0
dd 0
dd 0

dd 0
dd 1024
dd 768
dd 32

