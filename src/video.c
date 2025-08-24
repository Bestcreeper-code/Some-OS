#include "headers/video.h"
#include "headers/io.h"
#include "headers/console.h"
#include "headers/asm.h"
#include "headers/mouse.h"
#include "headers/math.h"
#include "data/textconsts.h"
#include "data/globals.h"
#include "headers/multiboot_info.h"

#include <stdint.h>
#include <stdbool.h>

volatile uint8_t* fb = (uint8_t*)0xA0000;
volatile uint8_t* color_pal_size = MODE13H_COLOR_PALETTE_SIZE;






RGBColor default_palette[64] = {//all 64 default values
    {63, 63, 0},   {0, 21, 0},   {0, 63, 63},  {63, 0, 42}, {0, 0, 21},    {21, 21, 42}, {21, 0, 63},  {63, 42, 63},
    {0, 63, 21},   {63, 0, 0},   {21, 21, 0},  {21, 0, 21}, {63, 42, 21},  {21, 42, 42}, {0, 42, 42},  {42, 21, 63},
    {21, 42, 0},   {42, 0, 42},  {42, 42, 63}, {21, 63, 63}, {0, 42, 0},    {42, 21, 21}, {42, 63, 42}, {42, 63, 0},
    {42, 0, 0},    {42, 42, 21}, {21, 63, 21}, {63, 21, 42}, {63, 63, 63},  {63, 21, 0},  {0, 21, 63},  {63, 63, 21},
    {0, 21, 21},   {63, 0, 63},  {0, 0, 42},   {63, 0, 21}, {0, 63, 42},   {0, 63, 0},   {63, 42, 42}, {21, 21, 63},
    {0, 0, 0},     {21, 0, 42},  {21, 42, 63}, {63, 42, 0}, {21, 21, 21},  {0, 42, 63},  {21, 0, 0},   {42, 21, 42},
    {21, 42, 21},  {42, 0, 63},  {0, 42, 21},  {42, 63, 63}, {42, 21, 0},   {42, 0, 21},  {42, 42, 42}, {21, 63, 42},
    {42, 63, 21},  {63, 21, 63}, {42, 42, 0},  {21, 63, 0}, {63, 21, 21},  {63, 63, 42}, {0, 21, 42},  {0, 0, 63},
};

RGBColor *palette13h = MODE13H_COLOR_PALETTE;


void init_13h_palette() {
    for (int i = 0; i < 64; i++) {
        set_palette_color(i, default_palette[i].r, default_palette[i].g, default_palette[i].b);
        palette13h[i] = default_palette[i];
    }
    *color_pal_size = 64;
}



void put_pixel(int x, int y, uint8_t color) {
#if (QEMU)
    int pitch = 320;
#else
    int pitch = Get_multiboot_info()->framebuffer_pitch;
#endif
    short mx,my;
    Get_Mouse_Pos(&mx,&my);
    fb[y * pitch + x] = color;
    if(x >= mx && x < mx + 4 && y >= my && y < my + 6){
        ((uint8_t*)MOUSE_PREV_BG)[(y - my) * 4 + (x - mx)] = color;
    }
}

void Force_put_pixel(int x, int y, uint8_t color) {//no mouse check
#if (QEMU)
    int pitch = 320;
#else
    int pitch = Get_multiboot_info()->framebuffer_pitch;
#endif
    fb[y * pitch + x] = color;
}

uint8_t get_pixel(int x, int y){
    return fb[y * Get_multiboot_info()->framebuffer_pitch + x];
}

// Draw a char from 32 to 127
//(charact, charx, chary, 4, 6, color, NULL, true) for default
void draw_bitmap_char(const unsigned char character, int x_pos, int y_pos, int width, int height, char color, void* font, bool use_default_font, bool force_draw) {
    uint8_t* font_array = (uint8_t*)font;

    if (use_default_font) font_array = (uint8_t*)Base_Font4x6;
    if (width <= 0 || height <= 0 || character < 32 || character > 127) return;

    // Each character has 'width' bytes (1 byte per column)
    uint8_t* character_data = &font_array[(character - 32) * width];

    for (int x = 0; x < width; x++) {
        uint8_t column = character_data[x];
        for (int y = 0; y < height; y++) {
            if ((column >> y) & 0x01) {
                if(force_draw) Force_put_pixel(x + x_pos, y + y_pos, color);
                else put_pixel(x + x_pos, y + y_pos, color);
            }
        }
    }
}

void draw_bitmap_string(const char* str, int x_pos, int y_pos, int font_width, int font_height, char color, void* font, bool use_default_font, int space) {
    if (!str) return;

    int cursor_x = x_pos;

    while (*str) {
        draw_bitmap_char((unsigned char)*str, cursor_x, y_pos, font_width, font_height, color, font, use_default_font, false);
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

uint8_t set_new13h_color(unsigned char r, unsigned char g, unsigned char b) {
    for (uint8_t i = 0; i < *color_pal_size; i++) {
        uint32_t diff = abs(palette13h[i].r - r);
        diff += abs(palette13h[i].g - g);
        diff += abs(palette13h[i].b - b);
        diff /= 4;
        if (diff <= 3) {
            return i;
        }
    }

    if (*color_pal_size == 255) {
        return 0; // Palette full
    }

    uint8_t new_index = *color_pal_size;
    set_palette_color(new_index, r, g, b);
    palette13h[new_index].r = r;
    palette13h[new_index].g = g;
    palette13h[new_index].b = b;
    (*color_pal_size)++;
    return new_index;

}


void reset_palette(){
    memcpy((void*)palette13h,(void*)default_palette,sizeof(default_palette));
    *color_pal_size = 64;
}

uint8_t get_color_palette_size(){
    return *color_pal_size;
}