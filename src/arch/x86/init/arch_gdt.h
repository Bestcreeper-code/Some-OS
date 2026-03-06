#ifndef GDT_H
#define GDT_H
#include <stdint.h>

#define TSS_GDT_INDEX 5

struct gdt_entry_struct
{
    uint16_t limit;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t access;
    uint8_t flags;  
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr_struct
{
    uint16_t limit;
    uint32_t base;
}__attribute__((packed)); 

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax, ecx, edx, ebx;
    uint32_t esp, ebp, esi, edi;
    uint16_t es, cs, ss, ds, fs, gs;
    uint16_t ldt;
    uint16_t trap, iomap_base;
} __attribute__((packed));

// extern struct tss_entry tss;

void init_gdt();
void init_tss(uintptr_t stack_ptr);

void setTss_sp(uintptr_t stack_ptr);



#endif 