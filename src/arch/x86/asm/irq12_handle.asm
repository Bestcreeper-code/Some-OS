[BITS 32]
%define MOUSE_FLAGS_ADDR    0x21B6
%define MOUSE_FLAG_ENABLED  1 << 7

global irq12_handler

extern mouse_irq_handler
extern Redraw_Mouse_Cursor

section .text
    irq12_handler:
        cli

        pushad
        pushfd
        
        call mouse_irq_handler

        mov al, [MOUSE_FLAGS_ADDR]
        test al, MOUSE_FLAG_ENABLED
        jz no_mouse_handle

        call Redraw_Mouse_Cursor
    no_mouse_handle:

        ; Send End of Interrupt (EOI) to PIC
        mov al, 0x20
        out 0xA0, al    ; EOI to slave PIC
        out 0x20, al    ; EOI to master PIC


        popfd
        popad
        sti
        iretd
    