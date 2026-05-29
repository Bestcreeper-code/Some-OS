#ifndef ARCH_ASM_H
#define ARCH_ASM_H

#include <stddef.h>
#include <stdint.h>










size_t inline get_stack_ptr(void) {
    size_t rsp;
    asm volatile("mov %%rsp, %0" : "=r"(rsp));
    return rsp;
}

typedef union {
    uint64_t val;
    uint32_t halves[2];
    uint16_t quarters[4];
    char bytes[8];

    void* ptr;
} x64register_t;

typedef struct {
    uintptr_t top,bottom;
    size_t size;
}  __attribute__((packed)) Stack_t;

typedef struct {
    uint64_t rax, rbx, rcx, rdx;
    uint64_t rsi, rdi, rbp, rsp;
    uint64_t r8,  r9,  r10, r11;
    uint64_t r12, r13, r14, r15;

    uint64_t rip, rflags;

    uint16_t cs, ds, es, fs, gs, ss;

    uint64_t cr0, cr2, cr3, cr4, cr8;
} __attribute__((packed)) cpu_registers_t;


inline void capture_cpu_registers(cpu_registers_t* regs) {
    asm volatile(
        "mov %%rax, 0(%0)\n\t"
        "mov %%rbx, 8(%0)\n\t"
        "mov %%rcx, 16(%0)\n\t"
        "mov %%rdx, 24(%0)\n\t"
        "mov %%rsi, 32(%0)\n\t"
        "mov %%rdi, 40(%0)\n\t"
        "mov %%rbp, 48(%0)\n\t"
        "mov %%rsp, 56(%0)\n\t"

        "mov %%r8,  64(%0)\n\t"
        "mov %%r9,  72(%0)\n\t"
        "mov %%r10, 80(%0)\n\t"
        "mov %%r11, 88(%0)\n\t"
        "mov %%r12, 96(%0)\n\t"
        "mov %%r13, 104(%0)\n\t"
        "mov %%r14, 112(%0)\n\t"
        "mov %%r15, 120(%0)\n\t"

        "pushfq\n\t"
        "popq 128(%0)\n\t"

        "mov %%cs, 136(%0)\n\t"
        "mov %%ds, 138(%0)\n\t"
        "mov %%es, 140(%0)\n\t"
        "mov %%fs, 142(%0)\n\t"
        "mov %%gs, 144(%0)\n\t"
        "mov %%ss, 146(%0)\n\t"

        "lea (%%rip), %%rax\n\t"
        "mov %%rax, 152(%0)\n\t"

        "mov %%cr0, %%rax\n\t"
        "mov %%rax, 160(%0)\n\t"

        "mov %%cr2, %%rax\n\t"
        "mov %%rax, 168(%0)\n\t"

        "mov %%cr3, %%rax\n\t"
        "mov %%rax, 176(%0)\n\t"

        "mov %%cr4, %%rax\n\t"
        "mov %%rax, 184(%0)\n\t"

        "mov %%cr8, %%rax \n\t"
        "mov %%rax, 192(%0)\n\t"

        :
        : "r"(regs)
        : "rax", "memory"
    );
}



#endif // ARCH_ASM_H
