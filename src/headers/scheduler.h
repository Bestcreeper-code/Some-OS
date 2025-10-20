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



#endif // SCHEDULER_H
