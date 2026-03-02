[BITS 32]

global irq_dummy_handler

section .text
irq_dummy_handler:
    cli             ; Disable interrupts
    pushad           ; Push all general-purpose registers

    ;EOI
    mov al, 0x20
    out 0xA0, al    ; Slave PIC
    out 0x20, al    ; Master PIC

    popad            ; Restore registers
    sti             ; Re-enable interrupts
    iretd           ; Return from interrupt
