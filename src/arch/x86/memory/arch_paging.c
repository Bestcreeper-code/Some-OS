#include "paging.h"
#include "arch_paging.h"
// #include "memory.h"
#include "multiboot_info.h"
#include "string.h"
// #include "io.h"
#include "time.h"
#include "io.h"
#include "Logger.h"
#include "memory.h"
#include <stdint.h>
#include <stdnoreturn.h>

PD_t _k_pd;

extern char _kernel_start;
extern char _kernel_end;

uint32_t _free_pages_bitmap[(1024 * 1024 / sizeof(uint32_t)) ];

uint32_t _pages_amount = 0;
uint32_t _pages_tables_amount = 0;

PTE* kernel_page_table_ptr;


int setup_paging() {
    Sys_log("Setting up paging...\n");

    _pages_amount = ((Get_multiboot_info()->mem_upper * 1024) + 1024*1024) / PAGE_SIZE;
    if (_pages_amount < MIN_OS_PAGES * 1.5) return -1;
    if (_pages_amount > 1024 * 1024) _pages_amount = 1024 * 1024;

    
    
    _pages_tables_amount = _pages_amount / PAGE_SIZE;

    dw_memset(_free_pages_bitmap, 0xFFFFFFFF, sizeof(_free_pages_bitmap)/sizeof(uint32_t));
    reserve_kernel_pages();
    Sys_log("reserving Page Tables\n");

    page_index kend =  (page_index)(&_kernel_end) >> 12;
    page_index kstart = (page_index)&_kernel_start >> 12;

    _k_pd.pde_arr = (PDE*)(page_alloc_nomap(1)<<12);
    
    dw_memset(_k_pd.pde_arr, 0, PAGE_SIZE/4);

    PTE* k_pte_array = (PTE*)(page_alloc_nomap(_MAX_PT_AMOUNT) << 12); 
    dw_memset(k_pte_array, 0, _MAX_PT_AMOUNT * PAGE_SIZE);
    kernel_page_table_ptr = k_pte_array;  

    for (uint32_t i = 0; i < _MAX_PT_AMOUNT ; i++) {
        if (i * 1024 >= _pages_amount) break;

        PTE* pt_base = (PTE*)&kernel_page_table_ptr[i * PAGE_SIZE / 4];
        
        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t page_idx = i * 1024 + j;
            
            if (page_idx >= _pages_amount) break;

            bool used = !(_free_pages_bitmap[page_idx / 32] & (1u << (page_idx % 32)));

            pt_base[j].present = used;
            pt_base[j].rw = 1;
            pt_base[j].user = 0;
            pt_base[j].addr = page_idx;
            if (page_idx == 0) pt_base[j].rw = 0;
        }
#if PAGE_DEBUG_MODE
        Sys_log("setup pTable %d\n",i);
#endif
        _k_pd.pde_arr[i].present = 1;
        _k_pd.pde_arr[i].rw = 1;
        _k_pd.pde_arr[i].user = 0;
        _k_pd.pde_arr[i].page_size = 0;
        _k_pd.pde_arr[i].addr = ((uintptr_t)pt_base) >> 12;
    }
    
    
    
    Sys_log("starting Paging %x\n",_k_pd.pde_arr);
    asm volatile (
        "mov %0, %%cr3 \n\t"
        "mov %%cr0, %%eax \n\t"
        "or $0x80000000, %%eax \n\t"
        "mov %%eax, %%cr0"
        :
        : "r"(_k_pd.pde_arr)
        : "eax"
    );
    
    unmap_page(0);
    
    Sys_log("Paging set up successfully.\n");
    return 0;
}


void reserve_kernel_pages() {
    uintptr_t kend   = (uintptr_t)&_kernel_end;

    Sys_log("Reserving kernel pages from 0 to %p\n", &_kernel_end);

    uint32_t end_page = (kend + PAGE_SIZE - 1) >> 12;
    uint32_t reserved = 0;

    for (uint32_t i = 0; i < end_page && i < _pages_amount; i++) {
        _free_pages_bitmap[i / 32] &= ~(1u << (i % 32));
        reserved++;
    }

    Sys_log("Reserved %u kernel pages\n", reserved);
}


void tlb_flush_page(uintptr_t virt) {
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

void tlb_flush_all(void) {
    asm volatile(
        "mov %%cr3, %%eax\n\t"
        "mov %%eax, %%cr3"
        : : : "eax", "memory"
    );
}


void pd_map_page(PD_t* pd, uint32_t virtual_addr, uint32_t physical_addr, uint8_t present,
              uint8_t rw, uint8_t user) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    PDE* pde = &pd->pde_arr[pd_index];

    PTE* pt_base;
    if (!pde->present) {
        pt_base = (PTE*)((uint32_t)page_alloc(1, PAGE_FLAG_RW) << 12);
        if (!pt_base) return;

        memset(pt_base, 0, PAGE_SIZE);

        pde->present = 1;
        pde->rw = rw;
        pde->user = user;
        pde->page_size = 0;
        pde->addr = ((uintptr_t)pt_base) >> 12;
    } else {
        pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    }

    PTE* pte = &pt_base[pt_index];

    sleep(0); // fixes all QEMU jank smh

    pte->present = present;
    pte->rw = rw;
    pte->user = user;
    pte->addr = physical_addr >> 12;

    tlb_flush_page(virtual_addr);
}


int map_page(page_index virt, page_index phys, uint8_t present, uint8_t rw, uint8_t user) {
    pd_map_page(&_k_pd, (uint32_t)(virt << 12), (uint32_t)(phys << 12), present, rw, user);
    return 0;
}


int map_range(page_index virt, page_index phys, size_t count, char flags) {
    uint8_t rw   = (flags & PAGE_FLAG_RW)   ? 1 : 0;
    uint8_t user = (flags & PAGE_FLAG_USER) ? 1 : 0;
    for (size_t i = 0; i < count; i++) {
        pd_map_page(&_k_pd, (uint32_t)((virt + i) << 12), (uint32_t)((phys + i) << 12), 1, rw, user);
    }
    return 0;
}


int pd_unmap_page(PD_t* target_pd, uint32_t virtual_addr) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    PDE* pde = &target_pd->pde_arr[pd_index];
    if (!pde->present) return -1;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    PTE* pte = &pt_base[pt_index];
    if (!pte->present) return -2;

    pte->present = 0;

    tlb_flush_page(virtual_addr);

    return 0;
}


int unmap_page(page_index virt) {
    return pd_unmap_page(&_k_pd, (uint32_t)(virt << 12));
}




PTE* get_pte(uint32_t index) {
    uint32_t pd_index = index >> 10; 
    uint32_t pt_index = index & 0x3FF;

    PDE* pde = &_k_pd.pde_arr[pd_index];
    if (!pde) return NULL;
    if (!pde->present) return NULL;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    return &pt_base[pt_index];
}

PTE* get_pte_for_pa(uint32_t pa) {
    uint32_t index = pa >> 12;
    return get_pte(index);
}

PAGE virt_to_page(void *address) {
    uint32_t va    = (uint32_t)address;
    uint32_t pd_i  = (va >> 22) & 0x3FF;
    uint32_t pt_i  = (va >> 12) & 0x3FF;

    PAGE result = {0};

    PDE* pde = &_k_pd.pde_arr[pd_i];
    if (!pde->present) return result;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    PTE* pte     = &pt_base[pt_i];

    result.present = pte->present;
    result.addr    = (uintptr_t)pte->addr << 12;
    result.rw      = pte->rw;
    result.us      = pte->user;
    return result;
}

bool page_is_present(page_index virt) {
    return virt_to_page((void*)(virt << 12)).present;
}




page_index page_alloc_nomap(size_t amount) {
    size_t found = 0;
    page_index start = 0;

    for (uint32_t i = 0; i < _pages_amount; i++) {
        if (_free_pages_bitmap[i / 32] & (1 << (i % 32))) {
            if (found == 0) start = i;
            found++;
            if (found == amount) {
                for (uint32_t j = 0; j < amount; j++) {
                    uint32_t idx = start + j;
                    _free_pages_bitmap[idx / 32] &= ~(1 << (idx % 32));
                }
#if PAGE_DEBUG_MODE
                Sys_log("page_alloc_nomap called for %u pages (%x)\n", (unsigned)amount, start * PAGE_SIZE);
#endif
                return start;
            }
        } else {
            found = 0;
        }
    }
#if PAGE_DEBUG_MODE
    Sys_Error("page_alloc_nomap for %d page(s) failed: not enough contiguous free pages\n", amount);
#endif
    return 0;
}


page_index page_alloc(size_t amount, char flags) {
    uint8_t rw   = (flags & PAGE_FLAG_RW)   ? 1 : 0;
    uint8_t user = (flags & PAGE_FLAG_USER) ? 1 : 0;

    page_index start = page_alloc_nomap(amount);
    if (start == 0) return 0;

    for (uint32_t j = 0; j < amount; j++) {
        uint32_t idx = start + j;
        PTE* p = get_pte(idx);
        if (p) {
            p->present = 1;
            p->rw      = rw;
            p->user    = user;
            p->addr    = idx;
        }
    }
#if PAGE_DEBUG_MODE
    Sys_log("page_alloc called for %u pages (%x) with mapping\n", (unsigned)amount, start * PAGE_SIZE);
#endif    
    return start;
}

void page_free(page_index pa, size_t amount) {
    uint32_t start_index = pa >> 12;

    for (uint32_t i = 0; i < amount; i++) {
        uint32_t idx = start_index + i;
        if (idx >= _pages_amount) break;

        _free_pages_bitmap[idx / 32] |= (1 << (idx % 32));

        PTE* pte = get_pte(idx);
        if (pte) pte->present = 0;
    }
}


void dump_pd() {
    uint32_t* pdes = (uint32_t*)_k_pd.pde_arr;
#if PAGE_DEBUG_MODE
    Sys_log("Dumping Page Directory from %x\n", _k_pd.pde_arr);
    Sys_log("first pde: %x\n", ((PTE*)pdes)[0].addr * PAGE_SIZE);
#endif
    for (int i = 0; i < 1; i++) {
        if (!(pdes[i] & 1)) continue;
        uint32_t base = pdes[i] & 0xFFFFF000;
        uint32_t* pt = (uint32_t*)base;
        for (int j = 0; j < 300; j++) {
            if (1||pt[j] & 1) {
                uint32_t pa = pt[j] & 0xFFFFF000;
#if PAGE_DEBUG_MODE
                Sys_log("PD[%03d] PT[%03d] VA=%x -> data=%x\n", i, j, (i<<22)|(j<<12), pt[j]);
#endif
            }
        }
    }
    while (1);
}

void debug_dump_kernel_mapping(PD_t* pd, uint32_t va) {
    uint32_t pd_i = (va >> 22) & 0x3FF;
    uint32_t pt_i = (va >> 12) & 0x3FF;

    PDE* pde = &pd->pde_arr[pd_i];
    Sys_Warning("K-VA 0x%x: PDE[%u] present=%u rw=%u user=%u addr=0x%x\n",
            va, pd_i, pde->present, pde->rw, pde->user, pde->addr << 12);

    if (!pde->present) return;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    PTE* pte = &pt_base[pt_i];

    Sys_Warning("           PTE[%u] present=%u rw=%u user=%u addr=0x%x\n",
            pt_i, pte->present, pte->rw, pte->user, pte->addr << 12);
}

uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, PD_t* out_pd_t) {
    if (!groups || group_count == 0) return 0;

    PDE* pd_addr = (PDE*)PAGE_ADDR(page_alloc(1, PAGE_FLAG_RW));
    if (!pd_addr) return 0;
    memset(pd_addr, 0, PAGE_SIZE);
    out_pd_t->pde_arr = pd_addr;

    for (uint32_t i = 0; i < 1024; i++) {
        out_pd_t->pde_arr[i] = _k_pd.pde_arr[i];
    }

    for (uint32_t g = 0; g < group_count; g++) {
        Sys_log("%x  %x   %x\n\n\n\n", groups[g].vaddr, groups[g].pte_bits, groups[g].size);
        Page_Group* group = &groups[g];
        if (group->size == 0) continue;

        for (uint32_t i = 0; i < group->size; i++) {
            uint32_t virt_page = group->vaddr + i;
            uint32_t phys_page = group->pte_bits.addr + i;

            uint32_t pd_i = virt_page >> 10;
            uint32_t pt_i = virt_page & 0x3FF;

            PDE* pde = &out_pd_t->pde_arr[pd_i];

            if (!pde->present) {
                char flags = PAGE_FLAG_RW | (group->pte_bits.user ? PAGE_FLAG_USER : 0);
                PTE* new_pt = (PTE*)PAGE_ADDR(page_alloc(1, flags));
                if (!new_pt) return 0;
                memset(new_pt, 0, PAGE_SIZE);

                pde->present   = 1;
                pde->rw        = group->pte_bits.rw;
                pde->user      = group->pte_bits.user;
                pde->page_size = 0;
                pde->addr      = (uintptr_t)new_pt >> 12;
            }

            PTE* pt    = (PTE*)((uintptr_t)pde->addr << 12);
            PTE  entry = group->pte_bits;
            entry.addr = phys_page;
            entry.present = 1;
            pt[pt_i]   = entry;
        }
    }

    Sys_Warning("new_page_dir: cr3 at %x\n", out_pd_t->pde_arr);
    return (uintptr_t)out_pd_t->pde_arr;
}

void pd_free(PD_t* pd) {
    if (!pd || !pd->pde_arr) return;

    for (uint32_t pd_i = 0; pd_i < 1024; pd_i++) {
        PDE* pde = &pd->pde_arr[pd_i];
        if (!pde->present || !pde->user) continue;

        PTE* pt_base = (PTE*)((uintptr_t)(pde->addr) << 12);
        if (!pt_base) continue;

        for (uint32_t pt_i = 0; pt_i < 1024; pt_i++) {
            PTE* pte = &pt_base[pt_i];
            if (!pte->present || !pte->user) continue;

            page_free((uintptr_t)(pte->addr), 1);
        }

        page_free((uintptr_t)pt_base, 1);
    }

    page_free((uintptr_t)pd->pde_arr, 1);
    pd->pde_arr = NULL;
}


int v_map(PD_t* page_dir, Page_Group* groups, uint32_t group_count) {
    if (!page_dir || !page_dir->pde_arr || !groups) return false;

    for (uint32_t g = 0; g < group_count; g++) {
        Page_Group* group = &groups[g];
        if (group->size == 0) continue;

        uintptr_t virt_addr = group->vaddr;
        uint32_t page_idx_start = virt_addr >> 12;

        for (uint32_t i = 0; i < group->size; i++) {
            uint32_t virt_page_idx = page_idx_start + i;
            uint32_t pd_i = virt_page_idx >> 10;   
            uint32_t pt_i = virt_page_idx & 0x3FF; 

            PDE* pde = &page_dir->pde_arr[pd_i];
            PTE* pt_base;

            if (!pde->present) {
                pt_base = (PTE*)page_alloc(1, PAGE_FLAG_RW);
                if (!pt_base) return false;
                memset(pt_base, 0, PAGE_SIZE);

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
#if PAGE_DEBUG_MODE
        Sys_log("failed for %u pages\n", pte_count);
#endif
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
            pt_base = (PTE*)page_alloc(1, PAGE_FLAG_RW);
            if (!pt_base) return 0;
            memset(pt_base, 0, PAGE_SIZE);

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

int is_page_allocated(page_index pa) {
    uint32_t idx = (uint32_t)(pa >> 12);
    if (idx >= _pages_amount) return -1;
    return !((_free_pages_bitmap[idx / 32] >> (idx % 32)) & 1);
}

void page_force_alloc(page_index pa, size_t amount) {
    uint32_t start = pa >> 12;
    for (size_t i = 0; i < amount; i++) {
        uint32_t idx = start + i;
        if (idx >= _pages_amount) break;

        _free_pages_bitmap[idx / 32] &= ~(1 << (idx % 32));
        PTE* p = get_pte(idx);
        if (p) {
            p->present = 1;
            p->rw = 1;
            p->user = 0;
            p->addr = idx;
        }
    }
}

page_index k_append_pages(page_index phys_start_page, uint32_t amount, uint8_t rw, uint8_t us) {
    uint32_t start_page_idx = 0;
    uint32_t found = 0;

    for (page_index i = 1; i < KERNEL_PDE_COUNT * 1024; i++) {
        PTE* curr_pte = &kernel_page_table_ptr[(i / 1024) * 1024 + (i % 1024)];
        
        if (!curr_pte->present) {
            if (found == 0) start_page_idx = i;
            found++;
            if (found == amount) {
                goto found;
            }
        } else {
            found = 0;
        }
    }
    
    found:
    if (found < amount) {
#if PAGE_DEBUG_MODE
        Sys_log("k_append_pages failed: not enough contiguous free pages\n");
#endif
        return 0;
    }
    for (uint32_t j = 0; j < amount; j++) {
        uint32_t idx = start_page_idx + j;
        PTE* pte = &kernel_page_table_ptr[idx];

        pte->present = 1;
        pte->rw = rw ? 1 : 0;
        pte->user = us ? 1 : 0;
        pte->addr = phys_start_page + j;
#if PAGE_DEBUG_MODE
        tlb_flush_page(idx);
#endif
    }
#if PAGE_DEBUG_MODE
    Sys_log("k_append_pages mapped %u pages at VA %x\n", amount, start_page_idx * PAGE_SIZE);
#endif
    return start_page_idx;
}

int page_write(page_index virt, PAGE page) {
    uint32_t va   = (uint32_t)(virt << 12);
    uint32_t pd_i = (va >> 22) & 0x3FF;
    uint32_t pt_i = (va >> 12) & 0x3FF;

    PDE* pde = &_k_pd.pde_arr[pd_i];
    if (!pde->present) return -1;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    PTE* pte     = &pt_base[pt_i];

    pte->present = page.present;
    pte->rw      = page.rw;
    pte->user    = page.us;
    pte->addr    = page.addr >> 12;

    tlb_flush_page(va);
    return 0;
}

size_t get_used_ram() {
    uint32_t used_pages = 0;

    for (uint32_t i = 0; i < (_pages_amount + 31) / 32; i++) {
        uint32_t word = _free_pages_bitmap[i];
        uint32_t used_in_word = 0;
        for (int b = 0; b < 32; b++) {
            if (!(word & (1u << b))) used_in_word++;
        }
        used_pages += used_in_word;
    }

    return used_pages * PAGE_SIZE; 
}

page_index vmap(page_index phys, size_t count, char flags) {
    if (count == 0) return 0;

    page_index run_start = 0;
    size_t     run_len   = 0;

    for (page_index i = KVSPACE_FIRST_PAGE; i <= KVSPACE_LAST_PAGE; i++) {
        if (!page_is_present(i)) {
            if (run_len == 0) run_start = i;
            run_len++;
            if (run_len == count) goto found;
        } else {
            run_len = 0;
        }
    }
    return 0; 

found:
    if (map_range(run_start, phys, count, flags) != 0) return 0;
    return run_start;
}