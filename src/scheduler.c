#include "headers/scheduler.h"
#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/string.h"
#include "headers/gdt.h"
#include "headers/asm.h"


Linked_PCB_t* _scheduler_first_process = 0;

uint16_t process_list_depth = 1;

Linked_PCB_t* _scheduler_current_process = 0;

#define _PID_BITMAP_SIZE (MAX_PID / 32)


uint32_t pid_bitmap[_PID_BITMAP_SIZE];//0= used and 1 = FREE 

uint16_t _get_unused_pid() { 
    uint32_t* current;

    for (int i = 0; i < _PID_BITMAP_SIZE; i++) {
        current = &pid_bitmap[i];
        if (!*current){
            continue;
        }
        for (int j = 0; j < 32; j++) {
            if (*current & (1 << j)) {
                *current &= ~(1 << j);
                return (uint16_t)(i * 32 + j);
            }
        }
    }
    return (uint16_t)UINT16_MAX; 
}

void _free_pid(uint16_t pid) {
    
    uint16_t index = pid / 32;
    uint8_t bit = pid % 32;

    if (index >= _PID_BITMAP_SIZE) {
        return; 
    }

    uint32_t* addr = &pid_bitmap[index];
    *addr |= (1 << bit);
}

int new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, uint32_t* ebp) {
    
    Linked_PCB_t* new_pcb = (Linked_PCB_t*)malloc(sizeof(Linked_PCB_t));
    if (!new_pcb){ return -1;}

    
    new_pcb->name = strdup(name);
    new_pcb->esp = 100000;
    new_pcb->ebp = 110000;
    new_pcb->state = 0; 
    new_pcb->page_dir = page_dir;
    new_pcb->next = NULL;
    new_pcb->pid = _get_unused_pid();

    if (!_scheduler_first_process) {
        _scheduler_first_process = new_pcb;
        
    } else {
        Linked_PCB_t* current = _scheduler_first_process;
        Sys_log("shit\n");
        Sys_log("shit\n");
        while (current->next) {
            current = current->next;
        }
        current->next = new_pcb;
    }

    

    return 0;

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
    
    memset(pid_bitmap,0xFF,MAX_PID/8);
    pid_bitmap[0] |= (1 << 31);

    new_pcb(&_k_pd,"=============================Kernel\n",(uint32_t*)0X200000,(uint32_t*)0X200000);//placeholder esp since will be pushed anywabc is the curr proc
        
    
    Sys_log("shit %s\n",_scheduler_first_process->name);
    
    
    _scheduler_current_process = _scheduler_first_process;
    Sys_log("shit %s\n",_scheduler_current_process->name);

    TASK_SWITCHING_FLAG = 1;
    Sys_log("zf\n");sleep(10000);

    return 0;
}

void _setup_user_stack_sched_frame(void* stack_frame_upper, uint32_t v_esp, uint32_t entry){
    ProcessStackFrame* stack_frame = stack_frame_upper-sizeof(ProcessStackFrame);

    ProcessStackFrame new_frame = {
        .eflags = 0x202,

        .eax = 0,
        .ebx = 0,
        .ecx = 0,
        .edx = 0,
        .esp = ((uint32_t)stack_frame)+36,
        .ebp = v_esp,
        .esi = 0,
        .edi = 0,
        

        .eip = entry,
        .cs =USER_CODE_SEGMENT,
        .eflags_iret= 0x202,

        .useresp = v_esp,
        .ss = USER_DATA_SEGMENT

    };

    *stack_frame = new_frame;
}