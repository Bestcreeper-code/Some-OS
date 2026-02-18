#include "headers/scheduler.h"
#include "Logger.h"
#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/string.h"
#include "headers/gdt.h"
#include "headers/asm.h"
#include "headers/time.h"


Linked_PCB_t* _scheduler_first_process = 0;

uint16_t process_list_depth = 1;

Linked_PCB_t* _scheduler_current_process = 0;

#define _PID_BITMAP_SIZE (MAX_PID / 32)

uint8_t task_switching_flag = 0;


uint32_t pid_bitmap[_PID_BITMAP_SIZE] =  {[0 ... _PID_BITMAP_SIZE-1] = 0xFFFFFFFF};//0= used and 1 = FREE 

pid_t _get_unused_pid() { 
    uint32_t* current;

    for (int i = 0; i < _PID_BITMAP_SIZE; i++) {
        current = &pid_bitmap[i];
        
        if (!*current){
            continue;
        }
        for (int j = 0; j < 32; j++) {
            if (*current & (1 << j)) {
                *current &= ~(1 << j);
                return (pid_t)(i * 32 + j);
            }
            
        }
    }
    
    return (pid_t)-1; 
}

void _free_pid(pid_t pid) {
    
    uint16_t index = pid / 32;
    uint8_t bit = pid % 32;

    if (index >= _PID_BITMAP_SIZE) {
        return; 
    }

    uint32_t* addr = &pid_bitmap[index];
    *addr |= (1 << bit);
}

pid_t new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, Stack_t k_stack, Stack_t us_stack) {
    
    Linked_PCB_t* new_pcb = (Linked_PCB_t*)malloc(sizeof(Linked_PCB_t));
    if (!new_pcb){ return -1;}

    
    new_pcb->pid = _get_unused_pid();
    if(new_pcb->pid<0)return -2;

    new_pcb->name = strdup(name);
    new_pcb->state = 0; 
    
    new_pcb->user_stack = us_stack;
    new_pcb->kernel_stack = k_stack;
    
    new_pcb->k_esp = (uint32_t)*esp;  

    new_pcb->cr3 = (uintptr_t)page_dir->pde_arr; 
    new_pcb->next = NULL;

    if (!_scheduler_first_process) {
        _scheduler_first_process = new_pcb;
        
    } else {
        Linked_PCB_t* current = _scheduler_first_process;

        while (current->next) {
            current = current->next;
        }
        current->next = new_pcb;
    }

    pd_map_page(page_dir, (uint32_t)new_pcb, (uint32_t)new_pcb, 1, 1, 0);//identity map the pcb for sched
    

    return new_pcb->pid;

}

int kill_process(short proc_pid){
    Linked_PCB_t* pcb = _scheduler_first_process;
    Linked_PCB_t* old = NULL;
    int depth=0;
    while (pcb && pcb->pid != proc_pid && depth < process_list_depth)
    {
        old = pcb;
        pcb = pcb->next;
    }

    if(!pcb)return -1;

    old->next = pcb->next;
    
    _free_pid(pcb->pid);
    free(pcb);
    return 0;
}

void testing();

int scheduler_init(){   
    
    pid_bitmap[0] &= ~(1 << 0);
    
    new_pcb(&_k_pd,"Kernel\n",(uint32_t*)0x200000,(Stack_t){0x200000,0x1FF000},(Stack_t){1,1}); // FIXED: give valid kernel stack

    _scheduler_current_process = _scheduler_first_process;
    
    return 0;
}

void _setup_user_stack_sched_frame(void* stack_top, uint32_t* v_esp, uint32_t entry){
    ProcessStackFrame* frame = (ProcessStackFrame*)((uint8_t*)stack_top - sizeof(ProcessStackFrame));

    frame->eax = 0;
    frame->ebx = 0;
    frame->ecx = 0;
    frame->edx = 0;
    frame->esi = 0;
    frame->edi = 0;
    frame->ebp = (uint32_t)*v_esp;
    frame->esp = (uint32_t)*v_esp;
    frame->eip = entry;
    frame->cs = USER_CODE_SEGMENT;
    frame->eflags_iret = 0x202;
    frame->useresp = (uint32_t)*v_esp;
    frame->ss = USER_DATA_SEGMENT;

    *v_esp -=sizeof(ProcessStackFrame);

    Sys_log("making a sched frame at %x (v_esp=%x)\n",stack_top,*v_esp);
}
