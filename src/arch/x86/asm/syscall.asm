[BITS 32]
global _syscall_int_80_handler
extern syscall_handler


section .text
_syscall_int_80_handler:
    push ebx          ; preserve callee-saved registers

    push edi        ; arg5 and also saved regs
    push esi        ; arg4 and also saved regs
    

    pushad

    call syscall_handler

    add esp, 32

    pop esi
    pop edi
    pop ebx

    iretd

    