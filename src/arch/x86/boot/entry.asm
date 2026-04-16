[BITS 32]
global _start

extern kmain
extern bootloader_asm_entry

section .text

_start:
    call bootloader_asm_entry
    
    call kmain
    add esp, 8
    cli
    hlt
u_stupid:
    jmp u_stupid