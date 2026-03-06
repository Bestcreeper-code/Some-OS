#ifndef GDT_H
#define GDT_H
#include <stdint.h>

#define USER_CODE_SEGMENT 0x1B
#define USER_DATA_SEGMENT 0x23


void init_gdt();
void init_tss(uintptr_t stack_ptr);

void setTss_sp(uintptr_t stack_ptr);



#endif 