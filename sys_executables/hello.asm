BITS 32
global _start

section .text
_start:
    ; sys_write(1, msg, 5)
    mov  eax, 4
    mov  ebx, 1
    mov  ecx, msg
    mov  edx, 5
    int  0x80

    ; sys_exit(0)
    mov  eax, 1
    xor  ebx, ebx
    int  0x80

    mov eax,[0]

section .data
msg: db "hello", 0