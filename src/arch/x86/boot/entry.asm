[BITS 32]
global _start

extern kmain

section .text

_start:
    push ebx       ; push multiboot_info pointer
    push eax       ; push magic
    call kmain
    add esp, 8
    cli
    hlt
u_stupid:
    jmp u_stupid