#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MIN_OS_PAGES 8192
#define _PAGE_SIZE 4096

#define _PAGETABLE_MAPPED_SIZE 0x400000

typedef uintptr_t page_addr_t;

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
    page_addr_t addr;
    PTE pte_bits;
} __attribute__((packed)) Page_Group ;


int setup_paging();
void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present, uint8_t rw, uint8_t user);
PTE* get_pte(uint32_t index);
PTE* get_pte_for_pa(uint32_t pa);

// allocation
page_addr_t page_alloc(size_t amount, int read_write, int user_supervisor);
void page_free(page_addr_t pa, size_t amount);

page_addr_t pagealloc(size_t amount);
void pagefree(page_addr_t pa, size_t amount);

void page_force_alloc(page_addr_t pa, size_t amount);
int is_page_allocated(page_addr_t pa);

// PD
uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, PD_t* out_pd_t );
void pd_free(PD_t* pd);

int v_map(PD_t* page_dir, Page_Group* groups, uint32_t group_count);
uintptr_t PD_append_pages(PD_t* page_dir, PTE* ptes, uint32_t pte_count);


//Misc
void reserve_kernel_pages();

//debug
void dump_pd();

#endif
