#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include "asm.h"
#include "paging.h"

#define MAX_PID 32768

typedef struct Linked_PCB_t {
    uint16_t pid;
    char* name;

    uint32_t esp, ebp;

    uint8_t state;

    PD_t* page_dir;

    struct Linked_PCB_t* next;
} __attribute__((__packed__)) Linked_PCB_t; 

typedef struct __attribute__((packed)) ProcessStackFrame {
    // pushfd 
    uint32_t eflags;

    // pushad 
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    //iret frame (CPU pushes on interrupt) 
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags_iret;

    
    uint32_t useresp;
    uint32_t ss;
} ProcessStackFrame;


extern Linked_PCB_t* _scheduler_current_process;
extern Linked_PCB_t* _scheduler_first_process;

int scheduler_init();

int new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, uint32_t* ebp);

void _setup_user_stack_sched_frame(void* stack_frame_upper, uint32_t esp, uint32_t entry);


#endif // SCHEDULER_H
