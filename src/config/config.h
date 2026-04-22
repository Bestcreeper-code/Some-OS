#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"
#include "../headers/kernel_data.h"


// DEBUG flags
#define PAGE_DEBUG      0
#define MEM_DEBUG       0
#define KSYMS_DEBUG     0
#define BLKDEV_DEBUG    1
#define ELF_DEBUG       0
#define SYSCALL_DEBUG   0

#define DEBUG_SCHED_LOG 0


#define POS_DEBUG_LOGS  1


#define VERY_EARLY_SERIAL 1




//Filesystem
#define OS_PARTITION_LABEL "CREEPER_OS"

static char graphics_mode = 0x03;
static FATFS* FatFsSys = (FATFS*)FATFS_SYS;

// Kernel Console
extern const uint32_t k_console_palette[16];

#endif