#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"

#define DEBUG_MODE 0
#define QEMU       0
#define FATFS_SYS_ADDR 0x24C0


static char graphics_mode = 0x03;
static FATFS* FatFsSys = (FATFS*)FATFS_SYS_ADDR;

#endif