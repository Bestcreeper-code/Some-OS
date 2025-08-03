#include "headers/video.h"
#include "headers/io.h"
#include "headers/asm.h"
#include "data/textconsts.h"
#include "data/globals.h"
#include "headers/multiboot_info.h"

#include <stdint.h>
#include <stdbool.h>

volatile uint8_t* fb = (uint8_t*)0xA0000;



void vga_set_mode_03h() {
    graphics_mode = 0x03;
    // Disable video output
    outb(0x3C4, 0x01);
    outb(0x3C5, 0x01);  // Synchronous reset

    // Sequencer Registers (Index 0x3C4, Data 0x3C5)
    uint8_t seq[5] = { 0x03, 0x00, 0x03, 0x00, 0x02 };
    for (int i = 0; i < 5; ++i) {
        outb(0x3C4, i);
        outb(0x3C5, seq[i]);
    }

    // Misc Output Register
    outb(0x3C2, 0x67);  // Enable text mode clocking, VGA enabled

    // CRTC Registers (0x3D4/0x3D5)
    uint8_t crtc[25] = {
        0x5B, 0x4F, 0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F,
        0x00, 0x41, 0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50,
        0x9C, 0x0E, 0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, 0xFF
    };

    // Unlock register 0x11
    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & 0x7F);

    for (int i = 0; i < 25; ++i) {
        outb(0x3D4, i);
        outb(0x3D5, crtc[i]);
    }

    // Graphics Controller Registers (0x3CE/0x3CF)
    uint8_t gfx[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF };
    for (int i = 0; i < 9; ++i) {
        outb(0x3CE, i);
        outb(0x3CF, gfx[i]);
    }

    // Attribute Controller Registers (0x3C0)
    uint8_t attr[21] = {
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
        0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
        0x0C, 0x00, 0x0F, 0x08, 0x00
    };

    for (int i = 0; i < 21; ++i) {
        inb(0x3DA);           // Reset flip-flop
        outb(0x3C0, i);
        outb(0x3C0, attr[i]);
    }

    // Enable video output again
    inb(0x3DA);       // Reset flip-flop
    outb(0x3C0, 0x20);
}


void vga_set_mode_13h() {
    graphics_mode = 0x13;
    // Sequencer registers (Index port 0x3C4, Data port 0x3C5)
    uint8_t seq_regs[5] = { 0x03, 0x01, 0x0F, 0x00, 0x0E };

    // CRT Controller registers (0x3D4/0x3D5)
    uint8_t crtc_regs[25] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54,
        0x80, 0xBF, 0x1F, 0x00, 0x41,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x9C, 0x0E, 0x8F, 0x28, 0x40,
        0x96, 0xB9, 0xA3, 0xFF, 0x00
    };

    // Graphics Controller registers (0x3CE/0x3CF)
    uint8_t gfx_regs[9] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF };

    // Attribute Controller registers (0x3C0)
    uint8_t attr_regs[21] = {
        0x00, 0x01, 0x02, 0x03, 0x04,
        0x05, 0x06, 0x07, 0x08, 0x09,
        0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
        0x0F, 0x41, 0x00, 0x0F, 0x00,
        0x00
    };

    int i;

    // 1. Disable video output during setup
    outb(0x3C4, 0x00);
    outb(0x3C5, 0x03);

    // 2. Set sequencer registers
    for (i = 0; i < 5; i++) {
        outb(0x3C4, i);
        outb(0x3C5, seq_regs[i]);
    }

    // 3. Unlock CRTC registers (clear bit 7 of register 0x11)
    outb(0x3D4, 0x11);
    uint8_t val = inb(0x3D5);
    outb(0x3D5, val & 0x7F);

    // 4. Set CRTC registers
    for (i = 0; i < 25; i++) {
        outb(0x3D4, i);
        outb(0x3D5, crtc_regs[i]);
    }

    // 5. Set graphics controller registers
    for (i = 0; i < 9; i++) {
        outb(0x3CE, i);
        outb(0x3CF, gfx_regs[i]);
    }

    // 6. Set attribute controller registers
    for (i = 0; i < 21; i++) {
        // Reset flip-flop by reading from 0x3DA (input status register 1)
        (void)inb(0x3DA);

        outb(0x3C0, i);
        outb(0x3C0, attr_regs[i]);
    }

    // 7. Enable video output (bit 5 of attribute controller register 0x10)
    (void)inb(0x3DA);      // Reset flip-flop
    outb(0x3C0, 0x20);    // Enable video output
}



void put_pixel(int x, int y, uint8_t color) {
#if (QEMU)
    int pitch = 512;
#else
    int pitch = Get_multiboot_info()->framebuffer_pitch;
#endif
    fb[y * pitch + x] = color;
}

// Draw a char from 32 to 127
void draw_bitmap_char(const unsigned char character, int x_pos, int y_pos, int width, int height, char color, void* font, bool use_default_font) {
    uint8_t* font_array = (uint8_t*)font;

    if (use_default_font) font_array = (uint8_t*)Terminal4x6;
    if (width <= 0 || height <= 0 || character < 32 || character > 127) return;

    // Each character has 'width' bytes (1 byte per column)
    uint8_t* character_data = &font_array[(character - 32) * width];

    for (int x = 0; x < width; x++) {
        uint8_t column = character_data[x];
        for (int y = 0; y < height; y++) {
            if ((column >> y) & 0x01) {
                put_pixel(x + x_pos, y + y_pos, color);
            }
        }
    }
}

void draw_bitmap_string(const char* str, int x_pos, int y_pos, int width, int height, char color, void* font, bool use_default_font, int space) {
    if (!str) return;

    int cursor_x = x_pos;

    while (*str) {
        draw_bitmap_char((unsigned char)*str, cursor_x, y_pos, width, height, color, font, use_default_font);
        cursor_x += width + space;  // Move to next character position
        str++;
    }
}


void clear_13h_screen(char color) {
    memset((void*)0xA0000, color, 320 * 200);
}

void set_palette_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue) {
    outb(0x3C8, index);
    outb(0x3C9, red >> 2);
    outb(0x3C9, green >> 2);
    outb(0x3C9, blue >> 2);
}