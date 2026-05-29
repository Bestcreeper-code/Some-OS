#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "Logger.h"
#include "../config/config.h"
#include "kernel_data.h"


#define MULTIBOOT_MMAP_FREE_MEMORY  1
#define MULTIBOOT_MMAP_RESERVED     2

#define MAX_FREE_REGIONS 128

typedef struct {
    uintptr_t base_addr;
    size_t length;
} __attribute__((packed)) free_region_t;

typedef struct {
    int free_region_count;
    free_region_t free_regions[MAX_FREE_REGIONS];
} __attribute__((packed)) free_region_map_t;


extern volatile size_t ram_amount;
extern free_region_map_t* k_mmap;

int parse_memory_map();

void force_alloc(uintptr_t address, size_t size);
void force_free(uintptr_t address, size_t size);
size_t get_pter_size(void* pter) __attribute__((nonnull (1)));

void aligned_free(void* ptr)  __attribute__((nonnull (1)));;
void* aligned_malloc(size_t size, size_t alignment) __attribute__ ((malloc, malloc(aligned_free, 1)));

void kfree_impl(void* ptr) __attribute__((nonnull (1)));
void* kmalloc_impl(size_t size) __attribute__ ((malloc, malloc (kfree_impl, 1)));
void* krealloc_impl(void* ptr, size_t size);



#define kmalloc(size)              kmalloc_impl(size)//;Sys_log("\n");
#define kfree(ptr)                 kfree_impl(ptr)//;Sys_log("\n");
#define krealloc             krealloc_impl

#endif // MEMORY_H
