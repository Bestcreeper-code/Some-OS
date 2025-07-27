#include "headers/idt.h"
#include "headers/asm.h"
#include "headers/string.h"

#define IDT_ENTRIES 256
struct IDTEntry idt[IDT_ENTRIES];
struct IDTPtr idt_reg;

static inline void idt_flush(uint32_t idt_ptr_addr) {
    __asm__ volatile ("lidt (%0)" : : "r"(idt_ptr_addr));
}

extern void irq0_handler();
extern void isr13(); // General Protection Fault
extern void irq_dummy_handler();




void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
}

void idt_init() {
    idt_reg.base = (uint32_t)&idt;
    idt_reg.limit = sizeof(struct IDTEntry) * IDT_ENTRIES - 1;
    memset(&idt, 0, sizeof(idt));
    for (int i = 32; i <= 47; i++) {
        idt_set_gate(i, (uint32_t)irq_dummy_handler, 0x08, 0x8E);
    }

    idt_set_gate(32, (uint32_t)irq0_handler, 0x08, 0x8E); // IRQ0 (timer)
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);

    idt_flush((uint32_t)&idt_reg);
}




