
#pragma once
#include <stdint.h>

/* 64-bit interrupt gate — 16 bytes */
struct IDTEntry64 {
    uint16_t offset_0_15;
    uint16_t selector;
    uint8_t  ist;        /* bits 2:0 = IST index (0 = legacy stack switch) */
    uint8_t  type_attr;  /* 0x8E = present, DPL=0, 64-bit interrupt gate */
    uint16_t offset_16_31;
    uint32_t offset_32_63;
    uint32_t reserved;
} __attribute__((packed));

struct IDTPtr64 {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));