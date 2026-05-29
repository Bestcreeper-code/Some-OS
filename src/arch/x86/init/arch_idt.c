#include "idt.h"
#include "arch_idt.h"
#include "asm/arch_asm.h"
#include "string.h"
#include "Logger.h"
#include "PS-2/mouse.h"
#include "io.h"

#define IDT_ENTRIES 256
struct IDTEntry idt[IDT_ENTRIES];
struct IDTPtr idt_reg;

static inline void idt_flush(uint32_t idt_ptr_addr) {
    __asm__ volatile ("lidt (%0)" : : "r"(idt_ptr_addr));
}

extern void irq0_handler();
extern void irq1_handler();
extern void irq12_handler();
extern void _syscall_int_80_handler();

extern void isr0();   // Divide Error
extern void isr1();   // Debug Exception
extern void isr2();   // Non Maskable Interrupt (NMI)
extern void isr3();   // Breakpoint Exception
extern void isr4();   // Overflow Exception
extern void isr5();   // Bound Range Exceeded Exception
extern void isr6();   // Invalid Opcode Exception
extern void isr7();   // Device Not Available Exception
extern void isr8();   // Double Fault Exception
extern void isr9();   // Coprocessor Segment Overrun (reserved)
extern void isr10();  // Invalid TSS Exception
extern void isr11();  // Segment Not Present Exception
extern void isr12();  // Stack-Segment Fault
extern void isr13();  // General Protection Fault
extern void isr14();  // Page Fault
extern void isr15();  // Reserved
extern void isr16();  // x87 Floating-Point Exception
extern void isr17();  // Alignment Check Exception
extern void isr18();  // Machine Check Exception
extern void isr19();  // SIMD Floating-Point Exception
extern void isr20();  // Virtualization Exception
extern void isr21();  // Control Protection Exception
extern void isr22();  // Reserved
extern void isr23();  // Reserved
extern void isr24();  // Reserved
extern void isr25();  // Reserved
extern void isr26();  // Reserved
extern void isr27();  // Reserved
extern void isr28();  // Hypervisor Injection Exception
extern void isr29();  // VMM Communication Exception
extern void isr30();  // Security Exception
extern void isr31();  // Reserved


extern void irq_dummy_handler();



void idt_set_gate(uint8_t num, uint32_t base, uint16_t sel, uint8_t flags) {
    
    Sys_log("Setting IDT gate %x\n", num);
    idt[num].offset_low = base & 0xFFFF;
    idt[num].selector = sel;
    idt[num].zero = 0;
    idt[num].flags = flags;
    idt[num].offset_high = (base >> 16) & 0xFFFF;
}

void idt_init() {
    Sys_log("Setting up IDT...\n");

    idt_reg.base = (uint32_t)&idt;
    idt_reg.limit = sizeof(struct IDTEntry) * IDT_ENTRIES - 1;
    memset(&idt, 0, sizeof(idt));
    
    // CPU exceptions (ISRs 0-31)
    idt_set_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    idt_set_gate(1,  (uint32_t)isr1,  0x08, 0x8E);
    idt_set_gate(2,  (uint32_t)isr2,  0x08, 0x8E);
    idt_set_gate(3,  (uint32_t)isr3,  0x08, 0x8E);
    idt_set_gate(4,  (uint32_t)isr4,  0x08, 0x8E);
    idt_set_gate(5,  (uint32_t)isr5,  0x08, 0x8E);
    idt_set_gate(6,  (uint32_t)isr6,  0x08, 0x8E);
    idt_set_gate(7,  (uint32_t)isr7,  0x08, 0x8E);
    idt_set_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    idt_set_gate(9,  (uint32_t)isr9,  0x08, 0x8E);
    idt_set_gate(10, (uint32_t)isr10, 0x08, 0x8E);
    idt_set_gate(11, (uint32_t)isr11, 0x08, 0x8E);
    idt_set_gate(12, (uint32_t)isr12, 0x08, 0x8E);
    idt_set_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    idt_set_gate(14, (uint32_t)isr14, 0x08, 0x8E);
    idt_set_gate(15, (uint32_t)isr15, 0x08, 0x8E);
    idt_set_gate(16, (uint32_t)isr16, 0x08, 0x8E);
    idt_set_gate(17, (uint32_t)isr17, 0x08, 0x8E);
    idt_set_gate(18, (uint32_t)isr18, 0x08, 0x8E);
    idt_set_gate(19, (uint32_t)isr19, 0x08, 0x8E);
    idt_set_gate(20, (uint32_t)isr20, 0x08, 0x8E);
    idt_set_gate(21, (uint32_t)isr21, 0x08, 0x8E);
    idt_set_gate(22, (uint32_t)isr22, 0x08, 0x8E);
    idt_set_gate(23, (uint32_t)isr23, 0x08, 0x8E);
    idt_set_gate(24, (uint32_t)isr24, 0x08, 0x8E);
    idt_set_gate(25, (uint32_t)isr25, 0x08, 0x8E);
    idt_set_gate(26, (uint32_t)isr26, 0x08, 0x8E);
    idt_set_gate(27, (uint32_t)isr27, 0x08, 0x8E);
    idt_set_gate(28, (uint32_t)isr28, 0x08, 0x8E);
    idt_set_gate(29, (uint32_t)isr29, 0x08, 0x8E);
    idt_set_gate(30, (uint32_t)isr30, 0x08, 0x8E);
    idt_set_gate(31, (uint32_t)isr31, 0x08, 0x8E);
    //init every irq to dummy so no crashes but nothing happens
    for (int i = 32; i <= 47; i++) {
        idt_set_gate(i, (uint32_t)irq_dummy_handler, 0x08, 0x8E);
    }
    
    
    
    idt_set_gate(32, (uint32_t)irq0_handler, 0x08, 0x8E); // IRQ0 (timer)
    idt_set_gate(33, (uint32_t)irq1_handler, 0x08, 0x8E); /* IRQ1 (keyboard) */ init_keyboard();reset_input_buffer();
    idt_set_gate(44, (uint32_t)irq12_handler, 0x08, 0x8E); /* IRQ12 (mouse)*/init_mouse();
    idt_set_gate(0X80, (uint32_t)_syscall_int_80_handler, 0x08, 0xEE); /* IRQ80 (syscall)*/


    idt_flush((uint32_t)&idt_reg);
    
    Sys_Success("IDT set up successfully.\n");
}




