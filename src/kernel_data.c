#include "headers/kernel_data.h"

int Get_Kernel_Flag(Kernel_Data_Flag flag){
    const uint8_t entry_size = sizeof(kernel_data.kernel_flags[0]) *8;
    const uint16_t array_size = sizeof(kernel_data.kernel_flags) / sizeof(kernel_data.kernel_flags[0]);

    if ((flag / entry_size) >= array_size) {
        return -1;
    }

    return (kernel_data.kernel_flags[(uint32_t)flag/entry_size] >> (flag % entry_size)) & 1;
}

int Set_Kernel_Flag(Kernel_Data_Flag flag, bool value) {
    const uint8_t bits_per_entry = sizeof(kernel_data.kernel_flags[0]) * 8;
    const uint16_t array_size = sizeof(kernel_data.kernel_flags) / sizeof(kernel_data.kernel_flags[0]);

    uint16_t index = flag / bits_per_entry;
    uint8_t bit   = flag % bits_per_entry;

    if (index >= array_size) {
        return -1;
    }

    if (value)
        kernel_data.kernel_flags[index] |= (1U << bit);
    else
        kernel_data.kernel_flags[index] &= ~(1U << bit);

    return 0; 
}