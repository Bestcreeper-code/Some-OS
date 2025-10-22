global _syscall_int_80_handler
extern syscall_handler


section .text
_syscall_int_80_handler:
    push ebx          ; preserve callee-saved registers

    push edi        ; arg5 and also saved regs
    push esi        ; arg4 and also saved regs
    

    push edx        ; arg3
    push ecx        ; arg2
    push ebx        ; arg1
    push eax        ; syscall number

    call syscall_handler

    add esp, 16     ; cleanup (only the ea.. since other need preserv)

    pop esi
    pop edi
    pop ebx

    iretd

    