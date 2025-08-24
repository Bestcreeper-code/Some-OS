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

#include "data/globals.h"

extern int vgaX, vgaY;


void kmain(unsigned long magic, unsigned long addr) {
    Kern_log("interrupts disabled.\n");
    serial_init();
    Kern_log("Kernel starting...\n");
    Kern_log("Kernel compiled on %s at %s\n", __DATE__, __TIME__);
    Kern_log("with GCC ver %d.%d.%d \n", __GNUC__, __GNUC_MINOR__, __GNUC_PATCHLEVEL__);
    
    *((char*)TASK_SWITCHING_FLAG) = 0;
    // vga_set_mode_03h();
    Kern_log("Setting up GDT and IDT...\n");
    initGdt();
    idt_init();
    Kern_log("GDT and IDT set up successfully.\n");

    Kern_log("remapping PIC...\n");
    pic_remap();
    Kern_log("PIC remapped successfully.\n");

    Kern_log("Initializing PIT...\n");
    pit_init(); 
    Kern_log("PIT initialized.\n");
    
    
    disable_mouse_display();

    Kern_log("Parsing memory map...\n");
    *((uint32_t*)MULTIBOOT_INFO_ADDRESS) = addr;
    parse_memory_map( Get_multiboot_info() );
    Kern_log("Memory map parsed.\n");

    vga_set_mode(0X03);
    
    
    ClearScreen();
    
    // force_alloc(KERNEL_STACK_BASE-KERNEL_STACK_SIZE ,KERNEL_STACK_SIZE);
    force_alloc(KERNEL_DATA_START, KERNEL_DATA_END - KERNEL_DATA_START);
    
mounting:
    Kern_log("trying to mount filesystem...\n");
    FRESULT res = f_mount(FatFsSys, "0:", 1);
    if (res != FR_OK) {
        Kern_log("Failed to mount filesystem. Error code: %d\n Trying to mount again", res);
        goto mounting;
    } else {
        Kern_log("Filesystem mounted successfully.\n");
        // get_string();
    }
    
    
    Kern_log("Multiboot magic number: 0x%x\n", (void*)magic);
    Kern_log("Multiboot info address: 0x%x\n", addr);
    
    
    // clear_processes();
    // new_process("0:/filemger.bin");
    // sleep(2000);
    
    enable_cursor(0, 15);
    move_cursor(0, 0);
    // *((char*)TASK_SWITCHING_FLAG) = 1;
    // Load_bin_exe("0:/console.bin");

    __asm__ volatile ("sti"); // Enable interrupts
    Kern_log("Interrupts reenabled.\n");

    Kern_log("Loading login manager...\n");
    Load_bin_exe("0:/SYSTEM_CORE/Security/login.bin", 0, NULL);

    Kern_log("Starting console...\n");
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