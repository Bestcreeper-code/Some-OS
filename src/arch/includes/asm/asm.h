#pragma once 
#include <stdint.h>
#include "arch_asm.h"



#ifdef  __ARCH_X86__
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

static inline void invlpg(uint32_t index) {
    asm volatile("invlpg (%0)" : : "r"(index) : "memory");
}

static inline void cli(){
    asm volatile("cli");
} 

static inline void sti(){
    asm volatile("sti");
} 

#else
#error unsupported arch
#endif


static inline uintptr_t get_instruction_pointer() {
    uintptr_t ip;

#ifdef __ARCH_X86__
    __asm__ volatile (
        "call 1f\n\t"
        "1: pop %0"
        : "=r"(ip)
        :
        : "memory"
    );
#else
    #error missing arch implementation
#endif
    return ip;
}

