#include "ATA_IO.h"
#include "FileSystem.h"
#include "asm.h"
#include "console.h"
#include "fs.h"
#include "io.h"
#include "memory.h"
#include "multiboot_info.h"

#include <stdbool.h>
#include <stdint.h>
#include "ff.h"
#include "Logger.h"
#include "arch.h"
#include "path.h"
#include "time.h"
#include "types.h"
#include "video.h"
#include "gdt.h"
#include "mouse.h"
#include "paging.h"
#include "scheduler.h"
#include "symbols.h"
#include "config/config.h"
#include "cpu.h"
#include "kernel_data.h"
#include "string.h"
#include "vfs.h"
#include "devfs.h"





extern int vgaX, vgaY;

// extern void test_16func();
KernelData_t kernel_data;
KernelData_t* kernel_data_ptr;

extern const uint8_t _binary_syms_bin_start[];
extern const uint8_t _binary_syms_bin_end[];

multiboot_info_t* mb_struct_ptr;


void kmain(unsigned int magic, unsigned long mb_struct_addr) {
    mb_struct_ptr = (multiboot_info_t*)mb_struct_addr;
    kernel_data_ptr = &kernel_data;
    
    Set_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON, false);
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, false);

    arch_init();

    

    
    serial_init();
    
    
    
    

    
    Sys_log("interrupts disabled.\n");
    Sys_log("Kernel starting...\n");
    
    Sys_log("Multiboot magic number: 0x%x\n", magic);
    Sys_log("Multiboot info address: 0x%x\n", mb_struct_ptr);

    Sys_log("copying multiboot info struct...\n");
    memcpy(Get_multiboot_info(), mb_struct_ptr, sizeof(multiboot_info_t));  
    

    const char* cmdline = (const char*)Get_multiboot_info()->cmdline;
    Sys_log("kernel called with: %s\n", cmdline);

    
    if (setup_paging() != 0  ) {
        
        Sys_Error("Paging setup failed, halting.");
        
        while (1) __asm__ volatile ("hlt");
    }
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, true);
    Sys_Success("Paging set up successfully.\n");
    
    init_graphics();
    Set_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON, true);
    int pitch = Multiboot_info->framebuffer_bpp;
    
    

    
    Setup_Kernel_Syms();
    

    

    
    pic_remap();
    
    
    
    
    // disable_mouse_display();
    // enable_cursor(0, 2);

    
    
    
    
    
    

    
    
    
    Sys_log("Setting up Kernel Stack.\n");
    page_index allocated_stack_pages = page_alloc(KERNEL_STACK_PAGE_AMOUNT, PAGE_FLAG_RW);
    uintptr_t allocated_stack_top = allocated_stack_pages + (KERNEL_STACK_PAGE_AMOUNT * PAGE_SIZE);
    __asm__ volatile(
        "movl %0, %%esp\n"
        :
        : "r"(allocated_stack_top)
    );
    init_tss(allocated_stack_top);
    
    
    multiboot_info_t* temp_ptr = (multiboot_info_t*)PAGE_ADDR(page_alloc(1, PAGE_FLAG_RW));
    memcpy(temp_ptr, mb_struct_ptr, sizeof(multiboot_info_t));
    get_pte((uintptr_t)temp_ptr/1024)->rw=0;
    invlpg((uintptr_t)temp_ptr/1024);
    
    mb_struct_addr = (uintptr_t)temp_ptr;
    
    parse_memory_map(mb_struct_ptr);
    
    
    pit_init(); 
    
    
    //devfs bullshit test
    
    
    // #warning  remove pls
    // ata_init();

    
    
    
    __asm__ volatile ("sti"); // Enable interrupts
    
    
    int mount_counter = 0;
mounting:
    Sys_log("trying to mount filesystem...\n");
    int res = FS_Mount_Main_Partition(FatFsSys);

    if (res != 0) {
        Sys_Error("Failed to mount filesystem. Error code: %d\n Trying to mount again", res);
        mount_counter++;
        if(mount_counter < 3)goto mounting;

        move_cursor(0, 0);
        printf("No Os partition found. if this problem persists after a restart, you may want to reinstall the OS\n (continuing to the console in 10s)");
        sleep(10000);
        goto end_mounting;
    } else {
        Sys_Success("Filesystem mounted successfully.\n"); 
        // get_string();
    }
end_mounting:
    
    


//log kernel/cpu info
    sys_color_serial_logf("Kernel compiled on %s at %s\n",ANSI_BRIGHT_YELLOW,0,"kernel","",0, __DATE__, __TIME__);
    sys_color_serial_logf("with GCC ver %d.%d.%d \n",ANSI_BRIGHT_YELLOW,0,"kernel","",0, __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    
    cpu_log_specs();
    char simplified_nb_1[16]; 
    char simplified_nb_2[16];

    byte_nb_simplify(get_used_ram(), simplified_nb_1);
    byte_nb_simplify(ram_amount, simplified_nb_2);

    sys_color_serial_logf("Ram Used = %s / %s \n",ANSI_BRIGHT_GREEN,0,"","",0, 
        simplified_nb_1, simplified_nb_2);
    
    sys_color_serial_logf("Press Any key to continue\n", ANSI_BRIGHT_MAGENTA, ANSI_BG_BLACK, "", "", 0);
    
    
    devfs_init();
    init_fb_devfs_file();


    tree(root_dentry, 0);
    
    
    

    // scheduler_init();
    
    // task_switching_flag = 1;
    
    Sys_Breakpoint();
    Start_Console();

    while (1) { 
        __asm__ volatile ("hlt");
    }
}






