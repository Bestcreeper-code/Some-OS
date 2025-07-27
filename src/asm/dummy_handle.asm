global irq_dummy_handler

section .text
irq_dummy_handler:
    cli             ; Disable interrupts
    pusha           ; Push all general-purpose registers

    ; --- Send End of Interrupt (EOI) to PIC(s) ---
    mov al, 0x20
    out 0xA0, al    ; Slave PIC (just in case)
    out 0x20, al    ; Master PIC

    popa            ; Restore registers
    sti             ; Re-enable interrupts
    iretd           ; Return from interrupt
