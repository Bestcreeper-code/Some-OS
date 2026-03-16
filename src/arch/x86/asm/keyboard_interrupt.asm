[BITS 32]
    %define CTRL_KEY_COMBO 159

    ; 256 chars
    extern input_char_buffer

    global irq1_handler ; PS/2 keyboard 

    extern GetInputCharNonBlocking

    extern kernel_data_ptr

section .text
    irq1_handler:

        cli                         ; Disable interrupts
        
        pushad                      ; Save all general-purpose registers
        pushfd                      ; Save the flags register

        ; Call the GetInputCharNonBlocking function (returns in AL)
        call GetInputCharNonBlocking

        cmp al, 0
        je no_add
        
        mov ecx, 0
        
        mov edi, input_char_buffer
        



    loop1:
    
        cmp ecx, 256
        je no_add
        cmp BYTE [edi + ecx], 0
        je end_of_loop1

        inc ecx
        jmp loop1

    end_of_loop1:
        ; Store the character in the input buffer
        mov BYTE [edi + ecx], al
        

    no_add:
        ; Restore the flags and registers
        popfd
        popad

        sti                         ; Re-enable interrupts

        ; Send End Of Interrupt (EOI) to PIC
        mov al, 0x20
        out 0x20, al

        iretd                       ; Return from interrupt
