#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <stddef.h>
#include "multiboot_info.h"
#include "Logger.h"
#include "../data/globals.h"
#include "addresses.h"

#define MULTIBOOT_MMAP_FREE_MEMORY  1
#define MULTIBOOT_MMAP_RESERVED     2

#define MAX_FREE_REGIONS 128

extern uint32_t max_mem;

typedef struct {
    uint64_t base_addr;
    uint64_t length;
} __attribute__((packed)) free_region_t;

typedef struct {
    int free_region_count;
    free_region_t free_regions[MAX_FREE_REGIONS];
} __attribute__((packed)) free_region_map_t;

void parse_memory_map(multiboot_info_t* mb_info);

void force_alloc(uint32_t address, uint32_t size);
void force_free(uint32_t address, uint32_t size);
uint32_t get_pter_size(void* pter);

void* aligned_malloc(size_t size, size_t alignment);
void aligned_free(void* ptr);

void* malloc_impl(size_t size);
void free_impl(void* ptr);
void* realloc_impl(void* ptr, size_t size);

#if DEBUG_MODE == 1

#define malloc(size)        (Sys_log("[Debug] called malloc\n"), malloc_impl(size))
#define free(ptr)           (Sys_log("[Debug] called free\n"), free_impl(ptr))
#define realloc(ptr, size)  (Sys_log("[Debug] called realloc\n"), realloc_impl(ptr, size))

#else

#define malloc              malloc_impl
#define free                free_impl
#define realloc             realloc_impl

#endif // DEBUG_MODE

#endif // MEMORY_H
