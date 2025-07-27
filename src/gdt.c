#include "headers/gdt.h"

extern void gdt_flush(uint32_t);

struct gdt_entry_struct gdt_entries[5];
struct gdt_ptr_struct gdt_ptr;

void initGdt(){
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct)*5) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;
    
    setGdtGate(0,0,0,0,0); //null seg
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);//kernel code seg
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);//kernel data seg
    setGdtGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);//user code seg
    setGdtGate(3, 0, 0xFFFFFFFF, 0xF2, 0xCF);//user data seg

    gdt_flush((uint32_t)&gdt_ptr);
    
}


void setGdtGate(uint32_t gate, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity){
    gdt_entries[gate].base_low = (base & 0xFFFF);
    gdt_entries[gate].base_middle = (base >> 16) & 0xFF;
    gdt_entries[gate].base_high = (base >> 24) & 0xFF;

    gdt_entries[gate].limit = (limit & 0xFFFF);
    gdt_entries[gate].flags = (limit >> 16) & 0x0F;
    gdt_entries[gate].flags |= (granularity & 0xF0);
    gdt_entries[gate].access = access;
}