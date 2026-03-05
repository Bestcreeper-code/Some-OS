#include "arch.h"
#include "idt.h"
#include "init/gdt.h"

int arch_init(){
    init_gdt();
    idt_init();
}