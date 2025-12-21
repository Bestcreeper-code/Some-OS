#include <stdint.h>
#include <stdbool.h>
#include "headers/console.h"
#include "headers/ATA_IO.h"
#include "headers/io.h"
#include "headers/multiboot_info.h"
#include "headers/FileSystem.h"
#include "headers/memory.h"
#include "headers/idt.h"
#include "headers/time.h"
#include "headers/Logger.h"
#include "headers/gdt.h"
#include "../FatFs/ff.h"
#include "headers/asm.h"
#include "headers/video.h"
#include "headers/vga_modes.h"
#include "headers/mouse.h"
// #include "headers/usb.h"
#include "headers/bios.h"
#include "headers/elf.h"
#include "headers/paging.h"
#include "headers/scheduler.h"
#include "headers/symbols.h"

#include "config/config.h"
#include "data/textconsts.h"





#include "../output.h"
#include "kernel_data.h"





extern int vgaX, vgaY;

// extern void test_16func();
KernelData_t kernel_data;
KernelData_t* kernel_data_ptr;

extern const uint8_t _binary_syms_bin_start[];
extern const uint8_t _binary_syms_bin_end[];

static uintptr_t mb_struct_ptr;

void kmain(unsigned int magic, unsigned long mb_struct_addr) {
    mb_struct_ptr = mb_struct_addr;
    kernel_data_ptr = &kernel_data;
    
    Set_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON, false);
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, false);
    task_switching_flag = 0;

    Setup_Kernel_Syms();

    serial_init();


    
    Sys_log("interrupts disabled.\n");
    Sys_log("Kernel starting...\n");
    Sys_log("Kernel compiled on %s at %s\n", __DATE__, __TIME__);
    Sys_log("with GCC ver %d.%d.%d \n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    
    Sys_log("Multiboot magic number: 0x%x\n", magic);
    Sys_log("Multiboot info address: 0x%x\n", mb_struct_ptr);

    Sys_log("copying multiboot info struct...\n");
    memcpy(Get_multiboot_info(), (void*)mb_struct_ptr, sizeof(multiboot_info_t));  
    

    const char* cmdline = (const char*)Get_multiboot_info()->cmdline;
    Sys_log("kernel called with: %s\n", cmdline);

    
    init_desc_tables();

    
    idt_init();
    

    

    
    pic_remap();
    

    
    
    disable_mouse_display();
    // enable_cursor(0, 2);

    
    
    
    
    
    

    
    if (setup_paging() != 0  ) {
        
        Sys_Error("Paging setup failed, halting.");
        
        while (1) __asm__ volatile ("hlt");
    }
    Set_Kernel_Flag(KDATA_FLAG_PAGING_ON, true);
    Sys_Success("Paging set up successfully.\n");
    
    Sys_log("Setting up Kernel Stack.\n");
    page_index allocated_stack_pages = page_alloc(KERNEL_STACK_PAGE_AMOUNT, 1, 0);
    uintptr_t allocated_stack_top = allocated_stack_pages + (KERNEL_STACK_PAGE_AMOUNT * _PAGE_SIZE);
    __asm__ volatile(
        "movl %0, %%esp\n"
        :
        : "r"(allocated_stack_top)
    );
    init_tss(allocated_stack_top);
    
    
    
    
    // Sys_log("test %x\n",((multiboot_info_t*)mb_struct_ptr)->flags);for(;;);
    parse_memory_map((multiboot_info_t*)mb_struct_ptr);
    
    
    pit_init(); 
    
    
    
    init_graphics();
    
    Set_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON, true);
    
    
    
    
    
    
    int pitch = Multiboot_info->framebuffer_bpp;
    
    
    
    
    // ClearScreen();

    
    
    
    force_alloc(0x0, 65535);//stop kernel from allocating low mem as it can crash
    
    //-new_install
    
    
    // refer tocommented code #1 at the bottom of this file

    
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
    
    
    
    
    // move_cursor(0, 0);

    
    
    
    
    // // draw_bitmap_char('T',100,100,8,16,0xFFFFA500,NULL,true,false,false);
    // uint32_t* data = (uint32_t*)image_data;
    // //255²
    // for (size_t i = 0; i < 56; i++) {
    //     for (size_t j = 0; j < 56; j++) {
    //         uint32_t pixel = data[i * 56 + j];
    //         // Swap red and blue
    //         pixel = (pixel & 0xFF00FF00) | ((pixel & 0x00FF0000) >> 16) | ((pixel & 0x000000FF) << 16);

    //         // Draw 3x3 block for each pixel
    //         for (size_t dy = 0; dy < 7; dy++) {
    //             for (size_t dx = 0; dx < 7; dx++) {
    //                 put_pixel(j * 7 + dx, i * 7 + dy + 200, pixel);
    //             }
    //         }
    //     }
    // }

    // draw_bitmap_string("CREEPER OS",0,0,8,16,0x0000FF7F,font8x16,false,true,3);
    
    
    __asm__ volatile ("sti"); // Enable interrupts
    
    Sys_log("Interrupts reenabled.\n");
    
    
    // scheduler_init();
    
    Sys_log("Loading login manager...\n");
    exec_ELF("0:/loop.elf");//DEBUG sched just crashes when there is more than 1 process
    // task_switching_flag = 1;
    
    sleep(1000);
    
    Start_Console();

    while (1) {
        __asm__ volatile ("hlt");
    }
}



// entry point
__attribute__((naked)) void _start() {
    __asm__ volatile (
        
        "push %ebx\n"       // push multiboot_info pointer
        "push %eax\n"       // push magic
        "call kmain\n"
        "add $8, %esp\n"    
        "cli\n"
        "hlt\n"
        "jmp .\n"
    );
}


// readelf --relocs build_execs/gametest.o > relocations.txt



/*
    removed code #1:(may be used later)

    __asm__ volatile ("sti");//start ints just for using the keyboard
    printf("[Creeper OS Kernel]\n");
    printf("overwrite disk and install OS? (yes/no):\n");
    if(cmdline && !strcmp(cmdline, "-new_install") && !strcmp(Console_Get_Command(),"yes")){
        __asm__ volatile ("cli");

        Sys_log("New install flag detected, starting disk installer...\n");
        multiboot_module_t* os_image_file;

        os_image_file = Multiboot_Get_loaded_module(Get_multiboot_info(), "os.iso");
        

        if(!os_image_file){
            Sys_log("Bootloader or kernel module not found, halting.");
            return;
        }
        
        // int res = Install_OS_to_disk(os_image_file);
        if(res != 0){
            printf("Disk installer failed with code %d, halting.", res);
            Sys_log("Disk installer failed with code %d, halting.", res);
            while(1)__asm__ volatile ("hlt");
        }else{
            Sys_log("Disk installer finished successfully.\n");
            printf("OS installed successfully!\nYou may now remove the installer media and reboot.\n");
            while(1)__asm__ volatile ("hlt");
        }
    }
*/