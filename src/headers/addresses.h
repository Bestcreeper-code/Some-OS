#ifndef ADRESSES_H
#define ADRESSES_H

#include "loader.h"

#include <stdint.h>

#define KERNEL_STACK_BASE 0x101FFF//      |
#define KERNEL_STACK_SIZE 0x1FFF //8Kb    | - goes from 0x101FFF to 0x100000

#define KERNEL_STACK_POINTER_ADRESS 0x2575//uint32 | the adress of the kernel esp
#define PROCESS_TABLE 0x2579 //385 Bytes
#define MULTIBOOT_INFO_STORING_ADRESS 0x26FA //uint32
#define TICKS_AMOUNT 0x2700 //uint64
#define FREE_REGION_MAP 0x2708//2052 Bytes or 0x804 Bytes


// typedef struct {
//     uintptr_t mb_info_addr;
//     uintptr_t tick_amount_ptr;
//     uintptr
// } __attribute__((packed)) Sys_Info_Struct; scrapped for now since somehow breaks the adresses

#endif // ADRESSES_H
