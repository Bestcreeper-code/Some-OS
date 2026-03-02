#include "arch.h"
#include "init/gdt.h"

int arch_init(){
    init_gdt();
}