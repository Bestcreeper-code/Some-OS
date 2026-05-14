#include "scheduler.h"
#include "Logger.h"
#include "arch_paging.h"
#include "container_of.h"
#include "helpers.h"
#include "io.h"
#include "lists.h"
#include "paging.h"
#include "string.h"
#include "arch_gdt.h"
#include "arch_asm.h"
#include "memory.h"
#include "drivers.h"
#include "time.h"
#include <stdint.h>


HLIST_HEAD(_scheduler_process_list_head);   

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

Linked_PCB_t* new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, Stack_t k_stack, Stack_t us_stack) {
    Sys_Info("O\n");
    Linked_PCB_t* new_pcb = (Linked_PCB_t*)kmalloc(sizeof(Linked_PCB_t));
    if (!new_pcb){ return NULL;}
    Sys_Info("O\n");
    
    new_pcb->pid = _get_unused_pid();
    if(new_pcb->pid<0)return NULL;
    Sys_Info("O\n");
    new_pcb->name = strdup(name);
    new_pcb->state = 0; 
    
    new_pcb->user_stack = us_stack;
    new_pcb->kernel_stack = k_stack;
    Sys_Info("O\n");
    new_pcb->k_esp = (uint32_t)*esp;  
    

    uintptr_t cr3_val = (uintptr_t)page_dir->pde_arr;
    
    if (cr3_val >= KERNEL_VMA)
        cr3_val = HHDM_TO_PHYS(cr3_val);
    new_pcb->cr3 = cr3_val;
    

    new_pcb->list_node.next = NULL;
    Sys_Info("O\n");
    if (!_scheduler_process_list_head.first) {
        _scheduler_process_list_head.first = &new_pcb->list_node;
        
    } else {

        hlist_add_head(&new_pcb->list_node, &_scheduler_process_list_head);
        
    }
    Sys_Info("O\n");

    // pd_map_page(page_dir, (uint32_t)new_pcb, (uint32_t)new_pcb, 1, 1, 0);//identity map the pcb for sched
    
    Sys_Info("O\n");
    return new_pcb;

}

int kill_ktask(Linked_PCB_t* pcb) {
    if(!pcb)return -1;
    Sys_log("K Process %u (%s) exited with err:%d\n",pcb->pid,pcb->name,pcb->exit_code);
    hlist_del(&pcb->list_node);    
    _free_pid(pcb->pid);

    page_free(ADDR_TO_PAGE(pcb->kernel_stack.top), pcb->kernel_stack.size / PAGE_SIZE);
    page_free(ADDR_TO_PAGE(pcb->kernel_stack.top), pcb->kernel_stack.size / PAGE_SIZE);

    kfree(pcb);
    process_list_depth--;
    return 0;
}


REGISTER_DRIVER_CORE(scheduler, scheduler_init);
int scheduler_init(){   
    
    pid_bitmap[0] &= ~(1 << 0);

    PD_t k_pd = {.pde_arr=(void*)_k_pd_phys};
    uint32_t tmp=0x200000;
    new_pcb(&k_pd,"Kernel",&tmp,(Stack_t){0x200000,0x1FF000},(Stack_t){1,1}); 

    _scheduler_current_process = container_of(_scheduler_process_list_head.first,Linked_PCB_t,list_node);
    
    enable_scheduler();
    return 0;
}

void enable_scheduler(){
    task_switching_flag=1;
}

void disable_scheduler(){
    task_switching_flag=0;
}

void _setup_user_stack_sched_frame(void* us_stack_start, void* k_stack_start, uint32_t entry, uint32_t* out_esp) {
    uint32_t* sp = (uint32_t*)k_stack_start;

    *--sp = USER_DATA_SEGMENT; // ss
    *--sp = (uintptr_t)us_stack_start; // useresp
    // iret frame 
    *--sp = 0x202;                 // eflags
    *--sp = USER_CODE_SEGMENT;   // cs
    *--sp = entry;                 // eip

    // pushad 
    *--sp = 0; // eax
    *--sp = 0; // ecx
    *--sp = 0; // edx
    *--sp = 0; // ebx
    *--sp = 0; // esp (dummy)
    *--sp = 0; // ebp
    *--sp = 0; // esi
    *--sp = 0; // edi
    

    

    *out_esp = (uint32_t)sp;
}

Linked_PCB_t* us_task_start(void* entry, char* name, PD_t page_dir) {
    //kernel space bc ... why not?
    void* k_stack = (void*)PAGE_ADDR(page_alloc(DEFAULT_STACK_PAGE_AMOUNT,PAGE_FLAG_RW));
    RET_IF(!k_stack, 0);
    
    page_index us_stack_pages = page_alloc_nomap(DEFAULT_STACK_PAGE_AMOUNT);
    uintptr_t us_stack_bott = PAGE_ADDR(us_stack_pages);
    
    RET_IF(!us_stack_pages, 0);
    pd_map_page(&page_dir, STACK_UPPER_USPACE_ADDR-DEFAULT_STACK_PAGE_BYTES, us_stack_pages, 1, 1, 1);

    uintptr_t out_esp;
    _setup_user_stack_sched_frame(
        (void*)us_stack_bott+DEFAULT_STACK_PAGE_BYTES,
        k_stack+DEFAULT_STACK_PAGE_BYTES,
        (uintptr_t)entry,
        &out_esp
    );

    return new_pcb((PD_t*)&_k_pd, name, &out_esp,
        (Stack_t){.top = (uintptr_t)k_stack,.bottom = (uintptr_t)k_stack+DEFAULT_STACK_PAGE_BYTES,.size=DEFAULT_STACK_PAGE_BYTES},
        (Stack_t){.top = (uintptr_t)us_stack_bott,.bottom = (uintptr_t)us_stack_bott+DEFAULT_STACK_PAGE_BYTES,.size=DEFAULT_STACK_PAGE_BYTES}
        );
    
}



void _setup_kernel_stack_sched_frame(void* kstack_start, uint32_t entry, uint32_t* out_esp) {
    uint32_t* sp = (uint32_t*)kstack_start;

    *--sp = 0; // to stop the stack trace from dying
    // iret frame 
    *--sp = 0x202;                 // eflags
    *--sp = KERNEL_CODE_SEGMENT;   // cs
    *--sp = entry;                 // eip

    // pushad 
    *--sp = 0; // eax
    *--sp = 0; // ecx
    *--sp = 0; // edx
    *--sp = 0; // ebx
    *--sp = 0; // esp (dummy)
    *--sp = 0; // ebp
    *--sp = 0; // esi
    *--sp = 0; // edi

    

    *out_esp = (uint32_t)sp;
}

Linked_PCB_t* ktask_start(void* entry, char* name) {
    void* stack = (void*)PAGE_ADDR(page_alloc(DEFAULT_STACK_PAGE_AMOUNT,PAGE_FLAG_RW));
    RET_IF(!stack, 0);
    uintptr_t out_esp;
    _setup_kernel_stack_sched_frame(stack+DEFAULT_STACK_PAGE_BYTES, (uint32_t)entry, (uint32_t*)&out_esp);

    return new_pcb((PD_t*)&_k_pd, name, &out_esp,
        (Stack_t){.top = (uintptr_t)stack,.bottom = (uintptr_t)stack+DEFAULT_STACK_PAGE_BYTES},
        (Stack_t){0});
    
}


volatile int testdata=1;

void testing(){
    // ktask_start(testing, "test");
    uint32_t eax = 4;
    uint32_t ebx = testdata++;
    uint32_t ecx = 0x41414141;
    uint32_t edx = 4;
    for(int i=0;i<100;i++) {
        Sys_log("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d end\n",ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx,ebx);
        // asm volatile (

        //     "int $0x80"
        //     :
        //     : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx)
        //     : "memory"
        // );
    }
    Sys_log("how was the fall?\n");

    eax=1;
    ebx=-2;
    asm volatile (
        "int $0x80"
        :
        : "a"(eax), "b"(ebx), "c"(ecx), "d"(edx)
        : "memory"
    );
}

static inline void LOG_PCB(Linked_PCB_t* pcb) {
    // RET_IF(pcb->pid==1,);
    Sys_log_NoPos("Switching to process %s ",pcb->name);

    Sys_log_NoPos("PID = 0x%04x ", pcb->pid);
    Sys_log_NoPos("CR3 = 0x%x ", pcb->cr3);
    Sys_log_NoPos("V_ESP = 0x%x ", pcb->k_esp);
    Sys_log_NoPos("NEXT PCB phys addr = 0x%p\n", container_of(pcb->list_node.next,Linked_PCB_t,list_node));
}

void* sched_next_process_core(uint32_t saved_esp) {

    Linked_PCB_t* current = _scheduler_current_process;

    current->k_esp = saved_esp;
    

    struct hlist_node* next_node = current->list_node.next;
get_next_pcb:

    if (!next_node) {
        next_node = _scheduler_process_list_head.first;
    }

    Linked_PCB_t* next = container_of(next_node, Linked_PCB_t, list_node);
    if(next->state & PCB_STATE_ZOMBIE){
        next_node = next->list_node.next;
        kill_ktask(next);
        goto get_next_pcb;
    }
#if DEBUG_SCHED_LOG
    LOG_PCB(next);
#endif

    _scheduler_current_process = next;

    __asm__ volatile ("mov %0, %%cr3" :: "r"(next->cr3));


    // setTss_sp(next->k_esp);

    return (void*)next->k_esp;
}

extern void _ret_to_next_process(void* esp);
void yield_core(uint32_t esp) {
    _ret_to_next_process(sched_next_process_core(esp));
}

void _yield() {
    asm volatile (
        "int $0x80"
        :
        : "a"(158)
        : "memory"
    );
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