#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>

#define PD_PHYS_ADDR 0x100000
#define MIN_OS_PAGES 8192

typedef struct {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read/write (0 = read-only, 1 = read/write)
    uint32_t user       : 1;   // User/supervisor (0 = supervisor only, 1 = user)
    uint32_t pwt        : 1;   // Page-level write-through
    uint32_t pcd        : 1;   // Page-level cache disable
    uint32_t accessed   : 1;   // Accessed
    uint32_t reserved   : 1;   // Reserved (set to 0)
    uint32_t page_size  : 1;   // Page size (0 = 4KB, 1 = 4MB)
    uint32_t ignored    : 1;   // Ignored by CPU
    uint32_t available  : 3;   // Available for OS use
    uint32_t addr       : 20;  // Physical address of the page table (aligned to 4KB)
} __attribute__((packed)) PDE;

typedef struct {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read/write (0 = read-only, 1 = read/write)
    uint32_t user       : 1;   // User/supervisor (0 = supervisor only, 1 = user)
    uint32_t pwt        : 1;   // Page-level write-through
    uint32_t pcd        : 1;   // Page-level cache disable
    uint32_t accessed   : 1;   // Accessed
    uint32_t dirty      : 1;   // Dirty (has been written to)
    uint32_t pat        : 1;   // Page Attribute Table
    uint32_t global     : 1;   // Global page
    uint32_t available  : 3;   // Ignored bits
    uint32_t addr       : 20;  // Physical address of the 4KB page frame
} __attribute__((packed)) PTE;

int setup_paging();

#endif // PAGING_H
