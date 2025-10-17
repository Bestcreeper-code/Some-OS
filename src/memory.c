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
free_region_map_t* k_mmap = &region_map;


static inline free_region_map_t* get_free_region_map(void) {
    return k_mmap;
}

void force_alloc(uint32_t address, uint32_t size) {
    free_region_map_t* k_mmap = get_free_region_map();
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
    for (int i = 0; i < k_mmap->free_region_count; i++) {
        free_region_t* region = &k_mmap->free_regions[i];
        uint32_t region_end = region->base_addr + region->length;

        if (lock_end <= region->base_addr || address >= region_end) continue;

        if (address <= region->base_addr && lock_end >= region_end) {
            for (int j = i; j < k_mmap->free_region_count - 1; j++)
                k_mmap->free_regions[j] = k_mmap->free_regions[j + 1];
            k_mmap->free_region_count--;
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

            if (k_mmap->free_region_count < MAX_FREE_REGIONS) {
                for (int j = k_mmap->free_region_count; j > i + 1; j--)
                    k_mmap->free_regions[j] = k_mmap->free_regions[j - 1];
                k_mmap->free_regions[i + 1].base_addr = second_base;
                k_mmap->free_regions[i + 1].length = second_len;
                k_mmap->free_region_count++;
            }
            continue;
        }
    }
}

void force_free(uint32_t address, uint32_t size) {
    free_region_map_t* k_mmap = get_free_region_map();
    uint32_t new_start = address & ~0xFFFU; // align down to 4KB page
    uint32_t new_end   = (address + size + 0xFFFU) & ~0xFFFU; // align up

    for (int i = 0; i < k_mmap->free_region_count; i++) {
        free_region_t* region = &k_mmap->free_regions[i];
        uint32_t region_start = region->base_addr;
        uint32_t region_end   = region->base_addr + region->length;

        if (new_end >= region_start && new_start <= region_end) {
            uint32_t merged_start = (new_start < region_start) ? new_start : region_start;
            uint32_t merged_end   = (new_end > region_end) ? new_end : region_end;
            region->base_addr = merged_start;
            region->length = merged_end - merged_start;

            for (int j = 0; j < k_mmap->free_region_count; j++) {
                if (j == i) continue;
                free_region_t* other = &k_mmap->free_regions[j];
                uint32_t other_start = other->base_addr;
                uint32_t other_end   = other->base_addr + other->length;

                if (region->base_addr <= other_end && region->base_addr + region->length >= other_start) {
                    uint32_t merge_start = (region->base_addr < other_start) ? region->base_addr : other_start;
                    uint32_t merge_end   = ((region->base_addr + region->length) > other_end) ? (region->base_addr + region->length) : other_end;
                    region->base_addr = merge_start;
                    region->length = merge_end - merge_start;

                    for (int k = j; k < k_mmap->free_region_count - 1; k++) {
                        k_mmap->free_regions[k] = k_mmap->free_regions[k + 1];
                    }
                    k_mmap->free_region_count--;
                    if (j < i) i--;
                    j--;
                }
            }
            return;
        }
    }

    if (k_mmap->free_region_count < MAX_FREE_REGIONS) {
        k_mmap->free_regions[k_mmap->free_region_count].base_addr = new_start;
        k_mmap->free_regions[k_mmap->free_region_count].length = new_end - new_start;
        k_mmap->free_region_count++;
    }
}

void parse_memory_map(multiboot_info_t* mb_info) {
    free_region_map_t* k_mmap = get_free_region_map();
    //cleanup the k_mmap
    k_mmap->free_region_count = 0;
    memset(k_mmap->free_regions, 0, sizeof(free_region_t) * MAX_FREE_REGIONS);

    if (!checkFlag(*mb_info, 6)) {
        Sys_log("Multiboot mmap not present\n");
        return;
    }

    uintptr_t mmap_end = mb_info->mmap_addr + mb_info->mmap_length;
    multiboot_mmap_entry_t* mmap = (multiboot_mmap_entry_t*)mb_info->mmap_addr;

    
    // Mark all GRUB regions as unallocatable (bit 2)
    mmap = (multiboot_mmap_entry_t*)mb_info->mmap_addr;
    while ((uintptr_t)mmap < mmap_end) {
        uintptr_t base = mmap->addr & ~0xFFF;
        uintptr_t len  = (mmap->len + 0xFFF) & ~0xFFF;
        uintptr_t page_end = base + len;
        
        // Only mark non-available memory (type != 1) as unallocatable
        if (mmap->type != 1) {
            for (uintptr_t pa = base; pa < page_end; pa += _PAGE_SIZE) {
                PTE* pte = get_pte_for_pa(pa);
                if (!pte) continue;
                pte->os_unallocatable = 1;
            }
        }
        
        mmap = (multiboot_mmap_entry_t*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    }
    
    // Pre-allocate 3 kernel pages
    const int KERNEL_RESERVED_PAGES = 3;
    for (int i = 0; i < KERNEL_RESERVED_PAGES; i++) {
        uintptr_t page_addr = page_alloc(1,1,0);

        k_mmap->free_regions[k_mmap->free_region_count].base_addr = page_addr;
        k_mmap->free_regions[k_mmap->free_region_count].length = _PAGE_SIZE;
        k_mmap->free_region_count++;

        
    }
    
    Sys_log("Kernel memory regions (%d):\n", k_mmap->free_region_count);
    for (int i = 0; i < k_mmap->free_region_count; i++) {
        Sys_log("  Region %d: Base = 0x%x, Length = 0x%x\n",
                i,
                k_mmap->free_regions[i].base_addr,
                k_mmap->free_regions[i].length);
    }

    force_alloc((uint32_t)k_mmap, sizeof(free_region_map_t));
}



free_region_t* FirstRegionOfSizeOrMore(size_t size) {
    free_region_map_t* k_mmap = get_free_region_map();
    uint32_t requestedSize = (uint32_t)((size + sizeof(uint32_t) + 7) & ~7U);

    for (int i = 0; i < k_mmap->free_region_count; i++) {
        if (k_mmap->free_regions[i].length == 0) continue;

        uint32_t currBase = k_mmap->free_regions[i].base_addr;
        uint32_t currSize = k_mmap->free_regions[i].length;

        bool merged[MAX_FREE_REGIONS] = {false};
        merged[i] = true;

        bool changed;
        do {
            changed = false;
            for (int j = 0; j < k_mmap->free_region_count; j++) {
                if (merged[j] || k_mmap->free_regions[j].length == 0) continue;

                if (k_mmap->free_regions[j].base_addr == currBase + currSize) {
                    currSize += k_mmap->free_regions[j].length;
                    merged[j] = true;
                    changed = true;
                }
            }
        } while (changed);

        if (currSize >= requestedSize) {
            for (int j = 0; j < k_mmap->free_region_count; j++) {
                if (merged[j] && j != i) {
                    memset(&k_mmap->free_regions[j], 0, sizeof(free_region_t));
                }
            }

            k_mmap->free_regions[i].length = currSize;
            return &k_mmap->free_regions[i];
        }
    }

    return NULL;
}


void print_free_regions() {
    free_region_map_t* k_mmap = get_free_region_map();
    Sys_log("Free regions:\n");
    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (k_mmap->free_regions[i].length > 0) {
            Sys_log("[%d] base: 0x%x, size: %u\n", i, k_mmap->free_regions[i].base_addr, k_mmap->free_regions[i].length);
        }
    }
}

uint32_t get_pter_size(void* pter){
    uint8_t* raw = (uint8_t*)pter;
    uint32_t* sizeaddr = (uint32_t*)(raw - sizeof(uint32_t));
    return *sizeaddr;
}

void* malloc_impl(size_t size) {
#if MEM_DEBUG_MODE
    Sys_log("[MEM_DBG] this func was called with %d\n",size);
#endif
    if (size == 0) return NULL;

    free_region_map_t* k_mmap = get_free_region_map();
    if (!k_mmap) return NULL;

    if (size > UINT32_MAX - sizeof(uint32_t)) return NULL; // prevent overflow
    uint32_t full_size = (uint32_t)((size + sizeof(uint32_t) + 7) & ~7U);

    free_region_t* region = FirstRegionOfSizeOrMore(full_size);

    if (!region) {
        uint32_t pages_needed = (full_size + _PAGE_SIZE - 1) / _PAGE_SIZE;
        uintptr_t base = page_alloc(pages_needed,1,0);
        if (!base) {
            Sys_log("malloc failed(not enough pages): %u bytes\n", (unsigned)size);
            return NULL;
        }

        force_free((uint32_t)base, pages_needed * _PAGE_SIZE);
        region = FirstRegionOfSizeOrMore(full_size);
        if (!region) return NULL;
    }

    uint32_t* header = (uint32_t*)(uintptr_t)region->base_addr;
    *header = (uint32_t)size;

    region->base_addr += full_size;
    region->length -= full_size;
    if (region->length == 0) {
        int idx = (int)(region - k_mmap->free_regions);
        for (int i = idx; i < k_mmap->free_region_count - 1; i++)
            k_mmap->free_regions[i] = k_mmap->free_regions[i + 1];
        k_mmap->free_region_count--;
    }

#if MEM_DEBUG_MODE
    Sys_log("malloc'ed %u bytes at %p\n", (unsigned)full_size, header + 1);
#endif

    return (void*)(header + 1);
}



void free_impl(void* _Memory) {
#if MEM_DEBUG_MODE
    Sys_log("[MEM_DBG] this func was called with %d\n",_Memory);
#endif
    if (!_Memory) return;

    free_region_map_t* k_mmap = get_free_region_map();

    uintptr_t address = (uintptr_t)_Memory;
    uint32_t* sizeptr = (uint32_t*)(address - sizeof(uint32_t));
    uint32_t size = *sizeptr + sizeof(uint32_t);
    size = (size + 7) & ~7U;

    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (k_mmap->free_regions[i].length == 0) {
            k_mmap->free_regions[i].base_addr = (uint32_t)(address - sizeof(uint32_t));
            k_mmap->free_regions[i].length = size;
#if MEM_DEBUG_MODE
            Sys_log("freeing %u bytes at %p\n", size, _Memory);
#endif
            break;
        }
    }

#ifndef FREE_MERGES_MMAP_BLOCKS
    // Sort regions by base_addr
    for (int i = 0; i < MAX_FREE_REGIONS - 1; i++) {
        for (int j = i + 1; j < MAX_FREE_REGIONS; j++) {
            if (k_mmap->free_regions[j].length == 0) continue;
            if (k_mmap->free_regions[i].length == 0 || k_mmap->free_regions[j].base_addr < k_mmap->free_regions[i].base_addr) {
                free_region_t tmp = k_mmap->free_regions[i];
                k_mmap->free_regions[i] = k_mmap->free_regions[j];
                k_mmap->free_regions[j] = tmp;
            }
        }
    }

    // Merge adjacent regions
    for (int i = 0; i < MAX_FREE_REGIONS - 1; i++) {
        if (k_mmap->free_regions[i].length == 0) continue;

        for (int j = i + 1; j < MAX_FREE_REGIONS; j++) {
            if (k_mmap->free_regions[j].length == 0) continue;

            uint32_t end_i = k_mmap->free_regions[i].base_addr + k_mmap->free_regions[i].length;
            if (end_i == k_mmap->free_regions[j].base_addr) {
                k_mmap->free_regions[i].length += k_mmap->free_regions[j].length;

                for (int k = j; k < MAX_FREE_REGIONS - 1; k++)
                    k_mmap->free_regions[k] = k_mmap->free_regions[k + 1];

                k_mmap->free_regions[MAX_FREE_REGIONS - 1].base_addr = 0;
                k_mmap->free_regions[MAX_FREE_REGIONS - 1].length = 0;
                j--;
            }
        }
    }
#endif //FREE_MERGES_MMAP_BLOCKS

    // Recalculate free_region_count
    k_mmap->free_region_count = 0;
    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (k_mmap->free_regions[i].length > 0)
            k_mmap->free_region_count++;
    }
    
}


void* realloc_impl(void *ptr, size_t size) {
#if MEM_DEBUG_MODE
    Sys_log("[MEM_DBG] this func was called with %x , %d\n",ptr,size);
#endif
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
