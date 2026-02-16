#include "headers/multiboot_info.h"

#include "headers/string.h"


extern multiboot_info_t* mb_struct_ptr;

bool checkFlag(uint32_t flags, uint8_t index){
    return flags & (1 << index);
}

multiboot_info_t* Get_multiboot_info(){
    return mb_struct_ptr;
}

multiboot_module_t* Multiboot_Get_loaded_module(multiboot_info_t* mbinfo, const char* name) {
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