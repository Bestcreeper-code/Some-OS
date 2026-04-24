#include "arch_syscalls.h"
#include "arch_asm.h"
#include "Logger.h"
#include "container_of.h"
#include "lists.h"
#include "scheduler.h"

syscall_args_t _syscall_args;

int syscall_handler(      
    x32register_t edi,//arg5
    x32register_t esi,//arg4
    x32register_t ebp,//base ptr
    x32register_t esp,//stack ptr
    x32register_t ebx,//arg1
    x32register_t edx,//arg3
    x32register_t ecx,//arg2
    x32register_t eax //syscall number
) {
#if (SYSCALL_DEBUG)
    Sys_log("syscall: eax=%x, ebx=%x, ecx=%x, edx=%x, esi=%x, edi=%x\n",
        eax.val, ebx.val, ecx.val, edx.val, esi.val, edi.val);
#endif

    switch (eax.val) {
        case 1:
            sys_exit(ebx,esp);
            
        case 4: // sys_write
            return sys_write(ebx, ecx, edx);
        default:
            Sys_log("Unknown syscall: %x\n", eax.val);
            return -1;
        

    }

    return 0;
}

int sys_write(
    x32register_t fd,
    x32register_t buf,
    x32register_t count
) {

    Sys_log("write called: fd=%d, buf=%p, count=%d\n",
        fd.val, buf.ptr, count.val);
    return 0;
}

void sys_exit(x32register_t code, x32register_t esp) {
    _scheduler_current_process->exit_code = code.val;
    _scheduler_current_process->state     = PCB_STATE_ZOMBIE;
    
    yield_core(esp.val);
}