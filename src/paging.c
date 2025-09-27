#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/multiboot_info.h"

static PD_t pd __attribute__((aligned(4096)));


int setup_paging() {
    uint32_t pages_amount = (Get_multiboot_info()->mem_upper + 1024) / 4; // pages in KB / 4 = 4KB pages
    if (pages_amount < MIN_OS_PAGES * 1.5) {
        Sys_log("Not enough memory for paging setup: have %d pages, need %d", pages_amount, MIN_OS_PAGES + (MIN_OS_PAGES / 2));
        return -1;
    }

    if (pages_amount > 1024 * 1024) {
        Sys_log("Computer has more than 4GB of RAM, capping to 4GB.\n");
        pages_amount = 1024 * 1024;
    }

    Sys_log("Setting up paging with %d 4KB pages", pages_amount);

    uint32_t pde_count = (pages_amount + 1023) / 1024;

    for (uint32_t i = 0; i < 1024; i++) {
        if (i < pde_count) {
            pd.pde_arr[i].present = 1;
            pd.pde_arr[i].rw = 1;
            pd.pde_arr[i].user = 0;
            pd.pde_arr[i].pwt = 0;
            pd.pde_arr[i].pcd = 0;
            pd.pde_arr[i].accessed = 0;
            pd.pde_arr[i].reserved = 0;
            pd.pde_arr[i].page_size = 0; // 4KB pages
            pd.pde_arr[i].ignored = 0;
            pd.pde_arr[i].available = 0;
            pd.pde_arr[i].addr = ((uint32_t)&pd.pte_arr[i * 1024]) >> 12;
        } else {
            *((uint32_t*)&pd.pde_arr[i]) = 0; // Clear unused PDEs
        }
    }

    for (uint32_t i = 0; i < pages_amount; i++) {
        PTE* pte = &pd.pte_arr[i];
        pte->present = 1;
        pte->rw = 1;
        pte->user = (i >= MIN_OS_PAGES) ? 1 : 0;
        pte->pwt = 0;
        pte->pcd = 0;
        pte->accessed = 0;
        pte->dirty = 0;
        pte->pat = 0;
        pte->global = 0;
        pte->os_allocated = 0;
        pte->os_unallocatable = 0;
        pte->os_unused = 0;
        pte->addr = i; // Identity map
    }

    asm volatile (
        "mov %0, %%cr3 \n\t"
        "mov %%cr0, %%eax \n\t"
        "or $0x80000000, %%eax \n\t"
        "mov %%eax, %%cr0"
        :
        : "r"(&pd)
        : "eax"
    );

    return 0;
}

void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present, uint8_t rw, uint8_t user) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    PTE* pt = &pd.pte_arr[pd_index * 1024];

    if (!pd.pde_arr[pd_index].present) {
        pd.pde_arr[pd_index].present = 1;
        pd.pde_arr[pd_index].rw = rw;
        pd.pde_arr[pd_index].user = user;
        pd.pde_arr[pd_index].page_size = 0;
        pd.pde_arr[pd_index].addr = ((uint32_t)pt) >> 12;
    }

    PTE* pte = &pt[pt_index];
    pte->present = present;
    pte->rw = rw;
    pte->user = user;
    pte->addr = physical_addr >> 12;

    asm volatile ("invlpg (%0)" : : "r" (virtual_addr) : "memory");
}

PTE* get_pte(uint32_t index){
    return &pd.pte_arr[index];
}

PTE* get_pte_for_pa(uint32_t pa) {
    uint32_t index = pa / 0x1000;
    uint32_t max_pages = (Get_multiboot_info()->mem_upper + 1024) / 4;

    if (index >= max_pages)
        return NULL;

    return get_pte(index);
}

uint32_t page_alloc(size_t amount) {
    uint32_t max_pages = (Get_multiboot_info()->mem_upper + 1024) / 4;
    size_t found = 0;
    uint32_t start = 0;

    for (uint32_t i = 0; i < max_pages; i++) {
        if (!pd.pte_arr[i].os_allocated && !pd.pte_arr[i].os_unallocatable) {
            if (found == 0) start = i;
            found++;
            if (found == amount) {
                for (uint32_t j = 0; j < amount; j++)
                    pd.pte_arr[start + j].os_allocated = 1;
#if DEBUG_MODE
                Sys_log("allocated page %d",start);
#endif
                return start * 0x1000; //  physical address
            }
        } else {
            found = 0;
        }
    }

    return 0; // Not enough free pages
}

void page_free(uint32_t pa, size_t amount) {
    uint32_t max_pages = (Get_multiboot_info()->mem_upper + 1024) / 4;
    uint32_t start_index = pa / 0x1000;

    if (start_index >= max_pages)
        return; // Out-of-bounds

    for (uint32_t i = 0; i < amount; i++) {
        uint32_t idx = start_index + i;
        if (idx >= max_pages)
            break;
        pd.pte_arr[idx].os_allocated = 0;
    }
}
