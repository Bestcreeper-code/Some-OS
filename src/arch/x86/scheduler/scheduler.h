#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <limits.h>
#include <stdint.h>

#include "asm.h"
#include "arch_paging.h"

#define MAX_PID SHRT_MAX

typedef short pid_t;

typedef struct Linked_PCB_t {
    short pid;
    uint16_t state;
    char* name;

    Stack_t kernel_stack, user_stack;

    uint32_t k_esp;

    
    uintptr_t cr3;
    
    struct Linked_PCB_t* next;
} __attribute__((__packed__)) Linked_PCB_t; 

typedef struct __attribute__((packed)) ProcessStackFrame {
    // // seg regs push
    // uint32_t gs;
    // uint32_t fs;
    // uint32_t es;
    // uint32_t ds;

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
extern uint8_t task_switching_flag;

int scheduler_init();

pid_t new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, Stack_t k_stack, Stack_t us_stack);

void _setup_user_stack_sched_frame(void* stack_frame_upper, uint32_t* esp, uint32_t entry);


#endif // SCHEDULER_H
