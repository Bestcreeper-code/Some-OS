%define PTE_NAME       0;PTE= process table entry
%define PTE_BASE       4
%define PTE_STACK_BASE 8
%define PTE_STACK_END  12
%define PTE_SIZE       16

%define PTE_SIZE_TOTAL 20 

%define KERNEL_STACK_POINTER_ADDRESS 0x2575
%define CTRL_KEY_COMBO 159

global keyboard_handler
extern GetInputCharNonBlocking

section .data
    proc_esp dd 0   

section .text
keyboard_handler:

    cli
    pushad; Save general-purpose registers
    pushfd; Save flags
    
    mov [proc_esp],esp
    mov esp, [KERNEL_STACK_POINTER_ADDRESS]

    call GetInputCharNonBlocking ;returns in al

    cmp al, CTRL_KEY_COMBO + ('p' - 'a')
    je switch_proc

    popad                ; Restore registers
    popfd                ; Restore flags
    sti

    mov al, 0x20        ; Send EOI to PIC
    out 0x20, al
    iretd               ; Return from interrupt

switch_proc:
    sti
    push dword [proc_esp]
    call foo      ; call function
    ; add esp, 4    ; clean up the argument off the stack in func

