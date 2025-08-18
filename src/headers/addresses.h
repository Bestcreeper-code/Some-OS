#ifndef ADRESSES_H
#define ADRESSES_H

#include "loader.h"

#include <stdint.h>

#define INPUT_CHAR_BUFFER_SIZE  256

#define KERNEL_STACK_BASE 0x101FFF//      |
#define KERNEL_STACK_SIZE 0x1FFF //8Kb    | - goes from 0x101FFF to 0x100000

#define TASK_SWITCHING_FLAG            0x223B // activate/deactivate task switching between kernel and running app
#define KEYBOARD_MOD_KEYS_FLAGS        0x223C // Modifier keys flags (shift,ctrl,...)
#define INPUT_CHAR_BUFFER_ADDRESS      0x223D // Buffer for 256 input chars (start)
#define FATFS_SYS_ADDR                 0x233D // FATFS system data (0x0234 bytes size)
#define KERNEL_STACK_BASE_ADDRESS      0x2571 // uint32, address storing kernel EbP
#define KERNEL_STACK_POINTER_ADDRESS   0x2575 // uint32 | the address of the kernel ESP
#define PROCESS_TABLE                  0x2579 // 381 Bytes (Process_Table)
#define MULTIBOOT_INFO_ADDRESS         0x26EE // uint32
#define TICKS_AMOUNT                   0x26F2 // uint64
#define FREE_REGION_MAP                0x26FA // 2052 Bytes or 0x804 Bytes (free_region_map_t)
// typedef struct {
//     uintptr_t mb_info_addr;
//     uintptr_t tick_amount_ptr;
//     uintptr
// } __attribute__((packed)) Sys_Info_Struct; scrapped for now since somehow breaks the adresses

#endif // ADRESSES_H
