[BITS 32]
%define MOUSE_FLAG_ENABLED  1 << 7

global irq12_handler

extern mouse_irq_handler
extern Redraw_Mouse_Cursor

;vars
extern mouse_buttons

section .text
    irq12_handler:
        cli

        pushad
        pushfd
        
        call mouse_irq_handler

        mov al, [mouse_buttons]
        test al, MOUSE_FLAG_ENABLED
        jz no_mouse_handle

        call Redraw_Mouse_Cursor
    no_mouse_handle:

        

        popfd
        popad
        sti
        iretd
    