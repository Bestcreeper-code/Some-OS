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
#include "headers/gdt.h"
#include "../FatFs/ff.h"
#include "headers/asm.h"
#include "headers/video.h"
#include "headers/vga_modes.h"
#include "headers/mouse.h"

#include "data/globals.h"

extern int vgaX, vgaY;


void kmain(unsigned long magic, unsigned long addr) {

    *((char*)TASK_SWITCHING_FLAG) = 0;
    // vga_set_mode_03h();
    initGdt();
    idt_init();
    pic_remap();
    pit_init(); 
    disable_mouse_display();
    
    *((uint32_t*)MULTIBOOT_INFO_ADDRESS) = addr;
    parse_memory_map( Get_multiboot_info() );
    
    vga_set_mode(0X03);
    
    
    ClearScreen();
    
    // force_alloc(KERNEL_STACK_BASE-KERNEL_STACK_SIZE ,KERNEL_STACK_SIZE);
    force_alloc(KERNEL_DATA_START, KERNEL_DATA_END - KERNEL_DATA_START);
    
mounting:
    FRESULT res = f_mount(FatFsSys, "0:", 1);
    if (res != FR_OK) {
        printf("Failed to mount filesystem. Error code: %d\n Trying to mount again", res);
        goto mounting;
    } else {
        printf("Filesystem mounted successfully.\n");
        // get_string();
    }
    
    
    printf("Magic number: 0x%x\n", (void*)magic);
    printf("Multiboot info address: 0x%p\n", (void*)addr);
    
    
    // clear_processes();
    // new_process("0:/filemger.bin");
    // sleep(2000);
    
    enable_cursor(0, 15);
    move_cursor(0, 0);
    // *((char*)TASK_SWITCHING_FLAG) = 1;
    // Load_bin_exe("0:/console.bin");

    __asm__ volatile ("sti"); // Enable interrupts

    Load_bin_exe("0:/SYSTEM_CORE/Security/login.bin", 0, NULL);

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