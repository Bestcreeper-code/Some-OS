global _sched_next_process
global testing

extern serial_write_string
extern serial_log_hex


; typedef struct Linked_PCB_t {
;     uint16_t pid;
;     char* name;

;     uint32_t esp, ebp;

;     uint8_t state;

;     PD_t* page_dir;

;     struct Linked_PCB_t* next;
; } __attribute__((__packed__)) Linked_PCB_t;

%define Linked_PCB_pid_offset        0
%define Linked_PCB_name_offset       2
%define Linked_PCB_esp_offset        6
%define Linked_PCB_ebp_offset       10
%define Linked_PCB_state_offset     14
%define Linked_PCB_page_dir_offset  15
%define Linked_PCB_next_offset      19

extern _scheduler_current_process ; Linked_PCB_t*

extern _scheduler_first_process   ; Linked_PCB_t*

section .data
msg_switching     db "Switching process...", 0
msg_pid           db "PID", 0
msg_cr3           db "CR3", 0
msg_esp           db "V_ESP", 0
msg_ebp           db "V_EBP", 0
msg_next          db "NEXT PCB phys addr", 0
msg_name          db "NAME", 0
msg_iret db "IRET->EIP", 0


section .text
_sched_next_process:
    ;pushad |
    ;pushfd | by irq0
    
    
    ; push ds
    ; push es
    ; push fs
    ; push gs

    ;get current process PCB
    mov esi, [_scheduler_current_process]

    
    ; save process esp and ebp
    mov [esi + Linked_PCB_esp_offset], esp
    mov [esi + Linked_PCB_ebp_offset], ebp
    ;save cr3(to be sure)
    mov eax, cr3
    mov [esi + Linked_PCB_page_dir_offset],eax

    ; setup for next process
    mov esi, [esi + Linked_PCB_next_offset]

    

    cmp esi, 0
    jne .found_next
    ; go to first proc since no next
    mov esi, [_scheduler_first_process]
    
.found_next: ;<<<<<<<<<<<<<<< maybe add flags check if needed later >>>>>>>>>>>>>>>>>

    ;LOGGING START
    push msg_switching
    call serial_write_string
    add esp, 4

    push dword [esi + Linked_PCB_name_offset]
    call serial_write_string
    add esp, 4  

    mov ax, [esi + Linked_PCB_pid_offset]
    push word 0
    push word ax
    push msg_pid
    call serial_log_hex
    add esp, 8

    mov eax, [esi + Linked_PCB_page_dir_offset]
    push eax
    push msg_cr3
    call serial_log_hex
    add esp, 8

    mov eax, [esi + Linked_PCB_esp_offset]
    push eax
    push msg_esp
    call serial_log_hex
    add esp, 8

    mov eax, [esi + Linked_PCB_ebp_offset]
    push eax
    push msg_ebp
    call serial_log_hex
    add esp, 8

    mov eax, [esi + Linked_PCB_next_offset]
    push eax
    push msg_next
    call serial_log_hex
    add esp, 8
    ; LOGGING END
    


    ;update "_scheduler_current_process"
    mov [_scheduler_current_process], esi

    ;load page dir(since this handler is identity mapped in it)
    mov eax, [esi + Linked_PCB_page_dir_offset]
    mov cr3, eax
    

    ;load esp and ebp
    mov ebp, [esi + Linked_PCB_ebp_offset]
    mov esp, [esi + Linked_PCB_esp_offset]

    ; pop gs
    ; pop fs
    ; pop es
    ; pop ds
    ; mov eax, [esp + 36]               ; get EIP from iret frame
    ; push eax
    ; push msg_switching                ; reuse "Switching process..." label or make new msg_iret
    ; call serial_log_hex
    ; add esp, 8


    popfd; pushed by
    popad; the timer irq
    sti
    iretd



