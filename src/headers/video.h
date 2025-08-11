#ifndef VIDEO_H
#define VIDEO_H

#include <stdint.h>
#include <stdbool.h>

extern volatile uint8_t* fb;

void put_pixel(int x, int y, uint8_t color);
void draw_bitmap_char(const unsigned char character,int x,int y,int width,int height,char color,void* font,bool use_default_font);
void draw_bitmap_string(const char* str, int x_pos, int y_pos, int width, int height, char color, void* font, bool use_default_font, int space);

void clear_13h_screen(char color);
void set_palette_color(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
#endif