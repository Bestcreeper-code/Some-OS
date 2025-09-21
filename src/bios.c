#include "headers/bios.h"
#include "headers/memory.h"
#include "headers/Logger.h"
#include "headers/string.h"

extern void Realmode_func_runner(void (*func)(void));


extern uint8_t switch_handler_start[];
extern uint8_t switch_handler_leave16[];
extern uint8_t switch_handler_leave32[];
extern uint8_t switch_handler_end[];


void Realmode_run(void (*func)(void)) {
    // Calculate sizes of handlers
    size_t handler1_size = (size_t)(switch_handler_leave16 - switch_handler_start);
    size_t handler2_size = (size_t)(switch_handler_end - switch_handler_leave16);

    uint8_t *buffer = (uint8_t *)Setup_Base;

    memset(buffer,0,0X3000 - (uint32_t)buffer);

    // copy handler start to Setup_Base
    memcpy(buffer, switch_handler_start, handler1_size);

    // copy function after handler start
    memcpy(buffer + handler1_size, func, 150);  // assume max 150 bytes

    // copy of handler leave after function
    memcpy(buffer + handler1_size + 150, switch_handler_leave16, handler2_size);

    Realmode_func_runner(func);
}
