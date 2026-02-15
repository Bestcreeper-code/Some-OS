#ifndef ASM_H
#define ASM_H

#include <stdint.h>


static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outl(uint16_t port, uint32_t val) {
    asm volatile ("outl %0, %1" : : "a"(val), "Nd"(port));
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    asm volatile ("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}


static uintptr_t inline get_esp(void) {
    uintptr_t esp;
    asm volatile("mov %%esp, %0" : "=r"(esp));
    return esp;
}

typedef union {
    uint32_t val;
    char bytes[4];
    void* ptr;
    uint16_t halves[2];
} x32register_t;

typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp, esp;
    uint32_t eip, eflags;
    uint32_t cs, ds, es, fs, gs, ss;
    uint32_t cr0, cr2, cr3, cr4;
} __attribute__((packed)) cpu_registers_t;

typedef struct {
    uintptr_t top,bottom;
}  __attribute__((packed)) Stack_t;

static inline void capture_cpu_registers(cpu_registers_t* regs) {
    asm volatile(
        "movl %%eax, (%0)\n\t"
        "movl %%ebx, 4(%0)\n\t"
        "movl %%ecx, 8(%0)\n\t"
        "movl %%edx, 12(%0)\n\t"
        "movl %%esi, 16(%0)\n\t"
        "movl %%edi, 20(%0)\n\t"
        "movl %%ebp, 24(%0)\n\t"
        "movl %%esp, 28(%0)\n\t"

        "pushfl\n\t"
        "popl 32(%0)\n\t"

        "movw %%cs, 36(%0)\n\t"
        "movw %%ds, 38(%0)\n\t"
        "movw %%es, 40(%0)\n\t"
        "movw %%fs, 42(%0)\n\t"
        "movw %%gs, 44(%0)\n\t"
        "movw %%ss, 46(%0)\n\t"

        "call 1f\n\t"
        "1: popl 48(%0)\n\t"
        :
        : "r"(regs)
        : "memory"
    );
}




#endif // ASM_H
