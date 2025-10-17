#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/multiboot_info.h"

static PD_t pd;

extern char _kernel_start;
extern char _kernel_end;

int setup_paging() {
    uint32_t pages_amount = (Get_multiboot_info()->mem_upper + 1024) / 4;
    if (pages_amount < MIN_OS_PAGES * 1.5) {
        Sys_log("Not enough memory for paging setup: have %d pages, need %d",
                pages_amount, MIN_OS_PAGES + (MIN_OS_PAGES / 2));
        return -1;
    }

    if (pages_amount > 1024 * 1024) {
        Sys_log("Computer has more than 4GB of RAM, capping to 4GB.\n");
        pages_amount = 1024 * 1024;
    }

    Sys_log("Setting up paging with %d 4KB pages\n", pages_amount);

    uint32_t pde_count = (pages_amount + 1023) / 1024;

    pd.pde_arr = (PDE*)aligned_alloc(4096, 1024 * sizeof(PDE));
    pd.pte_ptrs = (PTE**)aligned_alloc(4096, 1024 * sizeof(PTE*));

    for (uint32_t i = 0; i < 1024; i++) {
        if (i < pde_count) {
            pd.pte_ptrs[i] = (PTE*)aligned_alloc(4096, 1024 * sizeof(PTE));
            for (uint32_t j = 0; j < 1024; j++) {
                PTE* pte = &pd.pte_ptrs[i][j];
                pte->present = 1;
                pte->rw = 1;
                pte->user = 0;
                pte->pwt = 0;
                pte->pcd = 0;
                pte->accessed = 0;
                pte->dirty = 0;
                pte->pat = 0;
                pte->global = 0;
                pte->os_allocated = 0;
                pte->os_unallocatable = 0;
                pte->os_unused = 0;
                pte->addr = (i * 1024 + j);
            }

            PDE* pde = &pd.pde_arr[i];
            pde->present = 1;
            pde->rw = 1;
            pde->user = 0;
            pde->pwt = 0;
            pde->pcd = 0;
            pde->accessed = 0;
            pde->reserved = 0;
            pde->page_size = 0;
            pde->ignored = 0;
            pde->available = 0;
            pde->addr = ((uint32_t)pd.pte_ptrs[i]) >> 12;
        } else {
            *((uint32_t*)&pd.pde_arr[i]) = 0;
            pd.pte_ptrs[i] = NULL;
        }
    }

    asm volatile (
        "mov %0, %%cr3 \n\t"
        "mov %%cr0, %%eax \n\t"
        "or $0x80000000, %%eax \n\t"
        "mov %%eax, %%cr0"
        :
        : "r"(pd.pde_arr)
        : "eax"
    );

    map_page(0, 0, 1, 1, 0, OS_PAGE_FLAGS_UNALLOCATABLE | OS_PAGE_FLAGS_ALLOCATED);
    reserve_kernel_pages();
    return 0;
}

void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present,
              uint8_t rw, uint8_t user, uint8_t os_flags) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    if (!pd.pte_ptrs[pd_index]) {
        pd.pte_ptrs[pd_index] = (PTE*)aligned_alloc(4096, 1024 * sizeof(PTE));
        for (uint32_t j = 0; j < 1024; j++) {
            *((uint32_t*)&pd.pte_ptrs[pd_index][j]) = 0;
        }

        PDE* pde = &pd.pde_arr[pd_index];
        pde->present = 1;
        pde->rw = rw;
        pde->user = user;
        pde->page_size = 0;
        pde->addr = ((uint32_t)pd.pte_ptrs[pd_index]) >> 12;
    }

    PTE* pte = &pd.pte_ptrs[pd_index][pt_index];
    pte->present = present;
    pte->rw = rw;
    pte->user = user;
    pte->addr = physical_addr >> 12;
    pte->os_allocated = os_flags & OS_PAGE_FLAGS_ALLOCATED;
    pte->os_unallocatable = os_flags & OS_PAGE_FLAGS_UNALLOCATABLE;
    pte->os_unused = os_flags & OS_PAGE_FLAGS_UNUSED;

    asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}

PTE* get_pte(uint32_t index) {
    uint32_t pd_index = index / 1024;
    uint32_t pt_index = index % 1024;
    if (!pd.pte_ptrs[pd_index])
        return NULL;
    return &pd.pte_ptrs[pd_index][pt_index];
}

PTE* get_pte_for_pa(uint32_t pa) {
    uint32_t index = pa / _PAGE_SIZE;
    uint32_t pd_index = index / 1024;
    uint32_t pt_index = index % 1024;

    if (pd_index >= 1024 || !pd.pte_ptrs[pd_index])
        return NULL;

    return &pd.pte_ptrs[pd_index][pt_index];
}

uint32_t page_alloc(size_t amount, int read_write, int user_supervisor) {
    Sys_log("requested %d pages\n", amount);
    uint32_t max_pages = (Get_multiboot_info()->mem_upper + 1024) / 4;
    size_t found = 0;
    uint32_t start = 0;

    for (uint32_t i = 0; i < max_pages; i++) {
        PTE* pte = get_pte(i);
        if (!pte) continue;

        if (!pte->os_allocated && !pte->os_unallocatable) {
            if (found == 0) start = i;
            found++;
            if (found == amount) {
                for (uint32_t j = 0; j < amount; j++) {
                    PTE* p = get_pte(start + j);
                    p->os_allocated = 1;
                    p->present = 1;
                    p->rw = read_write ? 1 : 0;
                    p->user = user_supervisor ? 1 : 0;
                }
                return start * _PAGE_SIZE;
            }
        } else {
            found = 0;
        }
    }

    Sys_log("page_alloc failed: not enough contiguous free pages\n");
    return 0;
}

void page_free(uint32_t pa, size_t amount) {
    uint32_t max_pages = (Get_multiboot_info()->mem_upper + 1024) / 4;
    uint32_t start_index = pa / _PAGE_SIZE;

    if (start_index >= max_pages)
        return;

    for (uint32_t i = 0; i < amount; i++) {
        uint32_t idx = start_index + i;
        if (idx >= max_pages) break;

        PTE* pte = get_pte(idx);
        if (pte)
            pte->os_allocated = 0;
    }
}

void reserve_kernel_pages() {
    Sys_log("Reserving kernel pages from %p to %p\n", &_kernel_start, &_kernel_end);
    uintptr_t start = (uintptr_t)&_kernel_start;
    uintptr_t end   = (uintptr_t)&_kernel_end;

    for (uintptr_t pa = start; pa < end; pa += _PAGE_SIZE) {
        PTE* pte = get_pte_for_pa(pa);
        if (!pte) continue;
        pte->os_unallocatable = 1;
    }
}

uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, uint32_t* PD_size) {
    if (!groups || group_count == 0) return 0;

    PD_t page_dir;

    void* pd_addr = page_alloc(1, 1, 0);
    if (!pd_addr) return 0;
    page_dir.pde_arr = (PDE*)pd_addr;

    memset(page_dir.pde_arr, 0, 1024 * sizeof(PDE));

    
    for (uint32_t g = 0; g < group_count; g++) {
        Page_Group* group = &groups[g];
        if (group->size == 0) continue;

        uintptr_t virt_addr = group->addr;
        uint32_t start_page_idx = virt_addr >> 12;
        uint32_t end_page_idx = start_page_idx + group->size - 1;

        uint32_t first_pde = start_page_idx >> 10;
        uint32_t last_pde  = end_page_idx >> 10;

        for (uint32_t pd_i = first_pde; pd_i <= last_pde; pd_i++) {
            if (!page_dir.pde_arr[pd_i].present) {
                PTE* pt_base = (PTE*)page_alloc(1, 1, 0);
                if (!pt_base) return 0;
                memset(pt_base, 0, 4096);

                page_dir.pde_arr[pd_i].present = 1;
                page_dir.pde_arr[pd_i].rw = 1;
                page_dir.pde_arr[pd_i].user = 0;
                page_dir.pde_arr[pd_i].page_size = 0;
                page_dir.pde_arr[pd_i].addr = ((uintptr_t)pt_base) >> 12;
            }
        }
    }

    
    if (!v_map(&page_dir, groups, group_count)) {
        return 0;
    }

    if (PD_size) *PD_size = 1;

    return (uintptr_t)page_dir.pde_arr;
}

int v_map(PD_t* page_dir, Page_Group* groups, uint32_t group_count) {
    if (!page_dir || !page_dir->pde_arr || !groups) return false;

    for (uint32_t g = 0; g < group_count; g++) {
        Page_Group* group = &groups[g];
        if (group->size == 0) continue;

        uintptr_t virt_addr = group->addr;
        uint32_t page_idx_start = virt_addr >> 12; 

        for (uint32_t i = 0; i < group->size; i++) {
            uint32_t virt_page_idx = page_idx_start + i;
            uint32_t pd_i = virt_page_idx >> 10;   
            uint32_t pt_i = virt_page_idx & 0x3FF; 

            PDE* pde = &page_dir->pde_arr[pd_i];
            PTE* pt_base;

            if (!pde->present) {
                pt_base = (PTE*)page_alloc(1, 1, 0);
                if (!pt_base) return false;
                memset(pt_base, 0, 4096);

                pde->present = 1;
                pde->rw = 1;
                pde->user = 0;
                pde->page_size = 0;
                pde->addr = ((uintptr_t)pt_base) >> 12;
            } else {
                pt_base = (PTE*)((uintptr_t)(pde->addr) << 12);
            }

            PTE* pte = &pt_base[pt_i];

            *pte = group->pte_bits;
            pte->addr = (group->pte_bits.addr + i); 
        }
    }

    return true;
}

uintptr_t PD_append_pages(PD_t* page_dir, PTE* ptes, uint32_t pte_count) {
    if (!page_dir || !page_dir->pde_arr || !ptes || pte_count == 0) return 0;

    uint32_t max_vpages = 1024 * 1024; 
    uint32_t free_run_start = 0;
    uint32_t run_length = 0;

    
    for (uint32_t i = 0; i < max_vpages; i++) {
        uint32_t pd_i = i >> 10;
        uint32_t pt_i = i & 0x3FF;

        PDE* pde = &page_dir->pde_arr[pd_i];

        if (!pde->present) {
            run_length++;
        } else {
            PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
            if (!pt_base[pt_i].present) {
                run_length++;
            } else {
                run_length = 0;
                free_run_start = i + 1;
                continue;
            }
        }

        if (run_length == pte_count) {
            break;
        }
    }

    if (run_length < pte_count) {
        Sys_log("failed for %u pages\n", pte_count);
        return 0;
    }

    uint32_t base_page_idx = free_run_start;
    for (uint32_t i = 0; i < pte_count; i++) {
        uint32_t virt_page_idx = base_page_idx + i;
        uint32_t pd_i = virt_page_idx >> 10;
        uint32_t pt_i = virt_page_idx & 0x3FF;

        PDE* pde = &page_dir->pde_arr[pd_i];
        PTE* pt_base;

        if (!pde->present) {
            pt_base = (PTE*)page_alloc(1, 1, 0);
            if (!pt_base) return 0;
            memset(pt_base, 0, 4096);

            pde->present = 1;
            pde->rw = 1;
            pde->user = 0;
            pde->page_size = 0;
            pde->addr = ((uintptr_t)pt_base) >> 12;
        } else {
            pt_base = (PTE*)((uintptr_t)pde->addr << 12);
        }

        pt_base[pt_i] = ptes[i];
    }

    return base_page_idx << 12;
}
