#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/multiboot_info.h"

int setup_paging() {
    uint32_t pages_amount = (Get_multiboot_info()->mem_upper + 1024) / 4;
    if (pages_amount < MIN_OS_PAGES*1.5) {
        Sys_log("Not enough memory for paging setup: have %d pages, need %d", pages_amount, MIN_OS_PAGES + (MIN_OS_PAGES / 2));
        return -1; // Not enough memory
    }

    if (pages_amount > 1024*1024) {
        Sys_log("Compuer has more than 4GB of RAM, capping to 4GB.");
        pages_amount = 1024*1024; 
    }

    Sys_log("Setting up paging with %d 4KB pages", pages_amount);
    uint32_t pde_count = (pages_amount + 1023) / 1024;
    PDE* page_directory = (PDE*)PD_PHYS_ADDR;//array of 1024 PDEs
    for (uint32_t i = 0; i < 1024; i++) {
        page_directory[i].present = i < pde_count ? 1 : 0;
        page_directory[i].rw = 1; // writable
        page_directory[i].user = 0; // supervisor level
        page_directory[i].pwt = 0;
        page_directory[i].pcd = 0;
        page_directory[i].accessed = 0;
        page_directory[i].reserved = 0;
        page_directory[i].page_size = 0; // 0 for 4KB pages
        page_directory[i].ignored = 0;
        page_directory[i].available = 0;
        page_directory[i].addr = ((PD_PHYS_ADDR + 0x1000) + (i * 0x1000)) >> 12;
    }
    PTE* pages_array = (PTE*)(PD_PHYS_ADDR + 0x1000); //array of 1024*pages amount PTEs

    for (uint32_t i = 0; i < pages_amount ; i++) {
        pages_array[i].present = 1;
        pages_array[i].rw = 1; // writable
        pages_array[i].user = (i >= MIN_OS_PAGES) ? 1 : 0; // supervisor level
        pages_array[i].pwt = 0;
        pages_array[i].pcd = 0;
        pages_array[i].accessed = 0;
        pages_array[i].dirty = 0;
        pages_array[i].pat = 0;
        pages_array[i].global = 0;
        pages_array[i].available = 0;
        pages_array[i].addr = i; // identity mapping
    }

    asm volatile (
        "mov %0, %%cr3 \n\t"
        "mov %%cr0, %%eax \n\t"
        "or $0x80000000, %%eax \n\t"
        "mov %%eax, %%cr0"
        : 
        : "r"(PD_PHYS_ADDR)
        : "eax"
    );

    return 0;
}
//
// #define PD_PHYS_ADDR 0x100000
// #define MIN_OS_PAGES 8192

//