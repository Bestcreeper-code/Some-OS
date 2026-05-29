#include "memory.h"
#include "bootloader.h"

#include "drivers.h"
#include "err_codes.h"
#include "string.h"

#include <stddef.h>
#include <stdint.h>
#include <io.h>
#include "config.h"
#include "Logger.h"
#include "paging.h"

#define  KERNEL_RESERVED_PAGES 4

volatile size_t ram_amount;

static free_region_map_t region_map;
free_region_map_t* k_mmap = &region_map;

extern char _kernel_end;


static inline free_region_map_t* get_free_region_map(void) {
    return k_mmap;
}

void force_alloc(uintptr_t address, size_t size) {
    free_region_map_t* k_mmap = get_free_region_map();
    uintptr_t end = address + size;
    size_t page_size = 0x1000;

    for (uintptr_t page_addr = address & ~(page_size - 1); page_addr < end; page_addr += page_size) {
        
        uintptr_t page_start = page_addr;
        uintptr_t page_end = page_addr + page_size;

        if (address > page_start) {
            uintptr_t free_start = page_start;
            size_t free_len = address - page_start;
            force_free(free_start, free_len);
        }

        if (end < page_end) {
            uintptr_t free_start = end;
            size_t free_len = page_end - end;
            force_free(free_start, free_len);
        }
    }

    uintptr_t lock_end = end;
    for (int i = 0; i < k_mmap->free_region_count; i++) {
        free_region_t* region = &k_mmap->free_regions[i];
        uintptr_t region_end = region->base_addr + region->length;

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
            size_t first_len = address - region->base_addr;
            uintptr_t second_base = lock_end;
            size_t second_len = region_end - lock_end;
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

void force_free(uintptr_t address, size_t size) {
    free_region_map_t* k_mmap = get_free_region_map();

    uintptr_t new_start = address & ~0xFFFU; // align down to 4KB page
    uintptr_t new_end   = (address + size + 0xFFFU) & ~0xFFFU; // align up

    // never free memory below 0x10000
    if (new_end <= 0x10000) return;
    if (new_start < 0x10000) new_start = 0x10000;

    for (int i = 0; i < k_mmap->free_region_count; i++) {
        free_region_t* region = &k_mmap->free_regions[i];
        uintptr_t region_start = region->base_addr;
        uintptr_t region_end   = region->base_addr + region->length;

        if (new_end >= region_start && new_start <= region_end) {
            uintptr_t merged_start = (new_start < region_start) ? new_start : region_start;
            uintptr_t merged_end   = (new_end > region_end) ? new_end : region_end;
            region->base_addr = merged_start;
            region->length = merged_end - merged_start;

            for (int j = 0; j < k_mmap->free_region_count; j++) {
                if (j == i) continue;
                free_region_t* other = &k_mmap->free_regions[j];
                uintptr_t other_start = other->base_addr;
                uintptr_t other_end   = other->base_addr + other->length;

                if (region->base_addr <= other_end && region->base_addr + region->length >= other_start) {
                    uintptr_t merge_start = (region->base_addr < other_start) ? region->base_addr : other_start;
                    uintptr_t merge_end   = ((region->base_addr + region->length) > other_end) ? (region->base_addr + region->length) : other_end;
                    region->base_addr = merge_start;
                    region->length = merge_end - merge_start;

                    for (int k = j; k < k_mmap->free_region_count - 1; k++) {
                        k_mmap->free_regions[k] = k_mmap->free_regions[k + 1];
                    }
                    k_mmap->free_regions[k_mmap->free_region_count - 1].base_addr = 0;
                    k_mmap->free_regions[k_mmap->free_region_count - 1].length = 0;
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

REGISTER_DRIVER_CORE(k_allocator, parse_memory_map);
int parse_memory_map() {
        
    ram_amount = get_bootloader_mem_info()->mem_lower+get_bootloader_mem_info()->mem_upper;
        
    for (int i = 0; i < KERNEL_RESERVED_PAGES; i++) {
        uintptr_t page_addr = PAGE_ADDR(page_alloc(1,PAGE_FLAG_RW));

        k_mmap->free_regions[k_mmap->free_region_count].base_addr = page_addr;
        k_mmap->free_regions[k_mmap->free_region_count].length = PAGE_SIZE;
        k_mmap->free_region_count++;

        
    }
    
    Sys_log("Kernel \"heap\" memory regions (%d):\n", k_mmap->free_region_count);
    for (int i = 0; i < k_mmap->free_region_count; i++) {
        Sys_log("  Region %d: Base = 0x%x, Length = 0x%x\n",
                i,
                k_mmap->free_regions[i].base_addr,
                k_mmap->free_regions[i].length);
    }

    for (int i = 0; i < k_mmap->free_region_count; i++) {
        free_region_t* region = &k_mmap->free_regions[i];

        if (region->length == 0) continue;

        if (region->base_addr < (uintptr_t)&_kernel_end) {
            size_t overlap = (uintptr_t)&_kernel_end - region->base_addr;
            if (overlap >= region->length) {
                region->length = 0; // entire region is under kernel, remove it
            } else {
                region->base_addr += overlap;
                region->length -= overlap;
            }
        }
    }


    Sys_log("Memory map parsed.\n");
    return 0;
}



free_region_t* FirstRegionOfSizeOrMore(size_t size) {
    free_region_map_t* k_mmap = get_free_region_map();
    size_t requestedSize = (size + sizeof(uintptr_t) + 7) & ~7U;

    for (int i = 0; i < k_mmap->free_region_count; i++) {
        if (k_mmap->free_regions[i].length == 0) continue;

        uintptr_t currBase = k_mmap->free_regions[i].base_addr;
        size_t currSize = k_mmap->free_regions[i].length;

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

uintptr_t get_pter_size(void* pter){
    uint8_t* raw = (uint8_t*)pter;
    uintptr_t* sizeaddr = (uintptr_t*)(raw - sizeof(uintptr_t));
    return *sizeaddr;
}

void* kmalloc_impl(size_t size) {
#if MEM_DEBUG
    Sys_log("[MEM_DBG] this func was called with %d\n",size);
#endif
    if (size == 0) return NULL;

    free_region_map_t* k_mmap = get_free_region_map();
    if (!k_mmap) return NULL;

    if (size > SIZE_MAX - sizeof(uintptr_t)) return NULL; // prevent overflow
    size_t full_size = (size + sizeof(uintptr_t) + 7) & ~7U;

    free_region_t* region = FirstRegionOfSizeOrMore(full_size);

    if (!region || region->base_addr <= 0xFFFF) {
        size_t pages_needed = (full_size + PAGE_SIZE - 1) / PAGE_SIZE;
        page_index base = page_alloc(pages_needed, PAGE_FLAG_RW);
        if (!base) {
            return NULL;
        }
        Sys_Error("malloc failed(not enough pages): %u bytes\n", (unsigned)size);

        // force free only above 0x10000
        uintptr_t page_addr = PAGE_ADDR(base);
        if (page_addr < 0x10000) page_addr = 0x10000;
        force_free(page_addr, pages_needed * PAGE_SIZE);

        region = FirstRegionOfSizeOrMore(full_size);
    }

    if(region->base_addr <= 0xFFFF){
        Sys_Error("BRUH : %u",region->base_addr);
        Sys_Step_Point();
    }

    uintptr_t* header = (uintptr_t*)region->base_addr;
    *header = (uintptr_t)size;

    region->base_addr += full_size;
    region->length -= full_size;
    if (region->length == 0) {
        int idx = (int)(region - k_mmap->free_regions);
        for (int i = idx; i < k_mmap->free_region_count - 1; i++)
            k_mmap->free_regions[i] = k_mmap->free_regions[i + 1];
        k_mmap->free_region_count--;
    }

#if MEM_DEBUG
    Sys_log("malloc'ed %u bytes at %p\n", (unsigned)full_size, header + 1);
#endif

    return (void*)(header + 1);
}



void kfree_impl(void* _Memory) {
#if MEM_DEBUG
    Sys_log("[MEM_DBG] this func was called with %d\n",_Memory);
#endif
    

    free_region_map_t* k_mmap = get_free_region_map();

    uintptr_t address = (uintptr_t)_Memory;
    uintptr_t* sizeptr = (uintptr_t*)(address - sizeof(uintptr_t));
    size_t size = *sizeptr + sizeof(uintptr_t);
    size = (size + 7) & ~7U;

    for (int i = 0; i < MAX_FREE_REGIONS; i++) {
        if (k_mmap->free_regions[i].length == 0) {
            k_mmap->free_regions[i].base_addr = address - sizeof(uintptr_t);
            k_mmap->free_regions[i].length = size;
#if MEM_DEBUG
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

            uintptr_t end_i = k_mmap->free_regions[i].base_addr + k_mmap->free_regions[i].length;
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


void* krealloc_impl(void *ptr, size_t size) {
#if MEM_DEBUG
    Sys_log("[MEM_DBG] this func was called with %x , %d\n",ptr,size);
#endif
    if (size == 0) {
        kfree(ptr);
        return NULL;
    }

    if (!ptr) {
        return kmalloc(size);
    }

    void *new_ptr = kmalloc(size);
    if (!new_ptr) {
        return NULL;
    }

    size_t old_len = get_pter_size(ptr);
    size_t copy_size = (size < old_len) ? size : old_len;

    memcpy(new_ptr, ptr, copy_size);
    kfree(ptr);

    return new_ptr;
}

void* aligned_malloc(size_t size, size_t alignment) {
    if ((alignment & (alignment - 1)) != 0) {
        Sys_log("Error: alignment must be power of two\n");
        return NULL;
    }

    size_t extra = alignment - 1 + sizeof(uintptr_t);
    void* raw = kmalloc_impl(size + extra);
    if (!raw) return NULL;

    uintptr_t raw_addr = (uintptr_t)raw;
    uintptr_t aligned_addr = (raw_addr + sizeof(uintptr_t) + alignment - 1) & ~(alignment - 1);

    ((uintptr_t*)aligned_addr)[-1] = raw_addr;

    return (void*)aligned_addr;
}

void aligned_free(void* ptr) {
    

    uintptr_t aligned_addr = (uintptr_t)ptr;
    void* raw = (void*)((uintptr_t*)aligned_addr)[-1];

    kfree_impl(raw);
}