#include "headers/multiboot_info.h"
bool checkFlag(multiboot_info_t mb_info, uint8_t index){
    return mb_info.flags & (1 << index);
};