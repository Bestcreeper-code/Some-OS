[BITS 32]
%define MOUSE_FLAGS_K_DATA_Off    0x0004+256
%define MOUSE_FLAG_ENABLED      1 << 7


%define TASKSWITCH_ENABLED      1 

;exports
global irq0_handler

;funcs
extern timer_irq 
extern Redraw_Mouse_Cursor
extern _sched_next_process

;vars
extern kernel_data
extern task_switching_flag

section .data
    counter  db 0 
section .text
    irq0_handler:
        cli

        pushad
        ; pushfd

        ;call the timer tick before anything else
        call timer_irq
        
        lea esi, [rel kernel_data]

        ;test for the display of te mouse
        mov al, [esi + MOUSE_FLAGS_K_DATA_Off]       
        test al, MOUSE_FLAG_ENABLED      ; Test bit 7

        jz mouse_display_skip           ;  skip redraw if not set

            call Redraw_Mouse_Cursor
        mouse_display_skip:

        ;test for the taskswitch flag
        mov al, [task_switching_flag]       
        test al, TASKSWITCH_ENABLED      ; Test bit 1

        jz taskswitch_skip           ;  skip task switch if not set
            ; Send End of Interrupt (EOI) to PIC since wont come back if switches
            
            mov al, 0x20
            out 0x20, al
            
            ; before
            ; call _sched_next_process
            ; after
            jmp _sched_next_process
        taskswitch_skip:
        


        ; Send End of Interrupt (EOI) to PIC
        mov al, 0x20
        out 0x20, al

        ; popfd
        popad
        sti
        iretd
    