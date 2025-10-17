#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"
#include "../headers/addresses.h"

#define DEBUG_MODE 0
#define PAGE_DEBUG_MODE 0
#define MEM_DEBUG_MODE 0
#define ELF_DEBUG_MODE 1
#define DEV_BUILD  1 // dev features like forcing disk part to 1 if not found

#define FREE_NO_MERGE 1 

#define Fat_SYS_Main_Part_Max_Size 0XFFFFFFFF //max possible size of a fat partition /"use max"
#define Fat_SYS_Main_Part_Drive_Percentage 85 //percentage of the disk to use for the main fat partition

#define OS_PARTITION_LABEL "CREEPER_OS"

static char graphics_mode = 0x03;
static FATFS* FatFsSys = (FATFS*)FATFS_SYS_ADDR;




#endif