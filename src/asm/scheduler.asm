global _sched_next_process
global testing

extern serial_write_string

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


_sched_next_process:
    push ds
    push es
    push fs
    push gs

    ; get current process PCB
    mov esi, [_scheduler_current_process]

    
    ; save process's esp and ebp
    mov [esi + Linked_PCB_esp_offset], esp
    mov [esi + Linked_PCB_ebp_offset], ebp

    ; setup for next process
    mov esi, [esi + Linked_PCB_next_offset]

    push dword [esi + Linked_PCB_name_offset]
    call serial_write_string

    cmp esi, 0
    jne .found_next
    ; go to first proc since no next
    mov esi, [_scheduler_first_process]
    
.found_next: ;<<<<<<<<<<<<<<< maybe add flags check if needed later >>>>>>>>>>>>>>>>>
    ;load esp and ebpo
    mov ebp, [esi + Linked_PCB_ebp_offset]
    mov esp, [esi + Linked_PCB_esp_offset]

    ;load page dir(since this handler is identity mapped in it)
    mov eax, [esi + Linked_PCB_page_dir_offset]
    mov cr3, eax


    ;update "_scheduler_current_process"
    mov [_scheduler_current_process], esi

    pop gs
    pop fs
    pop es
    pop ds
    popfd; pushed by
    popad; the timer irq
    sti
    iretd



testing:
    mov eax, 1
    int 13
    lop:
    jmp lop