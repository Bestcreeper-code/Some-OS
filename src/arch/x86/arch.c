#include "arch.h"
#include "Logger.h"
#include "arch_gdt.h"
#include "arch_paging.h"
#include "asm.h"
#include "bootloader.h"
#include "cpu.h"
#include "err_codes.h"
#include "idt.h"

#include "kernel_data.h"

#include "paging.h"
#include "time.h"
#include "string.h"

int arch_init(){
    Sys_log("Entering arch init");
    init_gdt();
    idt_init();

    register_cpu_features();

    int pg_res = setup_paging();
    if (pg_res != 0  ) {
        
        Sys_Error("Paging setup failed, halting.\n");
        char buf[128];
        if(pg_res ==  -E_NOMEM) Sys_Error("Not enough memory to init correctly [%x MiB]", (get_bootloader_mem_info()->mem_upper + get_bootloader_mem_info()->mem_lower)/1024);
        while (1) __asm__ volatile ("hlt");
    }
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, true);
    Sys_Success("Paging set up successfully.\n");
    
    Sys_log("Setting up Kernel Stack.\n");
    page_index allocated_stack_pages = page_alloc(KERNEL_STACK_PAGE_AMOUNT, PAGE_FLAG_RW);
    uintptr_t allocated_stack_top = (PAGE_ADDR(allocated_stack_pages) + (KERNEL_STACK_PAGE_AMOUNT * PAGE_SIZE));

    if (!allocated_stack_pages) {
        Sys_Error("Couldn't allocate kernel stack :(");
        while (1) __asm__ volatile ("hlt");
    }

    __asm__ volatile(
        "movl %0, %%esp\n"
        :
        : "r"(allocated_stack_top)
    );
    init_tss(allocated_stack_top);
    
    
    // multiboot_info_t* temp_ptr = (multiboot_info_t*)PAGE_ADDR(page_alloc(1, PAGE_FLAG_RW));
    // memcpy(temp_ptr, Multiboot_info, sizeof(multiboot_info_t));
    // get_pte((uintptr_t)temp_ptr/1024)->rw=0;
    // invlpg((uintptr_t)temp_ptr/1024);
    
    // Multiboot_info = (multiboot_info_t*)temp_ptr;
    
    pic_remap();
    
    return 0;
    
}