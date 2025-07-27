#ifndef GDT_H
#define GDT_H
#include <stdint.h>

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

void initGdt();
void setGdtGate(uint32_t gate, uint32_t base, uint32_t limit, uint8_t access, uint8_t granularity);



#endif 