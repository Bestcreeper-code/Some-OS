#include "FileSystem.h"
#include "arch_paging.h"
#include "bootloader.h"
#include "console.h"
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

    // logger_thread_init();
    
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


    
    // tree(root_dentry, 0);
    
    
    // task_switching_flag = 1;
    disable_scheduler();
    // // Sys_Step_Point();
    
    
    extern void testing();
    // Sys_log_Pos("starting process that spams logs with pid 0x%x\n",ktask_start(testing, "test"));
    

    
    
    ////////////////// ktask_start(Start_Console, "kconsole");

    // int crash();
    enable_scheduler();

    // crash();
    // Sys_Breakpoint();
    // Add_Console_Request("exec initrd/hello.elf");
    Start_Console();
    late_init();
    
    while (1) { 
        __asm__ volatile ("hlt");
    }
}

int temp_stalling(){
    for(;;);
}
REGISTER_DRIVER_LATE(slepping, temp_stalling);