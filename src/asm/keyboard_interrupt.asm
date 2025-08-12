    %define PTE_NAME       0 ; PTE = Process Table Entry
    %define PTE_BASE       4
    %define PTE_STACK_BASE 8
    %define PTE_STACK_END  12
    %define PTE_SIZE       16

    %define PTE_SIZE_TOTAL 20 

    %define KERNEL_STACK_POINTER_ADDRESS 0x2575
    %define CTRL_KEY_COMBO 159

    ; 256 chars
    %define INPUT_CHAR_BUFFER_ADDRESS 0x2241 

    global irq1_handler ; PS/2 keyboard 

    extern GetInputCharNonBlocking
    extern switch_proc_asm

section .data
    proc_esp dd 0    ; To store the saved stack pointer

section .text
    irq1_handler:

        cli                         ; Disable interrupts
        
        pushad                      ; Save all general-purpose registers
        pushfd                      ; Save the flags register

        ; Save the current stack pointer in proc_esp
        mov [proc_esp], esp

        ; Check if we're already at kernel space
        cmp BYTE [0x2579], 0
        je already_at_kernl

        ; Save the kernel stack pointer to KERNEL_STACK_POINTER_ADDRESS
        mov dword esp, [KERNEL_STACK_POINTER_ADDRESS]
        jmp no_krnl

    already_at_kernl:
        ; If already in kernel space, update the kernel stack pointer
        mov [KERNEL_STACK_POINTER_ADDRESS], esp

    no_krnl:
        ; Call the GetInputCharNonBlocking function (returns in AL)
        call GetInputCharNonBlocking

        cmp al, 0
        je no_add

        ; Check for CTRL + P combo to switch processes
        cmp al, CTRL_KEY_COMBO + ('p' - 'a')
        je switch_proc_asm

        mov ecx, 0
    loop1:
        cmp ecx, 256
        je no_add
        cmp BYTE [INPUT_CHAR_BUFFER_ADDRESS + ecx], 0
        je end_of_loop1

        inc ecx
        jmp loop1

    end_of_loop1:
        ; Store the character in the input buffer
        mov BYTE [INPUT_CHAR_BUFFER_ADDRESS + ecx], al

    no_add:
        ; Restore the saved stack pointer (esp) from proc_esp
        mov esp, [proc_esp]

        ; Restore the flags and registers
        popfd
        popad

        sti                         ; Re-enable interrupts

        ; Send End Of Interrupt (EOI) to PIC
        mov al, 0x20
        out 0x20, al

        iretd                       ; Return from interrupt

