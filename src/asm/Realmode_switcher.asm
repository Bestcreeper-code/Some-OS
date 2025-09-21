%define KERNEL_STACK_BASE_ADDRESS    0x2571
%define KERNEL_STACK_POINTER_ADDRESS 0x2575
%define Setup_Base 0x600

section .text

global Realmode_func_runner
extern memmove   ; (void *dest, const void *src, size_t n)

global switch_handler_start
global switch_handler_leave16
global switch_handler_leave32
global switch_handler_end



; ============================================================
;  Real-mode trampoline (runs in 16-bit mode)
; ============================================================


switch_handler_start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00

    push bx             ; save protected ESP low
    push cx             ; save protected ESP high
    push bp
    mov  bp, sp
    ; execution now falls directly into the copied realmode function

; --- when the copied real-mode function RETs, we land here ---
switch_handler_leave16:
    mov sp, bp 
    pop bp
    pop cx
    pop bx

    cli

    ; switch to 32-bit handler code
    jmp switch_handler_leave32


; ============================================================
;  Protected-mode resume (32-bit code)
; ============================================================


switch_handler_leave32:
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    jmp 0x08:protected_mode_resume_point

switch_handler_end:



; ============================================================
;  Launcher from protected mode
; ============================================================


Realmode_func_runner:
    pushad                 ; save all general-purpose registers
    push ebp
    mov ebp, esp

    cli                    ; disable interrupts
    
    ; save current protected-mode stack pointer
    mov [KERNEL_STACK_BASE_ADDRESS], ebp
    mov [KERNEL_STACK_POINTER_ADDRESS], esp

    push protected_mode_resume_point

    ; disable PE bit to switch to real mode
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax

    ; jump to real-mode trampoline (copied at Setup_Base)
    jmp 0x0000:Setup_Base

protected_mode_resume_point:
    mov ebp, [KERNEL_STACK_BASE_ADDRESS]
    mov esp, [KERNEL_STACK_POINTER_ADDRESS]
    sti                     ; enable interrupts
    leave
    popad
    ret

