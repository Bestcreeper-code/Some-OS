#ifndef GDT_H
#define GDT_H
#include <stdint.h>



void init_gdt();
void init_tss(uintptr_t stack_ptr);

void setTss_sp(uintptr_t stack_ptr);



#endif 