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
#include "cpu.h"





#include "../output.h"
#include "kernel_data.h"
#include "string.h"





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
    task_switching_flag = 0;

    Setup_Kernel_Syms();

    serial_init();


    
    Sys_log("interrupts disabled.\n");
    Sys_log("Kernel starting...\n");
    
    Sys_log("Multiboot magic number: 0x%x\n", magic);
    Sys_log("Multiboot info address: 0x%x\n", mb_struct_ptr);

    Sys_log("copying multiboot info struct...\n");
    memcpy(Get_multiboot_info(), mb_struct_ptr, sizeof(multiboot_info_t));  
    

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
    
    
    multiboot_info_t* temp_ptr = (multiboot_info_t*)Page_idx_to_Addr(page_alloc(1, 1, 0));
    memcpy(temp_ptr, mb_struct_ptr, sizeof(multiboot_info_t));
    get_pte((uintptr_t)temp_ptr/1024)->rw=0;
    invlpg((uintptr_t)temp_ptr/1024);
    
    mb_struct_addr = (uintptr_t)temp_ptr;
    
    parse_memory_map(mb_struct_ptr);
    
    
    pit_init(); 
    
    
    
    init_graphics();
    
    Set_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON, true);
    int pitch = Multiboot_info->framebuffer_bpp;
    
    ClearScreen();

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
    
    
    
    getc();

    






    
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
    
    
    
    
    
    scheduler_init();
    
    Sys_log("Loading login manager...\n");
    exec_ELF("0:/loop.elf");//DEBUG sched just crashes when there is more than 1 process
    
    task_switching_flag = 1;
    
    ClearScreen();
    enable_mouse_display();
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


