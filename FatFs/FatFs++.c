#include "FatFs++.h"
void fs_set(FATFS* fs, int vol){
	get_fatfs_sys_array()[vol] = fs;
}