#include "bootloader.h"
#include <stdint.h>




struct bootloader_loaded_module* get_bootloader_module(char* name) {
    
    for(int i = 0; i< sizeof(bl_modules_list) / sizeof(bl_modules_list[0]); i++) {
        if (strcmp(bl_modules_list[i].cmdline, name) == 0){
            return &bl_modules_list[i];
        }
    }
    return NULL;
}