#ifndef VGA_MODES_H
#define VGA_MODES_H




#include <stdint.h>
typedef struct  {
    uint8_t misc;
    uint8_t seq[5];
    uint8_t crtc[25];
    uint8_t gc[9];
    uint8_t attr[21];
} vga_mode;

void vga_set_mode(const char mode);
void vga_load_font(const uint8_t *font, uint16_t height);

#endif // VGA_MODES_H