#pragma once

#include "paging.h"   

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>



#define MIN_OS_PAGES 2048

#define _PT_SIZE        (_PAGE_SIZE / sizeof(PTE))

#define _MAX_PT_AMOUNT  1024

#define KERNEL_PDE_COUNT 128
#define KERNEL_PDE_OFFSET 0

#define _PAGETABLE_MAPPED_SIZE 0x400000

#define KERNEL_VMA      0xC0000000
#define HHDM_TO_PHYS(v) ((uintptr_t)(v) - KERNEL_VMA)
#define HHDM_TO_VIRT(p) ((uintptr_t)(p) + KERNEL_VMA)


#define KVSPACE_PAGES      (128 * 1024)
#define KVSPACE_FIRST_PAGE (KERNEL_VMA >> 12)   // 0xC0000
#define KVSPACE_LAST_PAGE  (KVSPACE_FIRST_PAGE + KVSPACE_PAGES)


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
    volatile PDE* pde_arr;
} __attribute__((packed)) PD_t;

typedef struct {
    uint32_t   size;
    page_index vaddr;
    PTE        pte_bits;
} __attribute__((packed)) Page_Group;


extern volatile PD_t _k_pd;
extern volatile uintptr_t _k_pd_phys;




void k_map_page(uint32_t virtual_addr, uint32_t physical_addr, uint8_t present, uint8_t rw, uint8_t user);
void pd_map_page(PD_t* pd, uint32_t virtual_addr, uint32_t physical_addr, uint8_t present, uint8_t rw, uint8_t user);
PTE* get_pte(uint32_t index);
PTE* get_pte_for_pa(uint32_t pa);


page_index page_alloc_nomap(size_t amount);


void page_force_alloc(page_index pa, size_t amount);
int  is_page_allocated(page_index pa);


uintptr_t  new_page_dir(Page_Group* groups, uint32_t group_count, volatile PD_t* out_pd_t);
void       pd_free(PD_t* pd);

int        v_map(PD_t* page_dir, Page_Group* groups, uint32_t group_count);
uintptr_t  PD_append_pages(PD_t* page_dir, PTE* ptes, uint32_t pte_count);

page_index k_append_pages(page_index phys_start_page, uint32_t amount, uint8_t rw, uint8_t us);


int pd_unmap_page(PD_t* target_pd, uint32_t virtual_addr);


void reserve_kernel_pages();


void dump_pd(PD_t* pd);

void pd_init(PD_t* pd);


typedef struct {
    page_index phys_base;
    size_t    size;
} early_reservation_t;
 
#define EARLY_RESERVATION_MAX  (PAGE_SIZE / sizeof(early_reservation_t))
 
int page_reserve_page_early(page_index phys_base, size_t size);

const early_reservation_t* page_early_reservations(int *out_count);
void page_reclaim_early_reservation_table();
static void _bitmap_mark_range_used(uintptr_t phys_base, size_t size);