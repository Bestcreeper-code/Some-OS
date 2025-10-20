#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"
#include "../headers/addresses.h"

#define DEBUG_MODE 0
#define PAGE_DEBUG_MODE 0
#define MEM_DEBUG_MODE 0
#define ELF_DEBUG_MODE 1
#define DEV_BUILD  1 



#define FREE_NO_MERGE 1 

#define OS_PARTITION_LABEL "CREEPER_OS"

static char graphics_mode = 0x03;
static FATFS* FatFsSys = (FATFS*)FATFS_SYS_ADDR;




#endif