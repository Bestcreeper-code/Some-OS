#include <stdint.h>
#include "multiboot_info.h"
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

void* malloc(size_t _Size);
void free(void *_Memory);
void *realloc(void *ptr, size_t size);