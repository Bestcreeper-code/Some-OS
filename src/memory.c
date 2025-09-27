#include "headers/memory.h"
#include "headers/string.h"
#include "headers/multiboot_info.h"
#include "headers/io.h"
#include <stdint.h>
#include "data/globals.h"
#include "headers/Logger.h"
#include "headers/paging.h"

uint32_t max_mem;

static free_region_map_t region_map;
free_region_map_t* map = &region_map;


static inline free_region_map_t* get_free_region_map(void) {
    return map;
}

void force_alloc(uint32_t address, uint32_t size) {
    free_region_map_t* map = get_free_region_map();
    uint32_t end = address + size;
    uint32_t page_size = 0x1000;

    for (uint32_t page_addr = address & ~(page_size - 1); page_addr < end; page_addr += page_size) {
        PTE* pte = get_pte(page_addr);
        if (!pte) continue;

        pte->os_allocated = 1;

        uint32_t page_start = page_addr;
        uint32_t page_end = page_addr + page_size;

        if (address > page_start) {
            uint32_t free_start = page_start;
            uint32_t free_len = address - page_start;
            force_free(free_start, free_len);
        }

        if (end < page_end) {
            uint32_t free_start = end;
            uint32_t free_len = page_end - end;
            force_free(free_start, free_len);
        }
    }

    uint32_t lock_end = end;
    for (int i = 0; i < map->free_region_count; i++) {
        free_region_t* region = &map->free_regions[i];
        uint32_t region_end = region->base_addr + region->length;

        if (lock_end <= region->base_addr || address >= region_end) continue;

        if (address <= region->base_addr && lock_end >= region_end) {
            for (int j = i; j < map->free_region_count - 1; j++)
                map->free_regions[j] = map->free_regions[j + 1];
            map->free_region_count--;
            i--;
            continue;
        }

        if (address <= region->base_addr && lock_end < region_end) {
            region->length = region_end - lock_end;
            region->base_addr = lock_end;
            continue;
        }

        if (address > region->base_addr && lock_end >= region_end) {
            region->length = address - region->base_addr;
            continue;
        }

        if (address > region->base_addr && lock_end < region_end) {
            uint32_t first_len = address - region->base_addr;
            uint32_t second_base = lock_end;
            uint32_t second_len = region_end - lock_end;
            region->length = first_len;

            if (map->free_region_count < MAX_FREE_REGIONS) {
                for (int j = map->free_region_count; j > i + 1; j--)
                    map->free_regions[j] = map->free_regions[j - 1];
                map->free_regions[i + 1].base_addr = second_base;
                map->free_regions[i + 1].length = second_len;
                map->free_region_count++;
            }
            continue;
        }
    }
}

void force_free(uint32_t address, uint32_t size) {
    free_region_map_t* map = get_free_region_map();
    uint32_t new_start = address & ~0xFFFU; // align down to 4KB page
    uint32_t new_end   = (address + size + 0xFFFU) & ~0xFFFU; // align up

    for (int i = 0; i < map->free_region_count; i++) {
        free_region_t* region = &map->free_regions[i];
        uint32_t region_start = region->base_addr;
        uint32_t region_end   = region->base_addr + region->length;

        if (new_end >= region_start && new_start <= region_end) {
            uint32_t merged_start = (new_start < region_start) ? new_start : region_start;
            uint32_t merged_end   = (new_end > region_end) ? new_end : region_end;
            region->base_addr = merged_start;
            region->length = merged_end - merged_start;

            for (int j = 0; j < map->free_region_count; j++) {
                if (j == i) continue;
                free_region_t* other = &map->free_regions[j];
                uint32_t other_start = other->base_addr;
                uint32_t other_end   = other->base_addr + other->length;

                if (region->base_addr <= other_end && region->base_addr + region->length >= other_start) {
                    uint32_t merge_start = (region->base_addr < other_start) ? region->base_addr : other_start;
                    uint32_t merge_end   = ((region->base_addr + region->length) > other_end) ? (region->base_addr + region->length) : other_end;
                    region->base_addr = merge_start;
                    region->length = merge_end - merge_start;

                    for (int k = j; k < map->free_region_count - 1; k++) {
                        map->free_regions[k] = map->free_regions[k + 1];
                    }
                    map->free_region_count--;
                    if (j < i) i--;
                    j--;
                }
            }
            return;
        }
    }

    if (map->free_region_count < MAX_FREE_REGIONS) {
        map->free_regions[map->free_region_count].base_addr = new_start;
        map->free_regions[map->free_region_count].length = new_end - new_start;
        map->free_region_count++;
    }
}
void parse_memory_map(multiboot_info_t* mb_info) {
    free_region_map_t* map = get_free_region_map();
    map->free_region_count = 0;
    memset(map->free_regions, 0, sizeof(free_region_t) * MAX_FREE_REGIONS);

    if (!checkFlag(*mb_info, 6)) {
        Sys_log("Multiboot mmap not present\n");
        return;
    }

    uintptr_t mmap_end = mb_info->mmap_addr + mb_info->mmap_length;
    multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mb_info->mmap_addr;

    // Pre-allocate 3 kernel pages
    const int KERNEL_RESERVED_PAGES = 3;
    for (int i = 0; i < KERNEL_RESERVED_PAGES; i++) {
        uintptr_t page_addr = page_alloc(1);

        map->free_regions[map->free_region_count].base_addr = page_addr;
        map->free_regions[map->free_region_count].length = PAGE_SIZE;
        map->free_region_count++;

        PTE* pte = get_pte_for_pa(page_addr);
        if (pte) {
            pte->os_allocated = 1;        // mark as allocated
            pte->os_unallocatable = 0;
        }
    }

    // Mark all GRUB regions as unallocatable (bit 2)
    mmap = (multiboot_mmap_entry_t*)mb_info->mmap_addr;
    while ((uintptr_t)mmap < mmap_end) {
        uintptr_t base = mmap->addr & ~0xFFF;
        uintptr_t len  = (mmap->len + 0xFFF) & ~0xFFF;
        uintptr_t page_end = base + len;

        for (uintptr_t pa = base; pa < page_end; pa += PAGE_SIZE) {
            PTE* pte = get_pte_for_pa(pa);
            if (!pte) continue;
            pte->os_unallocatable = 1; // mark as reserved by GRUB
        }

        mmap = (multiboot_mmap_entry_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }

    Sys_log("Kernel memory regions (%d):\n", map->free_region_count);
    for (int i = 0; i < map->free_region_count; i++) {
        Sys_log("  Region %d: Base = 0x%x, Length = 0x%x\n",
                i,
                map->free_regions[i].base_addr,
                map->free_regions[i].length);
    }

    force_alloc((uint32_t)map, sizeof(free_region_map_t));
}



free_region_t* FirstRegionOfSizeOrMore(size_t size) {
    free_region_map_t* map = get_free_region_map();
    uint32_t requestedSize = (uint32_t)((size + sizeof(uint32_t) + 7) & ~7U);

    for (int i = 0; i < map->free_region_count; i++) {
        if (map->free_regions[i].length == 0) continue;

        uint32_t currBase = map->free_regions[i].base_addr;
        uint32_t currSize = map->free_regions[i].length;

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
    Sys_log("Free regions:\n");
    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (map->free_regions[i].length > 0) {
            Sys_log("[%d] base: 0x%x, size: %u\n", i, map->free_regions[i].base_addr, map->free_regions[i].length);
        }
    }
}

uint32_t get_pter_size(void* pter){
    uint8_t* raw = (uint8_t*)pter;
    uint32_t* sizeaddr = (uint32_t*)(raw - sizeof(uint32_t));
    return *sizeaddr;
}
void* malloc_impl(size_t size) {
    if (size == 0) return NULL;

    free_region_map_t* map = get_free_region_map();
    if (!map) return NULL;

    if (size > UINT32_MAX - sizeof(uint32_t)) return NULL; // prevent overflow
    uint32_t full_size = (uint32_t)((size + sizeof(uint32_t) + 7) & ~7U);

    free_region_t* region = FirstRegionOfSizeOrMore(full_size);

    if (!region) {
        uint32_t pages_needed = (full_size + PAGE_SIZE - 1) / PAGE_SIZE;
        uintptr_t base = page_alloc(pages_needed);
        if (!base) {
            Sys_log("malloc failed(not enough pages): %u bytes\n", (unsigned)size);
            return NULL;
        }

        force_free((uint32_t)base, pages_needed * PAGE_SIZE);
        region = FirstRegionOfSizeOrMore(full_size);
        if (!region) return NULL;
    }

    uint32_t* header = (uint32_t*)(uintptr_t)region->base_addr;
    *header = (uint32_t)size;

    region->base_addr += full_size;
    region->length -= full_size;
    if (region->length == 0) {
        int idx = (int)(region - map->free_regions);
        for (int i = idx; i < map->free_region_count - 1; i++)
            map->free_regions[i] = map->free_regions[i + 1];
        map->free_region_count--;
    }

#if DEBUG_MODE
    Sys_log("malloc %u bytes at %p\n", (unsigned)full_size, header + 1);
#endif

    return (void*)(header + 1);
}



void free_impl(void* _Memory) {
    if (!_Memory) return;

    free_region_map_t* map = get_free_region_map();

    uintptr_t address = (uintptr_t)_Memory;
    uint32_t* sizeptr = (uint32_t*)(address - sizeof(uint32_t));
    uint32_t size = *sizeptr + sizeof(uint32_t);
    size = (size + 7) & ~7U;

    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (map->free_regions[i].length == 0) {
            map->free_regions[i].base_addr = (uint32_t)(address - sizeof(uint32_t));
            map->free_regions[i].length = size;
#if DEBUG_MODE
            Sys_log("freeing %u bytes at %p\n", size, _Memory);
#endif
            break;
        }
    }

#ifndef FREE_MERGES_MMAP_BLOCKS
    // Sort regions by base_addr
    for (int i = 0; i < MAX_FREE_REGIONS - 1; i++) {
        for (int j = i + 1; j < MAX_FREE_REGIONS; j++) {
            if (map->free_regions[j].length == 0) continue;
            if (map->free_regions[i].length == 0 || map->free_regions[j].base_addr < map->free_regions[i].base_addr) {
                free_region_t tmp = map->free_regions[i];
                map->free_regions[i] = map->free_regions[j];
                map->free_regions[j] = tmp;
            }
        }
    }

    // Merge adjacent regions
    for (int i = 0; i < MAX_FREE_REGIONS - 1; i++) {
        if (map->free_regions[i].length == 0) continue;

        for (int j = i + 1; j < MAX_FREE_REGIONS; j++) {
            if (map->free_regions[j].length == 0) continue;

            uint32_t end_i = map->free_regions[i].base_addr + map->free_regions[i].length;
            if (end_i == map->free_regions[j].base_addr) {
                map->free_regions[i].length += map->free_regions[j].length;

                for (int k = j; k < MAX_FREE_REGIONS - 1; k++)
                    map->free_regions[k] = map->free_regions[k + 1];

                map->free_regions[MAX_FREE_REGIONS - 1].base_addr = 0;
                map->free_regions[MAX_FREE_REGIONS - 1].length = 0;
                j--;
            }
        }
    }
#endif //FREE_MERGES_MMAP_BLOCKS

    // Recalculate free_region_count
    map->free_region_count = 0;
    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (map->free_regions[i].length > 0)
            map->free_region_count++;
    }
    
}


void* realloc_impl(void *ptr, size_t size) {
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

    uint32_t old_len = get_pter_size(ptr);
    size_t copy_size = (size < old_len) ? size : old_len;

    memcpy(new_ptr, ptr, copy_size);
    free(ptr);

    return new_ptr;
}

void* aligned_malloc(size_t size, size_t alignment) {
    if ((alignment & (alignment - 1)) != 0) {
        Sys_log("Error: alignment must be power of two\n");
        return NULL;
    }

    uint32_t extra = (uint32_t)(alignment - 1 + sizeof(uint32_t));
    void* raw = malloc_impl(size + extra);
    if (!raw) return NULL;

    uintptr_t raw_addr = (uintptr_t)raw;
    uintptr_t aligned_addr = (raw_addr + sizeof(uint32_t) + alignment - 1) & ~(alignment - 1);

    ((uint32_t*)aligned_addr)[-1] = (uint32_t)raw_addr;

    return (void*)aligned_addr;
}

void aligned_free(void* ptr) {
    if (!ptr) return;

    uintptr_t aligned_addr = (uintptr_t)ptr;
    void* raw = (void*)(uintptr_t)((uint32_t*)((uintptr_t)aligned_addr))[-1];

    free_impl(raw);
}
