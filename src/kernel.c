#include "FileSystem.h"
#include "arch_paging.h"
#include "bootloader.h"
#include "drivers/drivers.h"
#include "elf--later/elf.h"
#include "ff.h"
#include "fs.h"
#include "memory.h"


#include <stdbool.h>
#include <stdint.h>
#include "Logger.h"
#include "arch.h"
#include "paging.h"
#include "scheduler.h"
#include "symbols.h"
#include "cpu.h"
#include "kernel_data.h"
#include "string.h"
#include "vfs.h"
#include "sysfs.h"
#include "drivers.h"





extern int vgaX, vgaY;

// extern void test_16func();
KernelData_t kernel_data;
KernelData_t* kernel_data_ptr;

extern const uint8_t _binary_syms_bin_start[];
extern const uint8_t _binary_syms_bin_end[];




void kmain() {
    
    kernel_data_ptr = &kernel_data;
    
    Set_Kernel_Flag(KDATA_FLAG_FRAMEBUFFER_ON, false);
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, false);

    arch_init();

    serial_init();
    


    
    
    
    
    

    Setup_Kernel_Syms();
    
       
    
    core_init();

    
    __asm__ volatile ("sti");

    dev_init();
        
    
//log kernel/cpu info
    Sys_log_NoPos("kernel called with: %s\n", get_bootloader_generic_info()->cmdline);
    Sys_log_NoPos("=====================================================\n");
    Sys_color_log_NoPos("Kernel compiled on %s at %s\n",ANSI_BRIGHT_YELLOW,0, __DATE__, __TIME__);
    Sys_color_log_NoPos("with GCC ver %d.%d.%d \n",ANSI_BRIGHT_YELLOW,0, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    Sys_log_NoPos("\n");
    Sys_color_log_NoPos("Booted by %s via %s\n",ANSI_BRIGHT_YELLOW,0, 
        get_bootloader_generic_info()->bootloader_name, 
        get_bootloader_generic_info()->boot_protocol);
    
    
    cpu_log_specs();
    char simplified_nb_1[16]; 
    char simplified_nb_2[16];

    byte_nb_simplify(get_used_ram(), simplified_nb_1,1);
    byte_nb_simplify(ram_amount, simplified_nb_2,1);

    Sys_color_log_NoPos("Ram Used = %s / %s \n",ANSI_BRIGHT_GREEN,0,simplified_nb_1, simplified_nb_2);
    Sys_log_NoPos("=====================================================\n");
    
    Sys_color_log_NoPos("Press Any key to continue\n", ANSI_BRIGHT_MAGENTA, ANSI_BG_BLACK);
    
    
    sysfs_init();
    
    








    fs_init();


    
    tree(root_dentry, 0);
    
    
    // task_switching_flag = 1;
    disable_scheduler();
    // // Sys_Step_Point();
    extern void testing();
    Sys_log_Pos("-------0x%x\n",ktask_start(testing, "test"));
    pid_t spawn_test_task();
    disable_scheduler();
    Sys_log_Pos("-------0x%x\n",spawn_test_task());
    


    // Sys_log_Pos("-------0x%x\n",ktask_start(testing, "test"));
    // Sys_log_Pos("-------0x%x\n",ktask_start(testing, "test"));

    enable_scheduler();
    late_init();
    
    while (1) { 
        __asm__ volatile ("hlt");
    }
}






extern char user_test_entry;
extern char user_test_end;

pid_t spawn_test_task() {
    PD_t pd;
    pd.pde_arr = (PDE*)PAGE_ADDR( page_alloc(1, PAGE_FLAG_RW));
    
    pd_init(&pd);
    
    uintptr_t vaddr = 0x400000;

    uintptr_t phys = page_alloc(1, PAGE_FLAG_RW | PAGE_FLAG_USER);
    if (!phys) return -1;
    
    uintptr_t paddr = phys << 12;

    // map user page
    pd_map_page(&pd, vaddr, phys, 1, 1, 1);

    // copy ASM blob instead of C function
    size_t size = (uintptr_t)&user_test_end - (uintptr_t)&user_test_entry;
    memcpy((void*)paddr,
           (void*)&user_test_entry,
           size);

    // start at ASM entry point
    return us_task_start((void*)vaddr, "test_uspace_task", pd);
}