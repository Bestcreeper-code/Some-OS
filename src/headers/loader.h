#ifndef LOADER_H
#define LOADER_H

#include <stdint.h>
#include "addresses.h"

#define MAX_PROCESSES 16
#define PROCESS_STACK_SIZE 0x4000 //16KB

typedef struct 
{
    char* name;
    uint32_t base; //NULL if Unused
    uint32_t stack_base;
    uint32_t stack_end; 
    uint32_t size;
} __attribute__((packed)) Process_Table_Entry;

typedef struct 
{
    uint8_t current_process;
    Process_Table_Entry entries[16];
}__attribute__((packed)) Process_Table;


void* new_process(char* path);

void switch_process(uint32_t old_proc_esp);
 
#endif // LOADER_H
