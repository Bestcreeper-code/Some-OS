#include "headers/multiboot_info.h"
bool checkFlag(multiboot_info_t mb_info, uint8_t index){
    return mb_info.flags & (1 << index);
}

multiboot_info_t* Get_multiboot_info(){
    return (multiboot_info_t*)*(intptr_t*)MULTIBOOT_INFO_ADDRESS;
}