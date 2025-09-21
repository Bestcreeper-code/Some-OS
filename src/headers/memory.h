#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include "multiboot_info.h"
#include "Logger.h"
#include "../data/globals.h"
#include <stddef.h>
#include "addresses.h"

#define MULTIBOOT_MMAP_FREE_MEMORY  1
#define MULTIBOOT_MMAP_RESERVED     2

#define MAX_FREE_REGIONS 128

// #define FREE_REGION_MAP 0x2710  

typedef struct {
    uint64_t base_addr;
    uint64_t length;
} __attribute__((packed)) free_region_t;

typedef struct {
    int free_region_count;
    free_region_t free_regions[MAX_FREE_REGIONS];
} __attribute__((packed)) free_region_map_t;


void parse_memory_map(multiboot_info_t* mb_info);

void force_alloc(uint64_t adress, uint64_t size);
void force_free(uint64_t address, uint64_t size);
uint64_t get_pter_size(void* pter);

void* aligned_malloc(size_t size, size_t alignment);
void aligned_free(void* ptr);

// debuggable funcs
void* malloc_impl(size_t _Size);
void free_impl(void *_Memory);
void *realloc_impl(void *ptr, size_t size);



#if DEBUG_MODE == 1


#define malloc(size)        (Sys_log("[Debug] called malloc"), malloc_impl(size))
#define free(ptr)           (Sys_log("[Debug] called free"), free_impl(ptr))
#define realloc(ptr, size)  (Sys_log("[Debug] called realloc"), realloc_impl(ptr, size))

#else

#define malloc              malloc_impl
#define free                free_impl
#define realloc             realloc_impl

#endif //-DEBUG

#endif //-MEMORY_H