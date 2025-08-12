#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"
#include "../headers/addresses.h"
#define DEBUG_MODE 0
#define QEMU       0



static char graphics_mode = 0x03;
static FATFS* FatFsSys = (FATFS*)FATFS_SYS_ADDR;

#endif