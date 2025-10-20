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

#include "data/globals.h"
#include "data/textconsts.h"

extern int vgaX, vgaY;

// extern void test_16func();
KernelData_t kernel_data;


void kmain(unsigned long magic, unsigned long mb_struct_addr) {

    

    serial_init();
    
    
    Sys_log("interrupts disabled.\n");
    Sys_log("Kernel starting...\n");
    Sys_log("Kernel compiled on %s at %s\n", __DATE__, __TIME__);
    Sys_log("with GCC ver %d.%d.%d \n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    
    TASK_SWITCHING_FLAG = 0;

    Sys_log("copying multiboot info struct...\n");
    memcpy(Get_multiboot_info(), (void*)mb_struct_addr, sizeof(multiboot_info_t));  
    
    initGdt();

    Sys_log("Setting upIDT...\n");
    idt_init();
    Sys_log("GDT and IDT set up successfully.\n");

    

    Sys_log("remapping PIC...\n");
    pic_remap();
    Sys_log("PIC remapped successfully.\n");

    Sys_log("Initializing PIT...\n");
    pit_init(); 
    Sys_log("PIT initialized.\n");
    
    
    disable_mouse_display();
    // enable_cursor(0, 2);

    *((uint32_t*)MULTIBOOT_INFO_ADDRESS) = mb_struct_addr;
    
    
    
    Sys_log("Memory map parsed.\n");
    
    Sys_log("Setting up paging...\n");
    
    if (setup_paging() != 0  ) {
        
        Sys_log("Paging setup failed, halting.");
        move_cursor(0, 0);
        printstr("Paging setup failed, halting.");
        while (1) __asm__ volatile ("hlt");
    }
    
    
    Sys_log("Paging set up successfully.\n");
    
    
    
    
    Sys_log("Parsing memory map...\n");
    parse_memory_map( Get_multiboot_info() );
    
    Sys_log("Initialising graphics.\n");
    init_graphics();
    
    
    force_alloc((uint32_t)Get_multiboot_info(), sizeof(multiboot_info_t));

    int pitch = Multiboot_info->framebuffer_bpp;
    
    
    
    
    ClearScreen();

    
    
    force_alloc(0x0, 65535);// reserve low memory for real mode bios calls/or whatever
    force_alloc(KERNEL_DATA_START, KERNEL_DATA_END - KERNEL_DATA_START);
    
    //-new_install
    const char* cmdline = (const char*)Get_multiboot_info()->cmdline;
    Sys_log("kernel called with: %s\n", cmdline);
    // refer tocommented code #1 at the bottom of this file

    int mount_counter = 0;
    
mounting:
Sys_log("trying to mount filesystem...\n");
int res = FS_Mount_Main_Partition(FatFsSys);

    if (res != 0) {
        Sys_log("Failed to mount filesystem. Error code: %d\n Trying to mount again", res);
        mount_counter++;
        if(mount_counter < 3)goto mounting;

        move_cursor(0, 0);
        printf("No Os partition found. if this problem persists after a restart, you may want to reinstall the OS\n (continuing to the console in 10s)");
        sleep(10000);
        goto end_mounting;
    } else {
        Sys_log("Filesystem mounted successfully.\n");
        // get_string();
    }
end_mounting:
    
    Sys_log("Multiboot magic number: 0x%x\n", (void*)magic);
    Sys_log("Multiboot info address: 0x%x\n", mb_struct_addr);
    
    
    move_cursor(0, 0);

    Sys_log("Interrupts reenabled.\n");
    __asm__ volatile ("sti"); // Enable interrupts
    
    
    
    // draw_bitmap_char('T',100,100,8,16,0xFFFFA500,NULL,true,false,false);

    
    for (size_t i = 0; i < 768; i++)
    {
        for (size_t j = 0; j < 1024; j++)
        {
            put_pixel(j,i,0xFFFFA500);
        }
        
    }
    draw_bitmap_string("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/",120,100,8,16,0xFF0000FF,font8x16,false,true,3);

    // ((uint32_t*)Multiboot_info->framebuffer_addr)[1]= 0XFFFFFFFF;
    
    Sys_log("Loading login manager...\n");

    LoadElf("0:/login.rel");
    


    Sys_log("Starting console...\n");
        
    Start_Console();

    while (1) {
        __asm__ volatile ("hlt");
    }
}

// entry point
__attribute__((naked)) void _start() {
    __asm__ volatile (
        "push %ebx\n"       // push multiboot_info pointer (2nd arg)
        "push %eax\n"       // push magic (1st arg)
        "call kmain\n"
        "add $8, %esp\n"    // clean up stack
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