#include "headers/scheduler.h"
#include "headers/memory.h"
#include "headers/string.h"

Linked_PCB_t* first_process = NULL;

uint16_t process_list_depth = 0;

#define _PID_BITMAP_SIZE MAX_PID / 32

uint32_t pid_bitmap[_PID_BITMAP_SIZE];

uint16_t _get_unused_pid() {
    uint32_t* current;

    for (int i = 0; i < _PID_BITMAP_SIZE; i++) {
        current = &pid_bitmap[i];
        if (!current){
            continue;
        }
        for (int j = 0; j < 32; j++) {
            if (!(*current & (1 << j))) {
                *current |= (1 << j);
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
    *addr &= ~(1 << bit);
}

int new_pcb(PD_t* page_dir, const char* name, uint32_t* esp, uint32_t* ebp) {
    
    Linked_PCB_t* new_pcb = (Linked_PCB_t*)malloc(sizeof(Linked_PCB_t));
    if (!new_pcb) return -1;

    
    new_pcb->name = strdup(name);
    new_pcb->esp = 0;
    new_pcb->ebp = 0;
    new_pcb->state = 0; 
    new_pcb->page_dir = page_dir;
    new_pcb->next = NULL;
    new_pcb->pid = _get_unused_pid();

    if (!first_process) {
        first_process = new_pcb;
    } else {
        Linked_PCB_t* current = first_process;
        while (current->next) {
            current = current->next;
        }
        current->next = new_pcb;
    }

    

    return 0;

}