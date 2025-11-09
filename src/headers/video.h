#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    unsigned char r;  // 0–63
    unsigned char g;
    unsigned char b;
}  __attribute__((__packed__)) RGBColor;

typedef struct {
    uint32_t x;  
    uint32_t y;
    uint32_t h;
    uint32_t w;
} __attribute__((__packed__)) Rect;

typedef struct {
    uint32_t x;  
    uint32_t y;
} __attribute__((__packed__)) Vector2;



extern volatile uint32_t* graph_mode_fb;

void put_pixel(int x, int y, uint32_t color);
uint32_t get_pixel(int x, int y);
void draw_bitmap_char(const unsigned char character, int x, int y, int font_width, int font_height, uint32_t color, void* font, bool use_default_font, bool ignore_cursor, bool row_major);
void draw_bitmap_string(const char* str, int x_pos, int y_pos, int font_width, int font_height, uint32_t color, void* font, bool use_default_font, bool row_major, int space);

void draw_rect(Rect rect,uint32_t color);

void clear_13h_screen(char color);
void set_palette_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);

uint8_t get_color_palette_size();
uint8_t set_new13h_color(unsigned char r, unsigned char g, unsigned char b);
RGBColor get_palette_color(uint8_t index);

//no mouse check
void Force_put_pixel(int x, int y, uint32_t color);

void reset_palette();

void init_graphics();
#endif