#include "arch.h"
#include "Logger.h"
#include "init/arch_gdt.h"
#include "bootloader.h"
#include "cpu/cpu.h"
#include "err_codes.h"
#include "idt.h"

#include "kernel_data.h"

#include "paging.h"
#include "time.h"

int arch_init(){
    Sys_log("Entering arch init");
    register_cpu_features();
    init_gdt();
    idt_init();


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
        "movq %0, %%rsp\n"
        :
        : "r"(allocated_stack_top)
    );
    init_tss(allocated_stack_top);
    
    pic_remap();
    
    return 0;
    
}