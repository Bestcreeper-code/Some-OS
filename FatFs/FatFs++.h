#ifndef FATFSPP_H
#define FATFSPP_H
#include "ff.h"
#include "../src/headers/Logger.h"

FATFS **get_fatfs_sys_array();

void fs_set(FATFS* fs, int vol);

#endif // FATFSPP_H

