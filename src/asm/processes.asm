global switch_proc_asm
global load_proc_asm


extern switch_process

section .data
    proc_esp dd 0   

section .text
switch_proc_asm:
    
    push dword [proc_esp]
    call switch_process      ; call function that rets next esp as eax and sets ebx as next ebp
    add esp, 4    ; clean up the argument off the stack in func

    cmp eax,0

    jne load_proc_asm 

    popad                ; Restore registers
    popfd                ; Restore flags
    sti
    iretd


load_proc_asm:
    cli
    mov ebp, ebx
    mov esp, eax

    popad                ; Restore registers
    popfd                ; Restore flags
    sti
    iretd
    