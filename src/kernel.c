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

#include "data/globals.h"

extern int vgaX, vgaY;



void write_crtc_registers(const uint8_t* regs) {
    outb(0x3D4, 0x11);         // Unlock register 0x11
    outb(0x3D5, inb(0x3D5) & 0x7F);

    for (int i = 0; i < 25; ++i) {
        outb(0x3D4, i);
        outb(0x3D5, regs[i]);
    }
}

void set_mode13h_vga() {
    static const uint8_t g_13h_crtc_regs[25] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F,
        0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E,
        0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF, 0x00, 0x00
    };

    // Misc Output Register
    outb(0x3C2, 0x63); // Set proper clock and enable RAM

    // Sequencer
    outb(0x3C4, 0x00); outb(0x3C5, 0x03);
    outb(0x3C4, 0x01); outb(0x3C5, 0x01);
    outb(0x3C4, 0x02); outb(0x3C5, 0x0F);
    outb(0x3C4, 0x03); outb(0x3C5, 0x00);
    outb(0x3C4, 0x04); outb(0x3C5, 0x0E);

    // CRTC Registers (full set)
    write_crtc_registers(g_13h_crtc_regs);

    // Graphics Controller
    outb(0x3CE, 0x00); outb(0x3CF, 0x00);
    outb(0x3CE, 0x01); outb(0x3CF, 0x00);
    outb(0x3CE, 0x02); outb(0x3CF, 0x00);
    outb(0x3CE, 0x03); outb(0x3CF, 0x00);
    outb(0x3CE, 0x04); outb(0x3CF, 0x00);
    outb(0x3CE, 0x05); outb(0x3CF, 0x40); // 256-color mode
    outb(0x3CE, 0x06); outb(0x3CF, 0x05);
    outb(0x3CE, 0x07); outb(0x3CF, 0x0F);
    outb(0x3CE, 0x08); outb(0x3CF, 0xFF);

    // Attribute Controller (setup palette + enable)
    for (uint8_t i = 0; i < 16; i++) {
        inb(0x3DA);                  // Reset flip-flop
        outb(0x3C0, i);              // Index
        outb(0x3C0, i);              // Value (same as index)
    }

    // Attribute Mode Control Register
    inb(0x3DA);
    outb(0x3C0, 0x10); outb(0x3C0, 0x41); // Graphics mode

    // Enable display output
    inb(0x3DA);
    outb(0x3C0, 0x20); // Enable video
}

// --- Drawing example: pixel plot ---
void put_pixel(int x, int y, uint8_t color) {
    volatile uint8_t* fb = (uint8_t*)0xA0000;
    fb[y * 320 + x] = color;
}


void kmain(unsigned long magic, unsigned long addr) {
    initGdt();
    idt_init();
    pic_remap();
    pit_init(); 
    __asm__ volatile ("sti"); // Enable interrupts
    
    ClearScreen();
    set_mode13h_vga();
    sleep(1111);
    memset((uint8_t*)0xA0000,2,320*200);
    for(int i=0;i<50;i++){
        for(int j=0;j<50;j++){
            put_pixel(j,i,4);
        }
    }
    sleep(111111);

    FRESULT res = f_mount(&FatFsSys, "0:", 1);
    if (res != FR_OK) {
        printf("Failed to mount filesystem. Error code: %d\n", res);
    } else {
        printf("Filesystem mounted successfully.\n");
        get_string();
    }

    printf("Magic number: 0x%x\n", (void*)magic);
    printf("Multiboot info address: 0x%p\n", (void*)addr);

    multiboot_info_t* mb_info = (multiboot_info_t*)addr;
    parse_memory_map(mb_info);

    get_string();

    enable_cursor(0, 15);
    move_cursor(0, 0);

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

