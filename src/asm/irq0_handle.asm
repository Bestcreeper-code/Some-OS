%define PTE_NAME       0 ; PTE = Process Table Entry
%define PTE_BASE       4
%define PTE_STACK_BASE 8
%define PTE_STACK_END  12
%define PTE_SIZE       16

%define PTE_SIZE_TOTAL 20 
%define PT_INDEX       0x2579;1byte
%define PT_BASE        0x257A;16 entries
%define KERNEL_STACK_BASE_ADDRESS      0x2571
%define KERNEL_STACK_POINTER_ADDRESS   0x2575

%define TASK_SWITCHING_FLAG            0x223B
global irq0_handler
extern timer_irq 

section .data
    counter  db 0 
section .text
    irq0_handler:
        cli

        pushad
        pushfd


        call timer_irq

    ;     cmp byte [TASK_SWITCHING_FLAG],1
    ;     jne no_switch
    ;     ; set the current process index to edx
    ;     movzx eax, byte [PT_INDEX]
    ;     mov ebx, PTE_SIZE_TOTAL
    ;     mul ebx ;Index*entry_size result is in edx:eax 
    ;     add eax, PT_BASE; Index*entry_size + PT_base


    ;     cmp byte [counter],0
    ;     je from_kernel
    ;     jne to_kernel
        

    ; from_kernel:
    ;     ;saving stack
    ;     mov dword [KERNEL_STACK_POINTER_ADDRESS],esp
    ;     mov dword [KERNEL_STACK_BASE_ADDRESS],ebp

    ;     cmp dword [eax + PTE_NAME], 0
    ;     je no_switch

    ;     ;setting the regs to change stack
    ;     mov ebx, dword [eax + PTE_STACK_END]
    ;     mov ecx, dword [eax + PTE_STACK_BASE]


    ;     jmp change_stack
    ; to_kernel:
    ;     ;saving stack
    ;     mov dword [eax + PTE_STACK_END], esp
    ;     mov dword [eax + PTE_STACK_BASE], ebp
    ;     ;setting the regs to change stackA
    ;     mov ebx, dword [KERNEL_STACK_POINTER_ADDRESS]
    ;     mov ecx, [KERNEL_STACK_BASE_ADDRESS]
 

    ; change_stack:

    ;     mov ebp, ecx
    ;     mov esp, ebx
    ;     xor byte [counter], 1
    ; no_switch:
    ;     ; Send End of Interrupt (EOI) to PIC
    ;scrapped on this branch for now
        mov al, 0x20
        out 0x20, al

        popfd
        popad
        sti
        iretd
    