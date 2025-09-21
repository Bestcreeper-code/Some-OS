; Full ISR setup with categorized macros and handlers

global isr0
global isr1
global isr2
global isr3
global isr4
global isr5
global isr6
global isr7
global isr8
global isr9
global isr10
global isr11
global isr12
global isr13
global isr14
global isr15
global isr16
global isr17
global isr18
global isr19
global isr20
global isr21
global isr22
global isr23
global isr24
global isr25
global isr26
global isr27
global isr28
global isr29
global isr30
global isr31

extern itoa    ; int value, char* str, int base (base=10)
extern Load_bin_exe ; (const char* file_path, int argc, char** argv)
; extern handle_noncrash 

section .data
    file_path db "0:/system_core/crashhndl.bin", 0

section .bss
    int_indx_str resb 12
    int_err_code_str resb 12
    argv resd 3        ; char* argv[3]

section .text


%macro ISR_NOCRASH 1
isr%1:
    call handle_noncrash
    iretd
%endmacro


%macro ISR_NOERR 1
isr%1:
    push dword 2147483648  ; dummy error code
    push dword %1          ; interrupt number
    pushad
    call isr_handler
    popad
    add esp, 8
    iretd
%endmacro


%macro ISR_ERR 1
isr%1:
    push dword %1      ; interrupt number
    pushad
    call isr_handler
    popad
    add esp, 8
    iretd
%endmacro


; Non-crashing ISRs
ISR_NOCRASH 3   ; Breakpoint
ISR_NOCRASH 4   ; Overflow
ISR_NOCRASH 7   ; Device Not Available
ISR_NOCRASH 9   ; Coprocessor Segment Overrun
ISR_NOCRASH 15  ; Reserved
ISR_NOCRASH 16  ; x87 FPU Floating Point Error

; ISRs without error code (default)
ISR_NOERR 0    ; Divide by zero
ISR_NOERR 1    ; Debug
ISR_NOERR 2    ; Non-maskable Interrupt (NMI)
ISR_NOERR 5    ; Bound Range Exceeded
ISR_NOERR 6    ; Invalid Opcode
ISR_NOERR 18   ; Machine Check
ISR_NOERR 19   ; SIMD Floating-Point Exception
ISR_NOERR 20   ; Virtualization Exception
ISR_NOERR 21   ; Control Protection Exception
ISR_NOERR 22   ; Reserved
ISR_NOERR 23   ; Reserved
ISR_NOERR 24   ; Reserved
ISR_NOERR 25   ; Reserved
ISR_NOERR 26   ; Reserved
ISR_NOERR 27   ; Reserved
ISR_NOERR 28   ; Reserved
ISR_NOERR 29   ; Reserved
ISR_NOERR 30   ; Security Exception
ISR_NOERR 31   ; Reserved

; ISRs with error code (already pushed)
ISR_ERR 8    ; Double Fault
ISR_ERR 10   ; Invalid TSS
ISR_ERR 11   ; Segment Not Present
ISR_ERR 12   ; Stack-Segment Fault
ISR_ERR 13   ; General Protection Fault
ISR_ERR 14   ; Page Fault
ISR_ERR 17   ; Alignment Check



isr_handler:
    mov eax, [esp + 32]       ; int_index
    mov ebx, [esp + 36]       ; error_code

    ; Convert int_index to string
    push 10                  ; base
    push int_indx_str        ; buffer
    push eax                 ; value
    call itoa
    add esp, 12

    ; Convert error_code to string
    push 10
    push int_err_code_str
    push ebx
    call itoa
    add esp, 12

    ; Prepare argv array
    mov dword [argv], int_indx_str
    mov dword [argv + 4], int_err_code_str

    push argv                ; char** argv
    push 2                   ; argc
    push file_path           ; const char* file_path
    sti                      ; needed for some handler funcs
    call Load_bin_exe
    add esp, 12

    ret


handle_noncrash:; placeholder
    ret
