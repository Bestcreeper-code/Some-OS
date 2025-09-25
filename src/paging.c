#include "headers/paging.h"
#include "headers/memory.h"
#include "headers/multiboot_info.h"

int setup_paging() {
    uint32_t pages_amount = (Get_multiboot_info()->mem_upper + 1024) / 4; // pages in KB / 4 = 4KB pages
    if (pages_amount < MIN_OS_PAGES * 1.5) {
        Sys_log("Not enough memory for paging setup: have %d pages, need %d", pages_amount, MIN_OS_PAGES + (MIN_OS_PAGES / 2));
        return -1;
    }

    if (pages_amount > 1024 * 1024) {
        Sys_log("Computer has more than 4GB of RAM, capping to 4GB.");
        pages_amount = 1024 * 1024;
    }

    Sys_log("Setting up paging with %d 4KB pages", pages_amount);
    
    uint32_t pde_count = (pages_amount + 1023) / 1024;

    // Page directory at fixed address
    PDE* page_directory = (PDE*)PD_PHYS_ADDR;

    // Page tables start right after the directory
    PTE* page_tables = (PTE*)(PD_PHYS_ADDR + 0x1000); // Each page table is 4KB

    for (uint32_t i = 0; i < 1024; i++) {
        if (i < pde_count) {
            page_directory[i].present = 1;
            page_directory[i].rw = 1;
            page_directory[i].user = 0;
            page_directory[i].pwt = 0;
            page_directory[i].pcd = 0;
            page_directory[i].accessed = 0;
            page_directory[i].reserved = 0;
            page_directory[i].page_size = 0; // 4KB pages
            page_directory[i].ignored = 0;
            page_directory[i].available = 0;
            page_directory[i].addr = ((uint32_t)&page_tables[i * 1024]) >> 12;
            // Sys_log("setting table %d at %x\n",i,&page_tables[i * 1024]);
        } else {
            *((uint32_t*)&page_directory[i]) = 0; // Clear unused PDEs
        }
    }

    for (uint32_t i = 0; i < pages_amount; i++) {
        PTE* pte = &page_tables[i];

        pte->present = 1;
        pte->rw = 1;
        pte->user = (i >= MIN_OS_PAGES) ? 1 : 0;
        pte->pwt = 0;
        pte->pcd = 0;
        pte->accessed = 0;
        pte->dirty = 0;
        pte->pat = 0;
        pte->global = 0;
        pte->available = 0;
        pte->addr = i; // Identity map
        // Sys_log("setting page %d\n",i);
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
