[BITS 32]
global _sched_next_process
global _ret_to_next_process

extern sched_next_process_core


section .text
_sched_next_process:
    cli
    push esp
    call sched_next_process_core
_ret_to_next_process:
    mov esp, eax
    ; popfd
    popad
    iretd