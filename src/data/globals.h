#ifndef GLOBAL_DATA
#define GLOBAL_DATA
#include "../../FatFs/ff.h"

#define DEBUG_MODE 0
#define QEMU       1


static char graphics_mode = 0x03;
static FATFS FatFsSys;

#endif