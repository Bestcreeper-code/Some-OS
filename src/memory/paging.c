#include "headers/paging.h"
#include "headers/multiboot_info.h"
#include "headers/string.h"
#include "headers/time.h"
#include "headers/Logger.h"
#include <stdint.h>
#include <stdnoreturn.h>

PD_t _k_pd;

extern char _kernel_start;
extern char _kernel_end;

uint32_t _free_pages_bitmap[(1024 * 1024 / 32)];

PTE _pte_array[(1024 * 1024)];
PDE _pde_array[1024];

uint32_t _pages_amount = 0;
uint32_t _pages_tables_amount = 0;

PTE* kernel_page_table_ptr;


int setup_paging() {
    Sys_log("Setting up paging...\n");

    _pages_amount = ((Get_multiboot_info()->mem_upper * 1024) + 1024*1024) / _PAGE_SIZE;
    if (_pages_amount < MIN_OS_PAGES * 1.5) return -1;
    if (_pages_amount > 1024 * 1024) _pages_amount = 1024 * 1024;
    
    _pages_tables_amount = _pages_amount / _PAGE_SIZE;

    Sys_log("Clearing bitmap...\n");
    dw_memset(_free_pages_bitmap, 0xFFFFFFFF, sizeof(_free_pages_bitmap)/sizeof(uint32_t));
    
    Sys_log("Reserving kernel pages...\n");
    reserve_kernel_pages();
    
    page_index kend = (page_index)(&_kernel_end) >> 12;
    page_index kstart = (page_index)(&_kernel_start) >> 12;

    Sys_log("kernel: %x -> %x (%u pages)\n", kstart, kend, kend - kstart);
    Sys_log("total pages: %u\n", _pages_amount);

    Sys_log("Allocating page directory...\n");
    page_index pd_idx = page_alloc(1, 1, 0);
    if (!pd_idx) {
        Sys_Error("Failed to allocate page directory\n");
        return -1;
    }
    Sys_log("PD at page %x (addr=%x)\n", pd_idx, pd_idx << 12);
    
    _k_pd.pde_arr = (PDE*)(pd_idx << 12);
    
    Sys_log("Clearing PD...\n");
    dw_memset(_k_pd.pde_arr, 0, _PAGE_SIZE);

    Sys_log("Allocating page tables (%u pages)...\n", _MAX_PT_AMOUNT);
    page_index pte_idx = page_alloc(_MAX_PT_AMOUNT, 1, 0);
    if (!pte_idx) {
        Sys_Error("Failed to allocate page tables\n");
        return -1;
    }
    Sys_log("PT at page %x (addr=%x, size: %u pages)\n", pte_idx, pte_idx << 12, _MAX_PT_AMOUNT);
    
    kernel_page_table_ptr = (PTE*)(pte_idx << 12);

    Sys_log("Building page tables...\n");
    for (uint32_t i = 0; i < _MAX_PT_AMOUNT ; i++) {
        if (i * 1024 >= _pages_amount) {
            Sys_log("Stopping PT loop at i=%u (pages_amount=%u)\n", i, _pages_amount);
            break;
        }

        uint32_t pt_phys_addr = (pte_idx << 12) + (i * _PAGE_SIZE);
        PTE* pt_base = (PTE*)pt_phys_addr;
        
        if (i % 32 == 0) Sys_log("Building PT[%u] at %x\n", i, pt_phys_addr);
        
        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t page_idx = i * 1024 + j;
            if (page_idx >= _pages_amount) break;
            
            uint8_t is_present = page_idx < kend ? 1 : 0;
            pt_base[j].present = is_present;
            pt_base[j].rw = 1;
            pt_base[j].user = 0;
            pt_base[j].addr = page_idx;
            if (page_idx == 0) pt_base[j].rw = 0;
        }

        Sys_log("Setting PDE[%u] to page %x\n", i, pte_idx + i);
        _k_pd.pde_arr[i].present = 1;
        _k_pd.pde_arr[i].rw = 1;
        _k_pd.pde_arr[i].user = 0;
        _k_pd.pde_arr[i].page_size = 0;
        _k_pd.pde_arr[i].addr = pte_idx + i;
    }
    
    Sys_log("Page tables built successfully\n");
    Sys_log("PD address: %x\n", (uint32_t)_k_pd.pde_arr);
    Sys_log("PDE[0]: present=%u addr=%x\n", _k_pd.pde_arr[0].present, _k_pd.pde_arr[0].addr);
    Sys_log("PDE[1]: present=%u addr=%x\n", _k_pd.pde_arr[1].present, _k_pd.pde_arr[1].addr);
    
    Sys_log("About to enable paging with CR3=%x\n", (uint32_t)_k_pd.pde_arr);
    Sys_log("Kernel code currently at %x\n", &setup_paging);
    
    asm volatile (
        "mov %0, %%cr3 \n\t"
        "mov %%cr0, %%eax \n\t"
        "or $0x80000000, %%eax \n\t"
        "mov %%eax, %%cr0"
        :
        : "r"((uint32_t)_k_pd.pde_arr)
        : "eax"
    );

    Sys_log("Paging enabled successfully\n");
    Sys_log("Paging enabled successfully\n");
    
    Sys_log("Unmapping page 0...\n");
    int unmap_result = unmap_page(&_k_pd, 0);
    Sys_log("Unmap result: %d\n", unmap_result);
    
    Sys_log("Paging set up successfully.\n");
    return 0;
}


void reserve_kernel_pages() {
    uintptr_t kend = (uintptr_t)&_kernel_end;

    uint32_t end_page = (kend + _PAGE_SIZE - 1) >> 12;
    uint32_t reserved = 0;

    for (uint32_t i = 0; i < end_page && i < _pages_amount; i++) {
        _free_pages_bitmap[i / 32] &= ~(1u << (i % 32));
        reserved++;
    }
}


void k_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present,
              uint8_t rw, uint8_t user) {
    pd_map_page(&_k_pd, virtual_addr, physical_addr, present, rw, user);
}

void pd_map_page(PD_t*  pd, uint32_t virtual_addr, uint32_t physical_addr, uint8_t present,
              uint8_t rw, uint8_t user) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    PDE* pde = &pd->pde_arr[pd_index];

    PTE* pt_base;
    if (!pde->present) {
        pt_base = (PTE*)((uint32_t)page_alloc(1, 1, 0) << 12);
        if (!pt_base) return;

        memset(pt_base, 0, _PAGE_SIZE);

        pde->present = 1;
        pde->rw = rw;
        pde->user = user;
        pde->page_size = 0;
        pde->addr = ((uintptr_t)pt_base) >> 12;
    } else {
        pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    }

    PTE* pte = &pt_base[pt_index];

    sleep(0);

    pte->present = present;
    pte->rw = rw;
    pte->user = user;
    pte->addr = physical_addr >> 12;

    asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}


int unmap_page(PD_t* target_pd, uint32_t virtual_addr) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    Sys_log("unmap_page: va=%x pd_idx=%u pt_idx=%u\n", virtual_addr, pd_index, pt_index);

    PDE* pde = &target_pd->pde_arr[pd_index];
    if (!pde->present) {
        Sys_log("unmap_page: PDE not present\n");
        return -1;
    }

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    Sys_log("unmap_page: pt_base=%x\n", (uint32_t)pt_base);
    
    PTE* pte = &pt_base[pt_index];
    if (!pte->present) {
        Sys_log("unmap_page: PTE not present\n");
        return -2;
    }

    pte->present = 0;
    Sys_log("unmap_page: cleared PTE\n");

    asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
    Sys_log("unmap_page: invlpg done\n");

    return 0;
}


PTE* get_pte(uint32_t index) {
    uint32_t pd_index = index >> 10; 
    uint32_t pt_index = index & 0x3FF;

    Sys_log("get_pte: index=%x pd_idx=%u pt_idx=%u\n", index, pd_index, pt_index);

    PDE* pde = &_k_pd.pde_arr[pd_index];
    if (!pde->present) {
        Sys_log("get_pte: PDE not present\n");
        return NULL;
    }

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    Sys_log("get_pte: pt_base=%x\n", (uint32_t)pt_base);
    
    return &pt_base[pt_index];
}


PTE* get_pte_for_pa(uint32_t pa) {
    uint32_t index = pa >> 12;
    return get_pte(index);
}


page_index page_alloc(size_t amount, int read_write, int user_supervisor) {
    Sys_log("page_alloc: looking for %u contiguous pages\n", (unsigned)amount);
    
    size_t found = 0;
    page_index start = 0;

    for (uint32_t i = 0; i < _pages_amount; i++) {
        uint32_t bitmap_idx = i / 32;
        uint32_t bit_pos = i % 32;
        
        if (_free_pages_bitmap[bitmap_idx] & (1u << bit_pos)) {
            if (found == 0) start = i;
            found++;
            if (found == amount) {
                Sys_log("page_alloc: found %u pages starting at %x\n", (unsigned)amount, start);
                
                for (uint32_t j = 0; j < amount; j++) {
                    uint32_t idx = start + j;
                    uint32_t bm_idx = idx / 32;
                    uint32_t bp = idx % 32;
                    _free_pages_bitmap[bm_idx] &= ~(1u << bp);
                    
                    if (j % 64 == 0) {
                        Sys_log("page_alloc: marking page %x as used\n", idx);
                    }
                }
                Sys_log("page_alloc called for %u pages at index %x\n", (unsigned)amount, start);
                return start;
            }
        } else {
            found = 0;
        }
    }

    Sys_Error("page_alloc for %d page(s) failed: not enough contiguous free pages\n", (unsigned)amount);
    return 0;
}

void dump_pd() {
    uint32_t* pdes = (uint32_t*)_k_pd.pde_arr;
    Sys_log("Dumping Page Directory from %x\n", _k_pd.pde_arr);
    Sys_log("first pde: %x\n", ((PTE*)pdes)[0].addr * _PAGE_SIZE);
    for (int i = 0; i < 1; i++) {
        if (!(pdes[i] & 1)) continue;
        uint32_t base = pdes[i] & 0xFFFFF000;
        uint32_t* pt = (uint32_t*)base;
        for (int j = 0; j < 300; j++) {
            if (pt[j] & 1) {
                uint32_t pa = pt[j] & 0xFFFFF000;
                Sys_log("PD[%03d] PT[%03d] VA=%x -> data=%x\n", i, j, (i<<22)|(j<<12), pt[j]);
            }
        }
    }
    while (1);
}


void page_free(page_index pa, size_t amount) {
    uint32_t start_index = pa >> 12;

    for (uint32_t i = 0; i < amount; i++) {
        uint32_t idx = start_index + i;
        if (idx >= _pages_amount) break;

        _free_pages_bitmap[idx / 32] |= (1u << (idx % 32));

        PTE* pte = get_pte(idx);
        if (pte) pte->present = 0;
    }
}


uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, PD_t* out_pd_t) {
    if (!groups || group_count == 0) return 0;

    page_index pd_idx = page_alloc(1, 1, 0);
    if (!pd_idx) return 0;
    
    out_pd_t->pde_arr = (PDE*)(pd_idx << 12);
    memset(out_pd_t->pde_arr, 0, 1024 * sizeof(PDE));

    for (uint32_t j = 0; j < group_count; j++) {
        Page_Group* group = &groups[j];
        if (group->size == 0) continue;

        page_index virt_addr = group->addr;
        page_index phys_addr = group->pte_bits.addr;

        uint32_t start_page_idx = virt_addr >> 12;

        for (uint32_t i = 0; i < group->size; i++) {
            uint32_t page_idx = start_page_idx + i;
            uint32_t pd_i = page_idx >> 10;      
            uint32_t pt_i = page_idx & 0x3FF;     

            if (!out_pd_t->pde_arr[pd_i].present) {
                page_index pt_idx = page_alloc(1, 1, 0);
                if (!pt_idx) return 0;
                
                PTE* pt_base = (PTE*)(pt_idx << 12);
                memset(pt_base, 0, _PAGE_SIZE);

                out_pd_t->pde_arr[pd_i].present = 1;
                out_pd_t->pde_arr[pd_i].rw = 1;
                out_pd_t->pde_arr[pd_i].user = 0;
                out_pd_t->pde_arr[pd_i].page_size = 0;
                out_pd_t->pde_arr[pd_i].addr = pt_idx;
            }

            PTE* pt_base = (PTE*)((uintptr_t)(out_pd_t->pde_arr[pd_i].addr) << 12);
            PTE entry = group->pte_bits;
            entry.addr = phys_addr + i;
            pt_base[pt_i] = entry;
        }
    }

    if (!v_map(out_pd_t, groups, group_count)) {
        return 0;
    }

    return pd_idx << 12;
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

            page_free((page_index)pte->addr << 12, 1);
        }

        page_free((page_index)pt_base >> 12, 1);
    }

    page_free((page_index)pd->pde_arr >> 12, 1);
    pd->pde_arr = NULL;
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
                page_index pt_idx = page_alloc(1, 1, 0);
                if (!pt_idx) return false;
                
                pt_base = (PTE*)(pt_idx << 12);
                memset(pt_base, 0, _PAGE_SIZE);

                pde->present = 1;
                pde->rw = 1;
                pde->user = 0;
                pde->page_size = 0;
                pde->addr = pt_idx;
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
            page_index pt_idx = page_alloc(1, 1, 0);
            if (!pt_idx) return 0;
            
            pt_base = (PTE*)(pt_idx << 12);
            memset(pt_base, 0, _PAGE_SIZE);

            pde->present = 1;
            pde->rw = 1;
            pde->user = 0;
            pde->page_size = 0;
            pde->addr = pt_idx;
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
    return !((_free_pages_bitmap[idx / 32] >> (idx % 32)) & 1u);
}

void page_force_alloc(page_index pa, size_t amount) {
    uint32_t start = pa >> 12;
    for (size_t i = 0; i < amount; i++) {
        uint32_t idx = start + i;
        if (idx >= _pages_amount) break;

        _free_pages_bitmap[idx / 32] &= ~(1u << (idx % 32));
    }
}

page_index k_append_pages(page_index phys_start_page, uint32_t amount, uint8_t rw, uint8_t us) {
    uint32_t start_page_idx = 0;
    uint32_t found = 0;

    for (page_index i = 1; i < KERNEL_PDE_COUNT * 1024; i++) {
        uint32_t pt_offset = (i / 1024) * (_PAGE_SIZE / 4) + (i % 1024);
        PTE* curr_pte = &kernel_page_table_ptr[pt_offset];
        
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
        Sys_log("k_append_pages failed: not enough contiguous free pages\n");
        return 0;
    }
    
    for (uint32_t j = 0; j < amount; j++) {
        uint32_t idx = start_page_idx + j;
        uint32_t pt_offset = (idx / 1024) * (_PAGE_SIZE / 4) + (idx % 1024);
        PTE* pte = &kernel_page_table_ptr[pt_offset];

        pte->present = 1;
        pte->rw = rw ? 1 : 0;
        pte->user = us ? 1 : 0;
        pte->addr = phys_start_page + j;
        asm volatile("invlpg (%0)" : : "r"(idx << 12) : "memory");
    }
    
    Sys_log("k_append_pages mapped %u pages at VA %x\n", amount, start_page_idx * _PAGE_SIZE);
    return start_page_idx;
}