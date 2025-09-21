#include "headers/multiboot_info.h"

#include "headers/string.h"
bool checkFlag(multiboot_info_t mb_info, uint8_t index){
    return mb_info.flags & (1 << index);
}

multiboot_info_t* Get_multiboot_info(){
    return (multiboot_info_t*)*(intptr_t*)MULTIBOOT_INFO_ADDRESS;
}

multiboot_module_t* Multibbot_Get_loaded_module(multiboot_info_t* mbinfo, const char* name) {
    if (mbinfo->mods_count == 0) {
        return NULL;
    }

    multiboot_module_t* modules = (multiboot_module_t*) mbinfo->mods_addr;

    for (uint32_t i = 0; i < mbinfo->mods_count; i++) {
        char* cmdline = (char*) modules[i].cmdline;
        if (cmdline && strcmp(cmdline, name) == 0) {
            return &modules[i];
        }
    }

    return NULL;  
}