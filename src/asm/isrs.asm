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


extern _panic_handler

section .bss
int_indx    resd 1
int_err_code resd 1
argv            resd 4
gp_regs         resd 20
stack_trace     resd 16

section .text

%macro SAVE_REGS 0
    mov esi, gp_regs

    ; General-purpose registers
    mov eax, [esp + 4]      ; EAX
    mov [esi + 0*4], eax

    mov eax, [esp + 16]     ; EBX
    mov [esi + 1*4], eax

    mov eax, [esp + 8]      ; ECX
    mov [esi + 2*4], eax

    mov eax, [esp + 12]     ; EDX
    mov [esi + 3*4], eax

    mov eax, [esp + 28]     ; ESI
    mov [esi + 4*4], eax

    mov eax, [esp + 32]     ; EDI
    mov [esi + 5*4], eax

    mov eax, [esp + 24]     ; EBP
    mov [esi + 6*4], eax

    mov eax, [esp + 20]     ; Original ESP (before pushad)
    mov [esi + 7*4], eax

    ; EIP, EFLAGS
    mov eax, [esp + 40]     ; EIP
    mov [esi + 8*4], eax

    mov eax, [esp + 48]     ; EFLAGS
    mov [esi + 9*4], eax

    ; Segment Registers
    push cs
    pop eax
    mov [esi + 10*4], eax

    push ds
    pop eax
    mov [esi + 11*4], eax

    push es
    pop eax
    mov [esi + 12*4], eax

    push fs
    pop eax
    mov [esi + 13*4], eax

    push gs
    pop eax
    mov [esi + 14*4], eax

    push ss
    pop eax
    mov [esi + 15*4], eax

    ; Control Registers (CR0, CR2, CR3, CR4 only)
    mov eax, cr0
    mov [esi + 16*4], eax

    mov eax, cr2
    mov [esi + 17*4], eax

    mov eax, cr3
    mov [esi + 18*4], eax

    ; mov eax, cr4 ;why tf does it cause a qemu crash
    ; mov [esi + 19*4], eax
%endmacro


%macro ISR_NOCRASH 1
isr%1:
    call handle_noncrash
    iretd
%endmacro

%macro ISR_NOERR 1
isr%1:
    cli
    push dword 2147483648
    push dword %1
    pushad
    SAVE_REGS
    call isr_handler
    popad
    add esp, 8
    iretd
%endmacro

%macro ISR_ERR 1
isr%1:
    cli
    push dword %1
    pushad
    SAVE_REGS
    call isr_handler
    popad
    add esp, 8
    iretd
%endmacro

ISR_NOCRASH 3
ISR_NOCRASH 4
ISR_NOCRASH 7
ISR_NOCRASH 9
ISR_NOCRASH 15
ISR_NOCRASH 16

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

ISR_ERR 8
ISR_ERR 10
ISR_ERR 11
ISR_ERR 12
ISR_ERR 13
ISR_ERR 14
ISR_ERR 17

isr_handler:
    mov dword eax, [esp + 36]       ; int_index
    mov dword ebx, [esp + 40]       ; error_code
    
    mov [int_indx], eax       ; int_index
    mov [int_err_code], ebx       ; error_code

    ; store faulting EIP first
    mov edx, [gp_regs + 8*4]
    mov [stack_trace], edx

    ; walk EBP chain for up to 8 frames
    mov ecx, 1
    mov esi, stack_trace
    mov ebp, [gp_regs + 2*4]  ; original EBP

.trace_loop:
    test ebp, ebp
    jz .trace_done
    mov eax, [ebp + 4]        ; saved return address
    test eax, eax
    jz .trace_done
    mov [esi + ecx*4], eax
    inc ecx
    cmp ecx, 8
    je .trace_done
    mov ebp, [ebp]             ; previous EBP
    jmp .trace_loop

.trace_done:

    mov eax,[int_indx]       ; int_index
    mov ebx,[int_err_code]       ; error_code

    mov dword [argv], eax
    mov dword [argv + 4], ebx
    mov dword [argv + 8], gp_regs
    mov dword [argv + 12], stack_trace

    push argv
    push 4
    
    call _panic_handler
    add esp, 12

    sti

    ret

handle_noncrash:
    ret
