#ifndef SCHEDULER_H
#define SCHEDULER_H


#include <stdint.h>

#include "asm/arch_asm.h"
#include "paging/arch_paging.h"
#include "lists.h"

#define MAX_PID                     16384   

#define DEFAULT_STACK_PAGE_AMOUNT   32
#define DEFAULT_STACK_PAGE_BYTES    (DEFAULT_STACK_PAGE_AMOUNT<<12)
#define STACK_UPPER_USPACE_ADDR     0XBFFFFFFF

typedef short pid_t;

typedef struct Linked_PCB_t {
    short pid;
    uint16_t state;
#define PCB_STATE_RUNNING   0x0000
#define PCB_STATE_ZOMBIE    0x0004
    char* name;

    Stack_t kernel_stack, user_stack;

    uint32_t k_esp;

    int exit_code;

    
    uintptr_t cr3;
    
    struct hlist_node list_node;
}  Linked_PCB_t; 

typedef struct __attribute__((packed)) ProcessStackFrame {
    
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


typedef struct __attribute__((packed)) KProcessStackFrame {
    

    // pushad
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t esp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    // CPU-pushed iret frame
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags_iret;

} KProcessStackFrame;

extern Linked_PCB_t* _scheduler_current_process;
extern struct hlist_head _scheduler_process_list_head;
extern uint8_t task_switching_flag;

int scheduler_init();


void* sched_next_process_core(uint32_t saved_esp);

Linked_PCB_t* new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, Stack_t k_stack, Stack_t us_stack);

void _setup_user_stack_sched_frame(void* us_stack_top, void* k_stack_top, uint32_t entry, uint32_t* out_esp);
void _setup_kernel_stack_sched_frame(void* stack_top, uint32_t entry, uint32_t* out_esp);

int kill_ktask(Linked_PCB_t* pcb);

Linked_PCB_t* ktask_start(void* entry, char* name);
Linked_PCB_t* us_task_start(void* entry, char* name, PD_t page_dir);
void enable_scheduler();
void disable_scheduler();

void yield_core(uint32_t esp);
void _yield();

#endif // SCHEDULER_H
