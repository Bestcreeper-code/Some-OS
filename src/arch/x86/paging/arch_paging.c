#include "asm/arch_asm.h"
#include "bootloader.h"
#include "err_codes.h"
#include "paging.h"
#include "paging/arch_paging.h"
// #include "memory.h"

#include "string.h"
// #include "io.h"
#include "time.h"
#include "helpers.h"
#include "Logger.h"
#include "memory.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdnoreturn.h>

volatile PD_t _k_pd;

volatile uintptr_t _k_pd_phys;

extern char _kernel_start;
extern char _kernel_end;


// 0 = used, 1 = free
uint32_t _free_pages_bitmap[(1024 * 1024 / sizeof(uint32_t)) ];

uint32_t _pages_amount = 0;
uint32_t _pages_tables_amount = 0;

PTE* kernel_page_table_ptr;

static early_reservation_t _early_page_reservations[EARLY_RESERVATION_MAX]
    __attribute__((aligned(PAGE_SIZE)));
 
static int _early_reservation_count = 0;


int setup_paging() {
    Sys_log("Setting up paging...\n");

    _pages_amount = ((get_bootloader_mem_info()->mem_upper * 1024) + 1024*1024) / PAGE_SIZE;
    if (_pages_amount < MIN_OS_PAGES * 1.5) return -E_NOMEM;
    if (_pages_amount > 1024 * 1024) _pages_amount = 1024 * 1024;

    _pages_tables_amount = (_pages_amount + 1023) / 1024;

    dw_memset(_free_pages_bitmap, 0xFFFFFFFF, sizeof(_free_pages_bitmap)/sizeof(uint32_t));
    reserve_kernel_pages();

    for(int i = 0; i < _early_reservation_count && i < EARLY_RESERVATION_MAX; i++){
        for (int j = 0; j < _early_page_reservations[i].size; j++) {
            bitmap_zero_bit((char*)_free_pages_bitmap, _early_page_reservations[i].phys_base + j);
        }
    }
    
    page_index pd_phys_page = page_alloc_nomap(1);
    uintptr_t  pd_phys      = pd_phys_page << 12;
    PDE*       pd_virt      = (PDE*)HHDM_TO_VIRT(pd_phys);

    _k_pd.pde_arr = pd_virt;
    dw_memset(pd_virt, 0, PAGE_SIZE / 4);

    
    page_index pt_phys_start_page = page_alloc_nomap(_MAX_PT_AMOUNT);
    uintptr_t  pt_phys_start      = pt_phys_start_page << 12;
    PTE*       k_pte_array        = (PTE*)HHDM_TO_VIRT(pt_phys_start);

    dw_memset(k_pte_array, 0, _MAX_PT_AMOUNT * PAGE_SIZE);
    kernel_page_table_ptr = k_pte_array;

    
    uint32_t kernel_pde_base = KERNEL_VMA >> 22;

    for (uint32_t i = 0; i < _MAX_PT_AMOUNT; i++) {
        if (i * 1024 >= _pages_amount) break;

        uintptr_t pt_phys = pt_phys_start + i * PAGE_SIZE;
        PTE*      pt_virt = (PTE*)HHDM_TO_VIRT(pt_phys);

        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t page_idx = i * 1024 + j;
            if (page_idx >= _pages_amount) break;

            bool used = !(_free_pages_bitmap[page_idx / 32] & (1u << (page_idx % 32)));
            pt_virt[j].present = used;
            pt_virt[j].rw      = 1;
            pt_virt[j].user    = 0;
            pt_virt[j].addr    = page_idx;
            if (page_idx == 0) pt_virt[j].rw = 0;
        }

        uint32_t pde_idx = kernel_pde_base + i;
        if (pde_idx >= 1024) break;

        pd_virt[pde_idx].present   = 1;
        pd_virt[pde_idx].rw        = 1;
        pd_virt[pde_idx].user      = 0;
        pd_virt[pde_idx].page_size = 0;
        pd_virt[pde_idx].addr      = pt_phys >> 12;
    }

    Sys_log("Switching CR3 to new PD (phys=0x%x)\n", pd_phys);
    
    asm volatile ("mov %0, %%cr3" :: "r"(pd_phys) : "memory");
    _k_pd_phys = pd_phys;
    Sys_log("Paging set up successfully.\n");
    page_reclaim_early_reservation_table();
    Sys_log("Reclaimed page res entries memory\n");
    return 0;
}


void reserve_kernel_pages() {
    // _kernel_end is VMA, convert to physical
    uintptr_t kend_phys = HHDM_TO_PHYS((uintptr_t)&_kernel_end);
    uint32_t end_page = (kend_phys + PAGE_SIZE - 1) >> 12;

    for (uint32_t i = 0; i < end_page && i < _pages_amount; i++) {
        _free_pages_bitmap[i / 32] &= ~(1u << (i % 32));
    }
    Sys_log("Reserved up to physical page %u (0x%x)\n", end_page, kend_phys);
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


// #error make that shit work and not a shitty cod from me being tired some time ago
void pd_map_page(PD_t* pd, uint32_t virtual_addr, uint32_t physical_addr,
                 uint8_t present, uint8_t rw, uint8_t user) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    volatile PDE* pde = &pd->pde_arr[pd_index];
    PTE* pt_base;

    if (!pde->present) {
        page_index pt_phys_page = page_alloc_nomap(1);
        uintptr_t  pt_phys      = pt_phys_page << 12;
        pt_base = (PTE*)HHDM_TO_VIRT(pt_phys);
        memset(pt_base, 0, PAGE_SIZE);

        pde->present   = 1;
        pde->rw        = rw;
        pde->user      = user;
        pde->page_size = 0;
        pde->addr      = pt_phys_page;
    } else {
        
        pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
    }

    PTE* pte    = &pt_base[pt_index];
    pte->present = present;
    pte->rw      = rw;
    pte->user    = user;
    pte->addr    = physical_addr >> 12;

    tlb_flush_page(virtual_addr);
}



int map_page(page_index virt, page_index phys, uint8_t present, uint8_t rw, uint8_t user) {
    pd_map_page((PD_t*)&_k_pd, (uint32_t)(virt << 12), (uint32_t)(phys << 12), present, rw, user);
    return 0;
}


int map_range(page_index virt, page_index phys, size_t count, char flags) {
    uint8_t rw   = (flags & PAGE_FLAG_RW)   ? 1 : 0;
    uint8_t user = (flags & PAGE_FLAG_USER) ? 1 : 0;
    for (size_t i = 0; i < count; i++) {
        pd_map_page((PD_t*)&_k_pd, (uint32_t)((virt + i) << 12), (uint32_t)((phys + i) << 12), 1, rw, user);
    }
    return 0;
}


int pd_unmap_page(PD_t* target_pd, uint32_t virtual_addr) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    volatile PDE* pde = &target_pd->pde_arr[pd_index];
    if (!pde->present) return -1;

    PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
    PTE* pte = &pt_base[pt_index];
    if (!pte->present) return -2;

    pte->present = 0;

    tlb_flush_page(virtual_addr);

    return 0;
}


int unmap_page(page_index virt) {
    return pd_unmap_page((PD_t*)&_k_pd, (uint32_t)(virt << 12));
}




PTE* get_pte(uint32_t index) {
    uint32_t pd_index = index >> 10;
    uint32_t pt_index = index & 0x3FF;

    volatile PDE* pde = &_k_pd.pde_arr[pd_index];
    if (!pde || !pde->present) return NULL;

    
    PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
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

    volatile PDE* pde = &_k_pd.pde_arr[pd_i];
    if (!pde) return result;
    if (!pde->present) return result;
    if (!pde->addr) return result;

    PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
    if (!pt_base) return result;
    PTE* pte     = &pt_base[pt_i];
    if (!pte) return result;

    result.present = pte->present;
    result.index    = (uintptr_t)pte->addr ;
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
#if PAGE_DEBUG
                Sys_Success("page_alloc_nomap success for %u pages\n", (unsigned)amount );
#endif
                return start;
            }
        } else {
            found = 0;
        }
    }
#if PAGE_DEBUG
    Sys_Error("page_alloc_nomap for %d page(s) failed: not enough contiguous free pages\n", amount);
#endif
    return 0;
}




page_index page_alloc(size_t amount, char flags) {
    page_index phys = page_alloc_nomap(amount);
    if (!phys){
#if PAGE_DEBUG
        Sys_Error("page_alloc_nomap failed for %u pages (p_page: 0x%x) with %d used ram\n", (unsigned)amount, phys, get_used_ram() );
#endif
        return 0;
    }

    page_index virt = vmap(phys, amount, flags);
    if (!virt) {
        page_free(phys, amount);
#if PAGE_DEBUG
        Sys_Error("page_alloc failed for %u pages (p_page: 0x%x) with mapping\n", (unsigned)amount, phys );
#endif
        return 0;
    }
#if PAGE_DEBUG
Sys_log("page_alloc called for %u pages (page: 0x%x) with mapping\n", (unsigned)amount, virt );
#endif
    return virt;  
}


void page_free(page_index pa, size_t amount) {
    uint32_t start_index = pa;

    for (uint32_t i = 0; i < amount; i++) {
        uint32_t idx = start_index + i;
        if (idx >= _pages_amount) break;
        PTE* pte = get_pte(idx);

        
        if (pte){
            _free_pages_bitmap[pte->addr / 32] |= (1 << (pte->addr % 32));
            pte->present = 0;
        }    
    }
}


void dump_pd(PD_t* pd) {
    uint32_t* pdes = (uint32_t*) PAGE_ADDR(vmap(ADDR_TO_PAGE(pd->pde_arr),1,PAGE_FLAG_RW));
    Sys_log("Dumping Page Directory from %x\n", (uint32_t)_k_pd.pde_arr);
    Sys_log("first pde: %x\n", ((PTE*)pdes)[0].addr * PAGE_SIZE);
    for (int i = 0; i < 1024; i++) {
        if (!(pdes[i] & 1)){Sys_log("no pde %x     %x\n",i, pdes[i]); continue;}
        uint32_t base = pdes[i] & 0xFFFFF000;
        uint32_t* pt = (uint32_t*)base;
        for (int j = 0; j < 1024; j++) {
            if (1||pt[j] & 1) {
                uint32_t pa = pt[j] & 0xFFFFF000;
                Sys_log("PD[%03d] PT[%03d] VA=%x -> data=%x\n", i, j, (i<<22)|(j<<12), pt[j]);
            }
        }
    }
}

void debug_dump_kernel_mapping(PD_t* pd, uint32_t va) {
    uint32_t pd_i = (va >> 22) & 0x3FF;
    uint32_t pt_i = (va >> 12) & 0x3FF;

    volatile PDE* pde = &pd->pde_arr[pd_i];
    Sys_Warning("K-VA 0x%x: PDE[%u] present=%u rw=%u user=%u addr=0x%x\n",
            va, pd_i, pde->present, pde->rw, pde->user, pde->addr << 12);

    if (!pde->present) return;

    PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
    PTE* pte = &pt_base[pt_i];

    Sys_Warning("           PTE[%u] present=%u rw=%u user=%u addr=0x%x\n",
            pt_i, pte->present, pte->rw, pte->user, pte->addr << 12);
}


void pd_free(PD_t* pd) {
    if (!pd || !pd->pde_arr) return;

    for (uint32_t pd_i = 0; pd_i < 1024; pd_i++) {
        volatile PDE* pde = &pd->pde_arr[pd_i];
        if (!pde->present || !pde->user) continue;

        PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
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


uintptr_t PD_append_pages(PD_t* page_dir, PTE* ptes, uint32_t pte_count) {
    if (!page_dir || !page_dir->pde_arr || !ptes || pte_count == 0) return 0;

    uint32_t max_vpages = 1024 * 1024; 
    uint32_t free_run_start = 0;
    uint32_t run_length = 0;

    for (uint32_t i = 0; i < max_vpages; i++) {
        uint32_t pd_i = i >> 10;
        uint32_t pt_i = i & 0x3FF;

        volatile PDE* pde = &page_dir->pde_arr[pd_i];

        if (!pde->present) {
            run_length++;
        } else {
            PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
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
#if PAGE_DEBUG
        Sys_log("failed for %u pages\n", pte_count);
#endif
        return 0;
    }

    uint32_t base_page_idx = free_run_start;
    for (uint32_t i = 0; i < pte_count; i++) {
        uint32_t virt_page_idx = base_page_idx + i;
        uint32_t pd_i = virt_page_idx >> 10;
        uint32_t pt_i = virt_page_idx & 0x3FF;

        volatile PDE* pde = &page_dir->pde_arr[pd_i];
        PTE* pt_base;

        if (!pde->present) {
            pt_base = (PTE*)page_alloc(1, PAGE_FLAG_RW);
            if (!pt_base) return 0;
            memset(pt_base, 0, PAGE_SIZE);

            pde->present = 1;
            pde->rw = 1;
            pde->user = 0;
            pde->page_size = 0;
            
            uintptr_t phys = HHDM_TO_PHYS((uintptr_t)pt_base);
            pde->addr = phys >> 12;

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
#if PAGE_DEBUG
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
#if PAGE_DEBUG
        tlb_flush_page(idx);
#endif
    }
#if PAGE_DEBUG
    Sys_log("k_append_pages mapped %u pages at VA %x\n", amount, start_page_idx * PAGE_SIZE);
#endif
    return start_page_idx;
}

int page_write(page_index virt, PAGE page) {
    uint32_t va   = (uint32_t)(virt << 12);
    uint32_t pd_i = (va >> 22) & 0x3FF;
    uint32_t pt_i = (va >> 12) & 0x3FF;

    volatile PDE* pde = &_k_pd.pde_arr[pd_i];
    if (!pde->present) return -1;

    PTE* pt_base = (PTE*)HHDM_TO_VIRT((uintptr_t)pde->addr << 12);
    PTE* pte     = &pt_base[pt_i];

    pte->present = page.present;
    pte->rw      = page.rw;
    pte->user    = page.us;
    pte->addr    = page.index;

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

// #error  wtf all 0??
void pd_init(PD_t* pd) {
    if (!pd || !pd->pde_arr){
        Sys_Error("erm..wtf");
        return;
    }
    PDE* pde_arr = (PDE*)(vmap(ADDR_TO_PAGE(pd->pde_arr),1,PAGE_FLAG_RW)<<12);

    memset((void*)pd->pde_arr, 0, PAGE_SIZE);

    uint32_t pde_start = KVSPACE_FIRST_PAGE >> 10;
    uint32_t pde_end   = KVSPACE_LAST_PAGE  >> 10;

    for (uint32_t i = 0; i < pde_end; i++) {
        
Sys_Info("frmt, ... %d",i);
        pd->pde_arr[i] = _k_pd.pde_arr[i];

    }
    dump_pd(pd);
    Sys_Success("yippee");
}




 
int page_reserve_page_early(uintptr_t phys_base, size_t size) {
    if (_early_reservation_count >= EARLY_RESERVATION_MAX) {
        Sys_Error("table full\n");
        return -1;
    }
    _early_page_reservations[_early_reservation_count].phys_base = phys_base;
    _early_page_reservations[_early_reservation_count].size      = size;
    _early_reservation_count++;
    return 0;
}
 
const early_reservation_t* page_early_reservations(int *out_count) {
    if (out_count) *out_count = _early_reservation_count;
    return _early_page_reservations;
}
 
void page_reclaim_early_reservation_table(void) {
    dw_memset(_early_page_reservations, 0, PAGE_SIZE / 4);
    _early_reservation_count = 0;
 
    force_free((uintptr_t)_early_page_reservations, PAGE_SIZE);
 
    Sys_log("Early reservation table reclaimed (phys=0x%x, %u bytes)\n",
            (uint32_t)(uintptr_t)_early_page_reservations, (uint32_t)PAGE_SIZE);
}
 



// int parse_memory_map() {
    // struct bl_mem_info* mem_info = get_bootloader_mem_info();
    // Sys_log("  Base = 0x%llx, Length = 0x%llx, Type = %u, Entry size = 0x%x zr,gcfh,nh,dbtvr %p\n", 
    //     mem_info->mmap_addr[0].addr, mem_info->mmap_addr[0].len, mem_info->mmap_addr[0].type,mem_info->mmap_addr[0].size,mem_info);
    // Sys_log("Parsing memory map...\n");

    // ram_amount = ((mem_info->mem_upper) + mem_info->mem_lower)*1024;

    // free_region_map_t* k_mmap = get_free_region_map();
    // //cleanup the k_mmap
    // k_mmap->free_region_count = 0;
    // memset(k_mmap->free_regions, 0, sizeof(free_region_t) * MAX_FREE_REGIONS);
    
    // if (!check_bl_flag(BL_BOOT_FLAG_MEM_MAP)) {
    //     Sys_Error("Bootloader mmap not present\n");
    //     RET_ERR(E_INVAL);
    // }

    // uintptr_t mmap_end = (uintptr_t)mem_info->mmap_addr + mem_info->mmap_length;
    
    // struct bootloader_mmap_entry* mmap = mem_info->mmap_addr;

    // Sys_log("Bootloader mmap: \n");
    
    // while ((uintptr_t)mmap < mmap_end) {
    //     uintptr_t base = mmap->addr & ~0xFFF;
    //     uintptr_t len  = (mmap->len + 0xFFF) & ~0xFFF;
    //     uintptr_t page_end = base + len;
        
    //     Sys_log("  Base = 0x%llx, Length = 0x%llx, Type = %u, Entry size = 0x%x\n", mmap->addr, mmap->len, mmap->type,mmap->size);
        
    //     mmap = (struct bootloader_mmap_entry*)((uintptr_t)mmap + mmap->size + sizeof(mmap->size));
    // }
    // Sys_log("mmap end \n");
