[BITS 32]
global _start

extern main

section .text

_start:
    push ebp
    mov ebp, esp

    ; assume the caller pushed argc, argv already
    mov eax, [ebp+8]    ; argc
    mov ecx, [ebp+12]   ; argv

    push ecx            ; argv
    push eax            ; argc
    call main           ; int main(int argc, char **argv)

    mov esp, ebp
    pop ebp
    ret                 ; return to caller

    ;nasm -f elf32 crt0.asm -o crt0.o