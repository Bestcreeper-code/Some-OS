#include "headers/video.h"
#include "headers/io.h"
#include "headers/console.h"
#include "headers/asm.h"
#include "headers/mouse.h"
#include "headers/math.h"
#include "data/textconsts.h"
#include "data/globals.h"
#include "headers/multiboot_info.h"
#include "headers/paging.h"
#include "headers/Logger.h"
#include "headers/math.h"

#include <stdint.h>
#include <stdbool.h>

volatile uint32_t* graph_mode_fb = (uint32_t*)0xA0000;
// volatile uint8_t* color_pal_size = (volatile uint8_t*)MODE13H_COLOR_PALETTE_SIZE_ADDR;



void init_graphics() {
    graph_mode_fb = (volatile uint32_t*)(uint32_t)Multiboot_info->framebuffer_addr;

    uint32_t fb_addr = Multiboot_info->framebuffer_addr;
    uint32_t fb_size = Multiboot_info->framebuffer_pitch * Multiboot_info->framebuffer_height;
    
    Sys_log("Mapping framebuffer at 0x%x, size: %d bytes\n", fb_addr, fb_size);

    for (uint32_t offset = 0; offset < fb_size; offset += 0x1000) {
        map_page(fb_addr + offset, fb_addr + offset, 1, 1, 1);
    }

} 


void put_pixel(int x, int y, uint32_t color) {

    int pitch = Multiboot_info->framebuffer_pitch/(Multiboot_info->framebuffer_bpp/8);
    short mx,my;
    Get_Mouse_Pos(&mx,&my);
    graph_mode_fb[y * pitch + x] = color;
    if(x >= mx && x < mx + 4 && y >= my && y < my + 6){
        ((uint8_t*)MOUSE_PREV_BG)[(y - my) * 4 + (x - mx)] = color;
    }
}

void Force_put_pixel(int x, int y, uint32_t color) {//no mouse check

    int pitch = Multiboot_info->framebuffer_pitch/(Multiboot_info->framebuffer_bpp/8);
    graph_mode_fb[y * pitch + x] = color;
}

uint32_t get_pixel(int x, int y){
    return graph_mode_fb[y * Multiboot_info->framebuffer_pitch + x];
}

// Draw a char from 32 to 127
//(charact, charx, chary, 4, 6, color, NULL, true) for default
void draw_bitmap_char(const unsigned char character, int x_pos, int y_pos, int width, int height, uint32_t color, void* font, bool use_default_font, bool ignore_cursor, bool row_major) {
    if (width <= 0 || height <= 0 || character < 32 || character > 127) return;

    uint8_t* font_array = (uint8_t*)font;
    if (use_default_font) {
        font_array = (uint8_t*)Base_Font4x6;
        height = 6;
        width = 4;
    }

    int char_index = character - 32;

    if (!row_major) {
        // --- Column-major font ---
        int bytes_per_column = (height + 7) / 8;
        int bytes_per_char = width * bytes_per_column;
        uint8_t* character_data = &font_array[char_index * bytes_per_char];

        for (int x = 0; x < width; x++) {
            uint16_t column = 0;
            for (int b = 0; b < bytes_per_column; b++) {
                column |= character_data[x * bytes_per_column + b] << (8 * b);
            }
            for (int y = 0; y < height; y++) {
                if ((column >> y) & 0x01) {
                    if ((x + x_pos) < Multiboot_info->framebuffer_width && (y + y_pos) < Multiboot_info->framebuffer_height) {
                        if (ignore_cursor) Force_put_pixel(x + x_pos, y + y_pos, color);
                        else put_pixel(x + x_pos, y + y_pos, color);
                    }
                }
            }
        }
    } else {
        // --- Row-major font ---
        int bytes_per_row = (width + 7) / 8;
        int bytes_per_char = height * bytes_per_row;
        uint8_t* character_data = &font_array[char_index * bytes_per_char];

        for (int y = 0; y < height; y++) {
            uint16_t row = 0;
            for (int b = 0; b < bytes_per_row; b++) {
                row |= character_data[y * bytes_per_row + b] << (8 * b);
            }
            for (int x = 0; x < width; x++) {
                if ((row >> (width - 1 - x)) & 0x01) { // MSB first
                    if ((x + x_pos) < Multiboot_info->framebuffer_width && (y + y_pos) < Multiboot_info->framebuffer_height) {
                        if (ignore_cursor) Force_put_pixel(x + x_pos, y + y_pos, color);
                        else put_pixel(x + x_pos, y + y_pos, color);
                    }
                }
            }
        }
    }
}


void draw_bitmap_string(const char* str, int x_pos, int y_pos, int font_width, int font_height, uint32_t color, void* font, bool use_default_font, bool row_major, int space) {
    if (!str) return;

    int cursor_x = x_pos;

    while (*str) {
        draw_bitmap_char((unsigned char)*str, cursor_x, y_pos, font_width, font_height, color, font, use_default_font, false, row_major);
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




