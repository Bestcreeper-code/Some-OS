#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "multiboot_info.h"
#include "Logger.h"
#include "../data/globals.h"
#include "kernel_data.h"

#define MULTIBOOT_MMAP_FREE_MEMORY  1
#define MULTIBOOT_MMAP_RESERVED     2

#define MAX_FREE_REGIONS 128

typedef struct {
    uint32_t base_addr;
    uint32_t length;
} __attribute__((packed)) free_region_t;

typedef struct {
    int free_region_count;
    free_region_t free_regions[MAX_FREE_REGIONS];
} __attribute__((packed)) free_region_map_t;

extern uint32_t max_mem;
extern free_region_map_t* k_mmap;

void parse_memory_map(multiboot_info_t* mb_info);

void force_alloc(uint32_t address, uint32_t size);
void force_free(uint32_t address, uint32_t size);
uint32_t get_pter_size(void* pter);

void* aligned_malloc(size_t size, size_t alignment);
void aligned_free(void* ptr);

void* malloc_impl(size_t size);
void free_impl(void* ptr);
void* realloc_impl(void* ptr, size_t size);


#define malloc(size)              malloc_impl(size)//;Sys_log("\n");
#define free(ptr)                free_impl(ptr)//;Sys_log("\n");
#define realloc             realloc_impl

#endif // MEMORY_H
