#ifndef PAGING_H
#define PAGING_H

#include <stdint.h>
#include <stddef.h>

#define MIN_OS_PAGES 8192
#define _PAGE_SIZE 4096

#define _PAGETABLE_MAPPED_SIZE 0x400000


typedef enum {
    OS_PAGE_FLAGS_ALLOCATED     = 1 << 0,
    OS_PAGE_FLAGS_UNALLOCATABLE = 1 << 1,
    OS_PAGE_FLAGS_UNUSED        = 1 << 2,
} OS_PAGE_FLAG;

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
    uint32_t present    : 1;
    uint32_t rw         : 1;
    uint32_t user       : 1;
    uint32_t pwt        : 1;
    uint32_t pcd        : 1;
    uint32_t accessed   : 1;
    uint32_t dirty      : 1;
    uint32_t pat        : 1;
    uint32_t global     : 1;
    uint32_t os_allocated     : 1;
    uint32_t os_unallocatable : 1;
    uint32_t os_unused        : 1;
    uint32_t addr       : 20;
} __attribute__((packed)) PTE;

typedef struct {
    PDE* pde_arr;
    PTE** pte_ptrs;
} PD_t;

typedef struct {
    uint32_t size;
    uintptr_t addr;
    PTE pte_bits;
} Page_Group;



int setup_paging();
void map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present, uint8_t rw, uint8_t user, uint8_t os_flags);
PTE* get_pte(uint32_t index);
PTE* get_pte_for_pa(uint32_t pa);
uint32_t page_alloc(size_t amount, int read_write, int user_supervisor);
void page_free(uint32_t pa, size_t amount);
void reserve_kernel_pages();

uintptr_t new_page_dir(Page_Group* groups, uint32_t group_count, uint32_t* PD_size);
int v_map(PD_t* page_dir, Page_Group* groups, uint32_t group_count);
uintptr_t PD_append_pages(PD_t* page_dir, PTE* ptes, uint32_t pte_count);

#endif
