global irq0_handler
extern timer_irq

section .text
irq0_handler:
    cli

    pusha
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10        ; Kernel data segment
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    call timer_irq

    pop gs
    pop fs
    pop es
    pop ds
    popa

    ; Send End of Interrupt (EOI) to PIC
    mov al, 0x20
    out 0x20, al

    sti
    iretd
