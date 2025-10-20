#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/multiboot_info.h"
#include "headers/string.h"
#include "headers/io.h"

static PD_t pd;

extern char _kernel_start;
extern char _kernel_end;

uint8_t _free_pages_bitmap[131072];
uint32_t _page_amount = 0;

int setup_paging() {
    memset(_free_pages_bitmap, 0xFF, sizeof(_free_pages_bitmap));
    reserve_kernel_pages();

    Sys_log("%X", _free_pages_bitmap[0]);

    uint32_t pages_amount = (Get_multiboot_info()->mem_upper + 1024) / 4;
    if (pages_amount < MIN_OS_PAGES * 1.5) return -1;
    if (pages_amount > 1024 * 1024) pages_amount = 1024 * 1024;

    _page_amount = pages_amount;

    _free_pages_bitmap[0] &= ~(1 << 0);
    uintptr_t kstart = (uintptr_t)&_kernel_start;
    uintptr_t kend = (uintptr_t)&_kernel_end;
    for (uint32_t i = kstart >> 12; i < (kend + _PAGE_SIZE - 1) >> 12 && i < _page_amount; i++)
        _free_pages_bitmap[i / 8] &= ~(1 << (i % 8));

    pd.pde_arr = (PDE*)page_alloc(1, 1, 0);

    for (uint32_t i = 0; i < 1024; i++) {
        if (i * 1024 >= pages_amount) break;

        PTE* pt_base = (PTE*)page_alloc(1, 1, 0);
        if (!pt_base) return -1;
        memset(pt_base, 0, _PAGE_SIZE);

        for (uint32_t j = 0; j < 1024; j++) {
            uint32_t page_idx = i * 1024 + j;
            if (page_idx >= pages_amount) break;

            pt_base[j].present = 1;
            pt_base[j].rw = 1;
            pt_base[j].user = 0;
            pt_base[j].addr = page_idx;
            if (page_idx == 0) pt_base[j].rw = 0;
        }

        pd.pde_arr[i].present = 1;
        pd.pde_arr[i].rw = 1;
        pd.pde_arr[i].user = 0;
        pd.pde_arr[i].page_size = 0;
        pd.pde_arr[i].addr = ((uintptr_t)pt_base) >> 12;
    }

    Sys_log("pd is at %x\n", pd.pde_arr);

    asm volatile (
        "mov %0, %%cr3 \n\t"
        "mov %%cr0, %%eax \n\t"
        "or $0x80000000, %%eax \n\t"
        "mov %%eax, %%cr0"
        :
        : "r"(pd.pde_arr)
        : "eax"
    );

    map_page(0, 0, 1, 0, 0);
    
    
    return 0;
}


void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present,
              uint8_t rw, uint8_t user) {
    uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
    uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

    PDE* pde = &pd.pde_arr[pd_index];

    PTE* pt_base;
    if (!pde->present) {
        pt_base = (PTE*)page_alloc(1, 1, 0);
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
    pte->present = present;
    pte->rw = rw;
    pte->user = user;
    pte->addr = physical_addr >> 12;

    asm volatile("invlpg (%0)" : : "r"(virtual_addr) : "memory");
}



PTE* get_pte(uint32_t index) {
    uint32_t pd_index = index >> 10; 
    uint32_t pt_index = index & 0x3FF;

    PDE* pde = &pd.pde_arr[pd_index];
    if (!pde->present) return NULL;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    return &pt_base[pt_index];
}



PTE* get_pte_for_pa(uint32_t pa) {
    uint32_t index = pa >> 12;
    uint32_t pd_index = index >> 10;
    uint32_t pt_index = index & 0x3FF;

    if (pd_index >= 1024) return NULL;

    PDE* pde = &pd.pde_arr[pd_index];
    if (!pde->present) return NULL;

    PTE* pt_base = (PTE*)((uintptr_t)pde->addr << 12);
    return &pt_base[pt_index];
}


page_addr_t pagealloc(size_t amount){
    uint32_t start = 0;
    size_t found = 0;
    for(uint32_t i=0;i<_page_amount;i++){
        if(_free_pages_bitmap[i/8] & (1 << (i%8))){
            if(found==0) start = i;
            found++;
            if(found==amount){
                for(uint32_t j=0;j<amount;j++){
                    _free_pages_bitmap[(start + j)/8] &= ~(1 << ((start + j)%8));
                }
                return start * _PAGE_SIZE;
            }
        } else { found = 0; }
    }
    return 0;
}

void pagefree(page_addr_t pa, size_t amount){
    uint32_t start = pa >> 12;
    for(uint32_t i=0;i<amount;i++){
        uint32_t idx = start + i;
        if(idx < _page_amount) _free_pages_bitmap[idx/8] |= (1 << (idx%8));
    }
}

page_addr_t page_alloc(size_t amount, int read_write, int user_supervisor) {
    
    size_t found = 0;
    page_addr_t start = 0;

    for (uint32_t i = 0; i < _page_amount; i++) {
        if (_free_pages_bitmap[i/8] & (1 << (i%8))) {
            if (found == 0) start = i;
            found++;
            if (found == amount) {
                for (uint32_t j = 0; j < amount; j++) {
                    uint32_t idx = start + j;
                    _free_pages_bitmap[idx/8] &= ~(1 << (idx%8));

                    PTE* p = get_pte(idx);
                    if (p) {
                        p->present = 1;
                        p->rw = read_write ? 1 : 0;
                        p->user = user_supervisor ? 1 : 0;
                        p->addr = idx;
                    }
                }
                Sys_log("page_alloc called for %u pages (%x)\n", (unsigned)amount, start * _PAGE_SIZE);
                return start * _PAGE_SIZE;
            }
        } else {
            found = 0;
        }
    }

    Sys_log("page_alloc failed: not enough contiguous free pages\n");
    return 0;
}

void dump_pd() {
    uint32_t* pdes = (uint32_t*)pd.pde_arr;
    Sys_log("Dumping Page Directory from %x\n", pd.pde_arr);
    Sys_log("first pde: %x\n", ((PTE*)pdes)[0].addr * _PAGE_SIZE);
    for (int i = 0; i < 1; i++) {
        if (!(pdes[i] & 1)) continue;
        uint32_t base = pdes[i] & 0xFFFFF000;
        uint32_t* pt = (uint32_t*)base;
        for (int j = 0; j < 300; j++) {
            if (1||pt[j] & 1) {
                uint32_t pa = pt[j] & 0xFFFFF000;
                Sys_log("PD[%03d] PT[%03d] VA=%x -> data=%x\n", i, j, (i<<22)|(j<<12), pt[j]);
            }
        }
    }
    while (1);
    
}




void page_free(page_addr_t pa, size_t amount) {
    uint32_t start_index = pa >> 12;

    for (uint32_t i = 0; i < amount; i++) {
        uint32_t idx = start_index + i;
        if (idx >= _page_amount) break;

        _free_pages_bitmap[idx / 8] |= (1 << (idx % 8));

        PTE* pte = get_pte(idx);
        if (pte) pte->present = 0;
    }
}


void reserve_kernel_pages() {
    Sys_log("Reserving kernel pages from %p to %p\n", &_kernel_start, &_kernel_end);

    memset(_free_pages_bitmap, 0, ((uintptr_t)&_kernel_end / _PAGE_SIZE + 7) / 8);

    // uintptr_t kstart = 0;
    // uintptr_t kend = (uintptr_t)&_kernel_end;
    // for (uint32_t i = kstart >> 12; i < (kend + _PAGE_SIZE - 1) >> 12 && i < _page_amount; i++){
    //     _free_pages_bitmap[i / 8] &= ~(1 << (i % 8));
    // }
}

uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, PD_t* out_pd_t ) {
    if (!groups || group_count == 0) return 0;


    void* pd_addr = (void*)page_alloc(1, 1, 0);
    if (!pd_addr) return 0;
    out_pd_t->pde_arr = (PDE*)pd_addr;

    memset(out_pd_t->pde_arr, 0, 1024 * sizeof(PDE));

    for (uint32_t j = 0; j < group_count; j++) {
        Page_Group* group = &groups[j];
        if (group->size == 0) continue;

        page_addr_t virt_addr = group->addr;
        page_addr_t phys_addr = group->pte_bits.addr;

        uint32_t start_page_idx = virt_addr >> 12;
        uint32_t end_page_idx = start_page_idx + group->size - 1;

        for (uint32_t i = 0; i < group->size; i++) {
            uint32_t page_idx = start_page_idx + i;

            uint32_t pd_i = page_idx >> 10;      
            uint32_t pt_i = page_idx & 0x3FF;     

            if (!out_pd_t->pde_arr[pd_i].present) {
                PTE* pt_base = (PTE*)page_alloc(1, 1, 0);
                if (!pt_base) return 0;
                memset(pt_base, 0, _PAGE_SIZE);

                out_pd_t->pde_arr[pd_i].present = 1;
                out_pd_t->pde_arr[pd_i].rw = 1;
                out_pd_t->pde_arr[pd_i].user = 0;
                out_pd_t->pde_arr[pd_i].page_size = 0;
                out_pd_t->pde_arr[pd_i].addr = ((page_addr_t)pt_base) >> 12;
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
                memset(pt_base, 0, _PAGE_SIZE);

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
            memset(pt_base, 0, _PAGE_SIZE);

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

int is_page_allocated(page_addr_t pa) {
    uint32_t idx = (uint32_t)(pa >> 12);
    if (idx >= _page_amount) return -1;
    return !((_free_pages_bitmap[idx / 8] >> (idx % 8)) & 1);
}

void page_force_alloc(page_addr_t pa, size_t amount) {
    uint32_t start = pa >> 12;
    for (size_t i = 0; i < amount; i++) {
        uint32_t idx = start + i;
        if (idx >= _page_amount) break;

        _free_pages_bitmap[idx / 8] &= ~(1 << (idx % 8));
        PTE* p = get_pte(idx);
        if (p) {
            p->present = 1;
            p->rw = 1;
            p->user = 0;
            p->addr = idx;
        }
    }
}

