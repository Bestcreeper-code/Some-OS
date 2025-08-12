#include <stdint.h>
#include <stdbool.h>
#include "headers/console.h"
#include "headers/ATA_IO.h"
#include "headers/io.h"
#include "headers/multiboot_info.h"
#include "headers/memory.h"
#include "headers/idt.h"
#include "headers/time.h"
#include "headers/gdt.h"
#include "../FatFs/ff.h"
#include "headers/asm.h"
#include "headers/video.h"

#include "data/globals.h"

extern int vgaX, vgaY;


void kmain(unsigned long magic, unsigned long addr) {
    // vga_set_mode_03h();
    initGdt();
    idt_init();
    pic_remap();
    pit_init(); 
    __asm__ volatile ("sti"); // Enable interrupts
    ClearScreen();

    force_alloc(FATFS_SYS_ADDR,sizeof(FATFS));
    force_alloc(KERNEL_STACK_BASE-KERNEL_STACK_SIZE,KERNEL_STACK_SIZE);
    
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
    
    *((uint32_t*)MULTIBOOT_INFO_ADDRESS) = addr;
    parse_memory_map( Get_multiboot_info() );
    
    
    enable_cursor(0, 15);
    move_cursor(0, 0);
    clear_processes();
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