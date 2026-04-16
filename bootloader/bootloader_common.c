#include "bootloader_common.h"
#include "bootloader.h"
#include <stdint.h>




bool check_bl_flag(uint32_t index){
    return get_bootloader_generic_info()->boot_flags & (index);
}