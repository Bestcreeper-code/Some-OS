[BITS 32]
global _sched_next_process

extern serial_write_string
extern serial_log_hex
extern _scheduler_current_process ; Linked_PCB_t*
extern _scheduler_first_process   ; Linked_PCB_t*
extern setTss_sp                  ; function to set TSS.esp0


; on/off logging
%define DEBUG_SCHED_LOG 0 



; === PCB structure offsets ===
%define Linked_PCB_pid             0
%define Linked_PCB_state           2
%define Linked_PCB_name            4

; kernel stack
%define Linked_PCB_kstack_top      8
%define Linked_PCB_kstack_bottom  12

; user stack
%define Linked_PCB_ustack_top     16
%define Linked_PCB_ustack_bottom  20

; saved kernel ESP
%define Linked_PCB_k_esp          24

; CR3 (page directory)
%define Linked_PCB_cr3            28

; next PCB pointer
%define Linked_PCB_next           32



%macro LOG_PCB 1
    %if DEBUG_SCHED_LOG
        push msg_switching
        call serial_write_string
        add esp, 4

        push dword [%1 + Linked_PCB_name]
        call serial_write_string
        add esp, 4

        mov ax, [%1 + Linked_PCB_pid]
        push word 0
        push ax
        push msg_pid
        call serial_log_hex
        add esp, 8

        mov eax, [%1 + Linked_PCB_cr3]
        push eax
        push msg_cr3
        call serial_log_hex
        add esp, 8

        mov eax, [%1 + Linked_PCB_k_esp]
        push eax
        push msg_esp
        call serial_log_hex
        add esp, 8

        mov eax, [%1 + Linked_PCB_next]
        push eax
        push msg_next
        call serial_log_hex
        add esp, 8
    %endif
%endmacro





section .data
msg_switching db "Switching process...",0
msg_pid       db "PID",0
msg_cr3       db "CR3",0
msg_esp       db "V_ESP",0
msg_next      db "NEXT PCB phys addr",0
msg_name      db "NAME",0

section .text
_sched_next_process:
    cli                 ; disable interrupts during switch

    ; --- Save current process state ---
    mov esi, [_scheduler_current_process]

    ; save ESP
    mov [esi + Linked_PCB_k_esp], esp

    ; save CR3
    mov eax, cr3
    mov [esi + Linked_PCB_cr3], eax

    ; --- Select next process ---
    mov esi, [esi + Linked_PCB_next]
    cmp esi, 0
    jne .found_next
    mov esi, [_scheduler_first_process]

.found_next:
    
    
LOG_PCB esi

    ; --- Update current process pointer ---
    mov [_scheduler_current_process], esi
    
    ; --- Load new process page directory ---
    mov eax, [esi + Linked_PCB_cr3]
    mov cr3, eax

    ; --- Update TSS.esp0 for the kernel stack of the new process ---
    mov eax, [esi + Linked_PCB_kstack_top]
    push eax
    call setTss_sp
    add esp, 4
    ; --- Restore kernel stack ---
    mov esp, [esi + Linked_PCB_k_esp]



    popfd       ; restore eflags 
    popad
    iretd       ; return from interrupt
