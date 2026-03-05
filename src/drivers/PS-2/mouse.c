#include "mouse.h"
#include "Logger.h"
#include "kernel_data.h"
#include "video.h"
#include "asm.h"
#include "Logger.h"
#include "io.h"
#include <stdint.h>


#define MAX_MOUSE_X Multiboot_info->framebuffer_width
#define MAX_MOUSE_Y Multiboot_info->framebuffer_height


volatile uint8_t mouse_cycle = 0;//3 (1by1 bytes) per action
volatile uint8_t mouse_bytes[3];
volatile short* mouse_x = (volatile short*)&MOUSE_X_POS;
volatile short* mouse_y = (volatile short*)&MOUSE_Y_POS;
volatile uint8_t* mouse_buttons = (volatile uint8_t*)&MOUSE_FLAGS;

void mouse_wait(uint8_t type) {
    if (type == 0) {
        while ((inb(0x64) & 1) == 0) {
            __asm__ volatile ("hlt");
        }
    } else {
        while ((inb(0x64) & 2) != 0) {
            __asm__ volatile ("hlt");
        }
    }
}

void mouse_write(uint8_t a_write) {
    mouse_wait(1);
    outb(0x64, 0xD4);
    mouse_wait(1);
    outb(0x60, a_write);
}

uint8_t mouse_read() {
    mouse_wait(0);
    return inb(0x60);
}

void init_mouse() {
    uint8_t mask = inb(0xA1);
    mask &= ~(1 << 4);
    outb(0xA1, mask);

    mouse_wait(1);
    outb(0x64, 0xA8);

    mouse_wait(1);
    outb(0x64, 0x20);
    mouse_wait(0);
    uint8_t status = (inb(0x60) | 2);
    mouse_wait(1);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, status);

    mouse_write(0xF6);
    mouse_read();

    mouse_write(0xF4);
    mouse_read();

    *mouse_x = 160;
    *mouse_y = 100;
    *mouse_buttons = 0;
}

void mouse_irq_handler() {
    uint8_t status = inb(0x64);
    if (!(status & 0x20)) return;  // no mouse data ready

    uint8_t data = inb(0x60);
    mouse_bytes[mouse_cycle] = data;
    mouse_cycle = (mouse_cycle + 1) % 3;

    if (mouse_cycle == 0 && (*mouse_buttons & MOUSE_DISPLAYED_FLAG)) { // full packet received
        // Byte 0: buttons and sign bits
        uint8_t buttons = mouse_bytes[0] & 0x07; // left, right, middle buttons (Left | Right | Middle)
        int8_t x_move = (int8_t)mouse_bytes[1];  // X movement delta (signed)
        int8_t y_move = (int8_t)mouse_bytes[2];  // Y movement delta (signed)

        *mouse_buttons = (*mouse_buttons & ~0x07) | (buttons & 0x07);
        


        *mouse_x += x_move;
        *mouse_y -= y_move; // Y inverted 
        
        if(*mouse_x < 0)*mouse_x = 0; 
        if(*mouse_y < 0)*mouse_y = 0;
        if(*mouse_x > MAX_MOUSE_X-1)*mouse_x = MAX_MOUSE_X-1; 
        if(*mouse_y > MAX_MOUSE_Y-1)*mouse_y = MAX_MOUSE_Y-1; 

        
    }

}


bool Get_Mouse_Button(Mouse_FLAGS button){
    return (*mouse_buttons & button) != 0;
}

void Get_Mouse_Pos(short* x, short* y){
    if(x)*x = *mouse_x;
    if(y)*y = *mouse_y;
}




const uint8_t def_mouse_icons6x8[] = {
     0x3F, 0x1E, 0x2C, 0x08, 0x00, 0x00,
     0x3F, 0x1E, 0x2C, 0x0F, 0x05, 0x07,
     0x3F, 0x1E, 0x2C, 0x0B, 0x5D, 0x07,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x21, 0x12, 0x0C, 0x0C, 0x12, 0x21,
     0xC3, 0xA5, 0x99, 0x99, 0xA5, 0xC3,
     0x0F, 0x03, 0x05, 0x09, 0x10, 0x20,
     0x0E, 0x15, 0x1F, 0x15, 0x2E, 0x40,
     0x0E, 0x15, 0x15, 0x15, 0x2E, 0x40,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x41, 0x41, 0x7F, 0x41, 0x41, 0x00,
     0x1C, 0x3E, 0x08, 0x08, 0x3E, 0x1C,
     0x24, 0x66, 0xFF, 0xFF, 0x66, 0x24,
     0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
     0x30, 0x7E, 0x78, 0x78, 0x78, 0x30,
     0x3F, 0x78, 0x7F, 0x78, 0x7F, 0x30,
     0x3E, 0x7C, 0x7E, 0x7C, 0x7E, 0x34 
};


// uint8_t mouse_icon8x12[] = {
//     0xFF, 0b111111, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
// };
    
uint32_t mouse_bg [256];
    

static uint8_t mouse_state = 0;
static uint8_t mouse_w = 6;
static uint8_t mouse_h = 8;

void Redraw_Mouse_Cursor() {
    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = MOUSE_X_POS_PREV + dx;
            int py = MOUSE_Y_POS_PREV + dy;

    
            if (px >= 0 && px < MAX_MOUSE_X && py >= 0 && py < MAX_MOUSE_Y)
                Force_put_pixel(px, py, mouse_bg[dy * 16 + dx]);
        }
    }

    for (int dy = 0; dy < 16; dy++) {
        for (int dx = 0; dx < 16; dx++) {
            int px = *mouse_x + dx;
            int py = *mouse_y + dy;

    
            if (px >= 0 && px < MAX_MOUSE_X && py >= 0 && py < MAX_MOUSE_Y){
                
                mouse_bg[dy * 16 + dx] = get_pixel(px, py);
            }
        }
    }

    draw_bitmap_char(32+mouse_state, *mouse_x, *mouse_y, mouse_w, mouse_h, 0xFFFFFFFF, (char*)def_mouse_icons6x8, false, true, false);

    MOUSE_X_POS_PREV = *mouse_x;
    MOUSE_Y_POS_PREV = *mouse_y;
}


void enable_mouse_display(){
    *mouse_buttons |= MOUSE_DISPLAYED_FLAG;
}

void disable_mouse_display(){
    *mouse_buttons &= ~MOUSE_DISPLAYED_FLAG;
}

void change_mouse_state(Cursorstate state){
    mouse_state = state;
}