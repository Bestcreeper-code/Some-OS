#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"
#include "../headers/addresses.h"
#define DEBUG_MODE 0
#define QEMU       0

#define Fat_SYS_Main_Part_Max_Size 0XFFFFFFFF //max possible size of a fat partition /"use max"
#define Fat_SYS_Main_Part_Drive_Percentage 85 //percentage of the disk to use for the main fat partition



static char graphics_mode = 0x03;
static FATFS* FatFsSys = (FATFS*)FATFS_SYS_ADDR;

#endif