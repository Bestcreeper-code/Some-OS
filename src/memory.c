#include "headers/memory.h"
#include "headers/string.h"
#include "headers/multiboot_info.h"
#include "headers/io.h"
#include <stdint.h>
#include "data/globals.h"

#define FREE_REGION_MAP_ADDR 0x2710  

static inline free_region_map_t* get_free_region_map(void) {
    return (free_region_map_t*)FREE_REGION_MAP_ADDR;
}

void force_alloc(uint64_t adress, uint64_t size) {
    free_region_map_t* map = get_free_region_map();
    uint64_t lock_end = adress + size;

    for (int i = 0; i < map->free_region_count; i++) {
        free_region_t* region = &map->free_regions[i];
        uint64_t region_end = region->base_addr + region->length;

        // If no overlap, continue
        if (lock_end <= region->base_addr || adress >= region_end) {
            continue;
        }

        // Full overlap: locked region covers whole free region -> remove free region
        if (adress <= region->base_addr && lock_end >= region_end) {

            for (int j = i; j < map->free_region_count - 1; j++) {
                map->free_regions[j] = map->free_regions[j + 1];
            }
            map->free_region_count--;
            i--; 
            continue;
        }

        // Partial overlaps: adjust free region boundaries

        // Locked region overlaps start of free region
        if (adress <= region->base_addr && lock_end < region_end) {
            uint64_t new_base = lock_end;
            uint64_t new_length = region_end - lock_end;
            region->base_addr = new_base;
            region->length = new_length;
            continue;
        }

        // Locked region overlaps end of free region
        if (adress > region->base_addr && lock_end >= region_end) {
            uint64_t new_length = adress - region->base_addr;
            region->length = new_length;
            continue;
        }

        // Locked region is inside free region, splitting it into two parts
        if (adress > region->base_addr && lock_end < region_end) {
            // Shrink current region to before locked region
            uint64_t first_part_len = adress - region->base_addr;

            // Create new region for after locked region
            uint64_t second_part_base = lock_end;
            uint64_t second_part_len = region_end - lock_end;

            region->length = first_part_len;

            // Insert new free region after current
            if (map->free_region_count < MAX_FREE_REGIONS) {
                for (int j = map->free_region_count; j > i + 1; j--) {
                    map->free_regions[j] = map->free_regions[j - 1];
                }
                map->free_regions[i + 1].base_addr = second_part_base;
                map->free_regions[i + 1].length = second_part_len;
                map->free_region_count++;
            }
            // else: no space to add second part, just discard trailing free memory

            continue;
        }
    }
}


void parse_memory_map(multiboot_info_t* mb_info) {
    free_region_map_t* map = get_free_region_map();
    force_alloc(FREE_REGION_MAP_ADDR,sizeof(free_region_map_t));
    map->free_region_count = 0;
    memset(map->free_regions, 0, sizeof(free_region_t) * MAX_FREE_REGIONS);
    
    if (!(checkFlag(*mb_info, 6))) {
        printf("bork");
        return;
    }

    uintptr_t mmap_end = mb_info->mmap_addr + mb_info->mmap_length;
    multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mb_info->mmap_addr;

    while ((uintptr_t)mmap < mmap_end) {
        if (mmap->type == MULTIBOOT_MMAP_FREE_MEMORY) {
            if (map->free_region_count < MAX_FREE_REGIONS) {
                map->free_regions[map->free_region_count].base_addr = mmap->addr;
                map->free_regions[map->free_region_count].length = mmap->len; 
                map->free_region_count++;
                printf("add: %p ||| len %llu\n",mmap->addr,mmap->len);
            }
            
        }
        mmap = (multiboot_mmap_entry_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }
    
    printf("Free memory regions (%d):\n", map->free_region_count);
    for (int i = 0; i < map->free_region_count; i++) {
        printf("  Region %d: Base = 0x%x, Length = 0x%x (%u bytes)\n",
               i,
               map->free_regions[i].base_addr,
               map->free_regions[i].length,
               map->free_regions[i].length
            );
    }
}

free_region_t* FirstRegionOfSizeOrMore(size_t size) {
    free_region_map_t* map = get_free_region_map();
    uint64_t requestedSize = (size + sizeof(uint64_t) + 7) & ~7ULL;

    for (int i = 0; i < map->free_region_count; i++) {
        if (map->free_regions[i].length == 0) continue;

        uint64_t currBase = map->free_regions[i].base_addr;
        uint64_t currSize = map->free_regions[i].length;

        bool merged[MAX_FREE_REGIONS] = {false};
        merged[i] = true;

        bool changed;
        do {
            changed = false;
            for (int j = 0; j < map->free_region_count; j++) {
                if (merged[j] || map->free_regions[j].length == 0) continue;
                if (map->free_regions[j].base_addr == currBase + currSize) {
                    currSize += map->free_regions[j].length;
                    merged[j] = true;
                    changed = true;
                }
            }
        } while (changed);

        if (currSize >= requestedSize) {
            for (int j = 0; j < map->free_region_count; j++) {
                if (merged[j] && j != i) {
                    memset(&map->free_regions[j], 0, sizeof(free_region_t));
                }
            }

            map->free_regions[i].length = currSize;
            return &map->free_regions[i];
        }
    }

    return NULL;
}

void print_free_regions() {
    free_region_map_t* map = get_free_region_map();
    printf("Free regions:\n");
    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (map->free_regions[i].length > 0) {
            printf("[%d] base: 0x%p, size: %llu\n", i, map->free_regions[i].base_addr, map->free_regions[i].length);
        }
    }
}

uint64_t get_pter_size(void* pter){
    uint8_t* raw = (uint8_t*)pter;
    uint64_t* sizeaddr = (uint64_t*)(raw - sizeof(uint64_t));
    return *sizeaddr;
}

void* malloc(size_t _Size) {
    free_region_map_t* map = get_free_region_map();
    size_t full_size = (_Size + sizeof(uint64_t) + 7) & ~7ULL;
    free_region_t* region = FirstRegionOfSizeOrMore(full_size);
    if (region == NULL || region->length < full_size) {
        printf("Couldn't find enough memory for %u bytes\n", (unsigned)_Size);
        print_free_regions();
        return NULL;
    }

    uint64_t* data = (uint64_t*)(uintptr_t)(region->base_addr);
    data[0] = _Size;
    region->length -= full_size;
    region->base_addr += full_size;

#if DEBUG_MODE == true
    printf("allocated %u bytes at %p\n", (unsigned)full_size, data);
#endif

    return (void*)&data[1];
}

void free(void* _Memory) {
    if (!_Memory) return;

    free_region_map_t* map = get_free_region_map();

    uintptr_t address = (uintptr_t)_Memory;
    uint64_t* sizeptr = (uint64_t*)(address - sizeof(uint64_t));
    uint64_t size = *sizeptr + sizeof(uint64_t);
    size = (size + 7) & ~7ULL;

    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (map->free_regions[i].length == 0) {
            map->free_regions[i].base_addr = ((uint64_t)address - sizeof(uint64_t));
            map->free_regions[i].length = size;
#if DEBUG_MODE == true
            printf("freeing %llu at %p\n", size, _Memory);
#endif
            break;
        }
    }

    // Sort regions by base_addr
    for (int i = 0; i < MAX_FREE_REGIONS - 1; i++) {
        for (int j = i + 1; j < MAX_FREE_REGIONS; j++) {
            if (map->free_regions[j].base_addr != 0 && map->free_regions[i].base_addr != 0 &&
                map->free_regions[j].base_addr < map->free_regions[i].base_addr) {
                uint64_t temp_addr = map->free_regions[i].base_addr;
                uint64_t temp_len = map->free_regions[i].length;

                map->free_regions[i].base_addr = map->free_regions[j].base_addr;
                map->free_regions[i].length = map->free_regions[j].length;

                map->free_regions[j].base_addr = temp_addr;
                map->free_regions[j].length = temp_len;
            }
        }
    }

    // Merge adjacent regions
    for (int i = 0; i < MAX_FREE_REGIONS - 1; i++) {
        if (map->free_regions[i].base_addr == 0) continue;
        for (int j = i + 1; j < MAX_FREE_REGIONS; j++) {
            if (map->free_regions[j].base_addr == 0) continue;

            uint64_t end_i = map->free_regions[i].base_addr + map->free_regions[i].length;
            if (end_i == map->free_regions[j].base_addr) {
                map->free_regions[i].length += map->free_regions[j].length;

                for (int k = j; k < MAX_FREE_REGIONS - 1; k++) {
                    map->free_regions[k] = map->free_regions[k + 1];
                }
                map->free_regions[MAX_FREE_REGIONS - 1].base_addr = 0;
                map->free_regions[MAX_FREE_REGIONS - 1].length = 0;
                j--;
            }
        }
    }

    // Recalculate free_region_count
    map->free_region_count = 0;
    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (map->free_regions[i].length > 0)
            map->free_region_count++;
    }
}

void* realloc(void *ptr, size_t size) {
    if (size == 0) {
        free(ptr);
        return NULL;
    }

    if (!ptr) {
        return malloc(size);
    }

    void *new_ptr = malloc(size);
    if (!new_ptr) {
        return NULL;
    }

    uint64_t old_len = get_pter_size(ptr);
    size_t copy_size = (size < old_len) ? size : old_len;

    memcpy(new_ptr, ptr, copy_size);
    free(ptr);

    return new_ptr;
}
