#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef uintptr_t page_index;

#define MIN_OS_PAGES 8192
#define _PAGE_SIZE 4096
#define _PT_SIZE _PAGE_SIZE/sizeof(PTE)

#define _MAX_PT_AMOUNT 1024

#define KERNEL_PDE_COUNT 128

#define _PAGETABLE_MAPPED_SIZE 0x400000


#define Page_idx_to_Addr(idx) (idx*_PAGE_SIZE)

// typedef enum {
//     OS_PAGE_FLAGS_ALLOCATED     = 1 << 0,
//     OS_PAGE_FLAGS_UNALLOCATABLE = 1 << 1,
//     OS_PAGE_FLAGS_UNUSED        = 1 << 2,
// } OS_PAGE_FLAG;

typedef struct {
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t reserved   : 1;
    uint32_t page_size  : 1;
    uint32_t ignored    : 1;
    uint32_t available  : 3;
    uint32_t addr       : 20;
} __attribute__((packed)) PDE;

typedef struct {
    uint32_t present      : 1;
    uint32_t rw           : 1;
    uint32_t user         : 1;
    uint32_t pwt          : 1;
    uint32_t pcd          : 1;
    uint32_t accessed     : 1;
    uint32_t dirty        : 1;
    uint32_t pat          : 1;
    uint32_t global       : 1;
    uint32_t os_unused1   : 1;
    uint32_t os_unused2   : 1;
    uint32_t os_unused3   : 1;
    uint32_t addr         : 20;
} __attribute__((packed)) PTE;

typedef struct {
    PDE* pde_arr;
} __attribute__((packed)) PD_t ;

typedef struct {
    uint32_t size;
    page_index vaddr;
    PTE pte_bits;
} __attribute__((packed)) Page_Group ;


extern PD_t _k_pd;


int setup_paging();
void k_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present, uint8_t rw, uint8_t user);
void pd_map_page(PD_t* pd, uint32_t virtual_addr, uint32_t physical_addr, uint8_t present,uint8_t rw, uint8_t user);
PTE* get_pte(uint32_t index);
PTE* get_pte_for_pa(uint32_t pa);

// allocation
page_index page_alloc_nomap(size_t amount);

page_index page_alloc(size_t amount, int read_write, int user_supervisor);
void page_free(page_index pa, size_t amount);


void page_force_alloc(page_index pa, size_t amount);
int is_page_allocated(page_index pa);

// PD
uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, PD_t* out_pd_t );
void pd_free(PD_t* pd);

int v_map(PD_t* page_dir, Page_Group* groups, uint32_t group_count);
uintptr_t PD_append_pages(PD_t* page_dir, PTE* ptes, uint32_t pte_count);

page_index k_append_pages(page_index phys_start_page,uint32_t amount,uint8_t rw,uint8_t us);

int unmap_page(PD_t* target_pd,uint32_t virtual_addr);


//Misc
void reserve_kernel_pages();

//debug
void dump_pd();

uint32_t get_used_ram();

#endif
