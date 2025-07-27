global isr13

section .text
isr13:
    cli
    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

.halt_loop:
    hlt
    jmp .halt_loop

    ; cleanup
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 4     ; remove error code
    iretd
