#include "headers/video.h"
#include "headers/io.h"
#include "headers/console.h"
#include "headers/asm.h"
#include "data/textconsts.h"
#include "data/globals.h"
#include "headers/multiboot_info.h"

#include <stdint.h>
#include <stdbool.h>

volatile uint8_t* fb = (uint8_t*)0xA0000;


extern  uint8_t font8x16[256 * 16]; // define somewhere in your data section






void put_pixel(int x, int y, uint8_t color) {
#if (QEMU)
    int pitch = 320;
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

void draw_bitmap_string(const char* str, int x_pos, int y_pos, int font_width, int font_height, char color, void* font, bool use_default_font, int space) {
    if (!str) return;

    int cursor_x = x_pos;

    while (*str) {
        draw_bitmap_char((unsigned char)*str, cursor_x, y_pos, font_width, font_height, color, font, use_default_font);
        cursor_x += font_width + space;  // Move to next character position
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