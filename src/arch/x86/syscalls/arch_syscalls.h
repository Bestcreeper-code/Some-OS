#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "arch_asm.h"

#include <stdint.h>

typedef struct 
{
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi;
} __attribute__((packed)) syscall_args_t;

extern syscall_args_t _syscall_args;

int syscall_handler(
    x32register_t eax,          //syscall number
    x32register_t ebx,          //arg1
    x32register_t ecx,          //arg2
    x32register_t edx,          //arg3
    x32register_t esi,          //arg4
    x32register_t edi           //arg5
);
int sys_write(
    x32register_t fd,
    x32register_t buf,
    x32register_t count
);

#endif // SYSCALLS_H
