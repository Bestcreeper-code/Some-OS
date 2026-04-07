#include "arch.h"
#include "Logger.h"
#include "err_codes.h"
#include "idt.h"
#include "gdt.h"
#include "kernel_data.h"
#include "multiboot_info.h"
#include "paging.h"
#include "time.h"

int arch_init(){
    init_gdt();
    idt_init();

    int pg_res = setup_paging();
    if (pg_res != 0  ) {
        
        Sys_Error("Paging setup failed, halting.\n");
        char buf[128];
        if(pg_res ==  -E_NOMEM) Sys_Error("Not enough memory to init correctly [%x MiB]", (Multiboot_info->mem_upper + Multiboot_info->mem_lower)/1024);
        while (1) __asm__ volatile ("hlt");
    }
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, true);
    Sys_Success("Paging set up successfully.\n");
    
    pic_remap();
    
    return 0;
    
}