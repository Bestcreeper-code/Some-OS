#include "scheduler.h"
#include "Logger.h"
#include "helpers.h"
#include "io.h"
#include "paging.h"
#include "string.h"
#include "arch_gdt.h"
#include "arch_asm.h"
#include "memory.h"
#include "drivers.h"
#include "time.h"


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
    
    Linked_PCB_t* new_pcb = (Linked_PCB_t*)kmalloc(sizeof(Linked_PCB_t));
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
    kfree(pcb);
    return 0;
}


REGISTER_DRIVER_CORE(scheduler, scheduler_init);
int scheduler_init(){   
    
    pid_bitmap[0] &= ~(1 << 0);
    
    new_pcb(&_k_pd,"Kernel",(uint32_t*)0x200000,(Stack_t){0x200000,0x1FF000},(Stack_t){1,1}); 

    _scheduler_current_process = _scheduler_first_process;
    
    enable_scheduler();
    return 0;
}

void enable_scheduler(){
    task_switching_flag=1;
}

void disable_scheduler(){
    task_switching_flag=0;
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




void _setup_kernel_stack_sched_frame(void* stack_top, uint32_t entry, uint32_t* out_esp) {
    uint32_t* sp = (uint32_t*)stack_top;

    // iret frame (CPU would have pushed this FIRST)
    *--sp = 0x202;                 // eflags (iret)
    *--sp = KERNEL_CODE_SEGMENT;   // cs
    *--sp = entry;                 // eip

    // pushad (your ISR)
    *--sp = 0; // eax
    *--sp = 0; // ecx
    *--sp = 0; // edx
    *--sp = 0; // ebx
    *--sp = 0; // esp (dummy)
    *--sp = 0; // ebp
    *--sp = 0; // esi
    *--sp = 0; // edi

    // // pushfd (your ISR, LAST push = TOP of stack)
    // *--sp = 0x202;

    *out_esp = (uint32_t)sp;
}

pid_t ktask_start(void* entry, char* name) {
    void* stack = (void*)PAGE_ADDR(page_alloc(DEFAULT_STACK_PAGE_AMOUNT,PAGE_FLAG_RW));
    RET_IF(!stack, 0);
    uintptr_t out_esp;
    _setup_kernel_stack_sched_frame(stack+PAGE_ADDR(DEFAULT_STACK_PAGE_AMOUNT), (uint32_t)entry, (uint32_t*)&out_esp);

    return new_pcb((PD_t*)&_k_pd, name, &out_esp,
        (Stack_t){.bottom = (uintptr_t)stack,.top = (uintptr_t)stack+PAGE_ADDR(DEFAULT_STACK_PAGE_AMOUNT)},
        (Stack_t){0});
    
}


volatile int testdata=1;

void testing(){
    int e = ++testdata;
    while (1) {
    
        Sys_log("hello from kthread %d\n",e);
        
    }
}

static inline void LOG_PCB(Linked_PCB_t* pcb) {
    RET_IF(pcb->pid==1,);
    Sys_log_NoPos("Switching to process %s ",pcb->name);

    Sys_log_NoPos("PID = 0x%04x ", pcb->pid);
    Sys_log_NoPos("CR3 = 0x%x ", pcb->cr3);
    Sys_log_NoPos("V_ESP = 0x%x ", pcb->k_esp);
    Sys_log_NoPos("NEXT PCB phys addr = 0x%p\n", pcb->next);
}

void* sched_next_process_core(uint32_t saved_esp) {

    Linked_PCB_t* current = _scheduler_current_process;

    current->k_esp = saved_esp;
    

    uint32_t cr3;
    __asm__ volatile ("mov %%cr3, %0" : "=r"(cr3));
    current->cr3 = cr3;

    Linked_PCB_t* next = current->next;
    if (!next) {
        next = _scheduler_first_process;
    }

#if DEBUG_SCHED_LOG
    LOG_PCB(next);
#endif

    _scheduler_current_process = next;

    __asm__ volatile ("mov %0, %%cr3" :: "r"(next->cr3));


    setTss_sp(next->k_esp);

    return (void*)next->k_esp;
}

// __attribute__((naked)) void _sched_next_process(void) {
//     __asm__ volatile (
//         "cli\n\t"

//         "call sched_next_process_core\n\t"
        
//         "popfd\n\t"
//         "popad\n\t"
//         "iretd\n\t"
//     );
// }