#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <stdint.h>

typedef struct 
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi;
} __attribute__((packed)) syscall_args_t;

extern syscall_args_t _syscall_args;

int syscall_handler();

#endif // SYSCALLS_H
