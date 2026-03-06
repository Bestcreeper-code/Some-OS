#include "arch_gdt.h"
#include "gdt.h"
#include "string.h"

extern void gdt_flush(uint32_t);

struct gdt_entry_struct gdt_entries[6];
struct gdt_ptr_struct gdt_ptr;
static struct tss_entry tss;

void setGdtGate(uint32_t gate, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity){
    gdt_entries[gate].base_low = (base & 0xFFFF);
    gdt_entries[gate].base_middle = (base >> 16) & 0xFF;
    gdt_entries[gate].base_high = (base >> 24) & 0xFF;

    gdt_entries[gate].limit = (limit & 0xFFFF);
    gdt_entries[gate].flags = (limit >> 16) & 0x0F;
    gdt_entries[gate].flags |= (granularity & 0xF0);
    gdt_entries[gate].access = access;
}

void setTssGate(uint32_t index, uint32_t base, uint32_t limit) {
    gdt_entries[index].base_low    = base & 0xFFFF;
    gdt_entries[index].base_middle = (base >> 16) & 0xFF;
    gdt_entries[index].base_high   = (base >> 24) & 0xFF;

    gdt_entries[index].limit       = limit & 0xFFFF;
    gdt_entries[index].flags       = (limit >> 16) & 0x0F; 
    gdt_entries[index].flags      |= 0x00;  // granularity = 0
    gdt_entries[index].access      = 0x89;  // present + type=0x9 (32-bit TSS)
}


void init_gdt(){
    gdt_ptr.limit = (sizeof(struct gdt_entry_struct)*6) - 1;
    gdt_ptr.base = (uint32_t)&gdt_entries;
    
    setGdtGate(0, 0, 0, 0, 0);                          // Null segment
    setGdtGate(1, 0, 0xFFFFFFFF, 0x9A, 0xCF);           // Kernel code
    setGdtGate(2, 0, 0xFFFFFFFF, 0x92, 0xCF);           // Kernel data
    setGdtGate(3, 0, 0xFFFFFFFF, 0xFA, 0xCF);           // User code
    setGdtGate(4, 0, 0xFFFFFFFF, 0xF2, 0xCF);           // User data
    setTssGate(TSS_GDT_INDEX, (uint32_t)&tss, sizeof(tss)-1);


    gdt_flush((uint32_t)&gdt_ptr);
    
}


void init_tss(uint32_t esp) {
    memset(&tss, 0, sizeof(tss));
    tss.ss0 = 0x10;              // kernel data selector
    tss.esp0 = esp;              // top of kernel stack
    asm volatile("ltr %0" : : "r"(5 << 3)); // load TSS (selector = 5<<3)
}




void setTss_sp(uint32_t esp){
    tss.esp0 = esp;
}


