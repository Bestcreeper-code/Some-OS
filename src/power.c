#include "power.h"
#include "asm.h"

void pc_reboot() {
    __asm__ volatile ("cli"); 
    
    outb(0x64, 0xFE); 

    while (1) { __asm__ volatile ("hlt"); }
}