#include "arch.h"
#include "idt.h"
#include "gdt.h"

int arch_init(){
    init_gdt();
    idt_init();
}