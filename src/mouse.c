#include "headers/mouse.h"
#include "headers/addresses.h"
#include "headers/video.h"
#include "headers/asm.h"
#include <stdint.h>


volatile uint8_t mouse_cycle = 0;//3 (1by1 bytes) per action
volatile uint8_t mouse_bytes[3];
volatile short* mouse_x = MOUSE_X_POS_ADDR;
volatile short* mouse_y = MOUSE_Y_POS_ADDR;
volatile uint8_t* mouse_buttons = MOUSE_FLAGS_ADDR;

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
        


        *mouse_x += x_move/2;
        *mouse_y -= y_move/2; // Y usually inverted 
        
        if(*mouse_x < 0)*mouse_x = 0; 
        if(*mouse_y < 0)*mouse_y = 0;
        if(*mouse_x > 319)*mouse_x = 319; 
        if(*mouse_y > 199)*mouse_y = 199; 

        
    }
}


bool Get_Mouse_Button(Mouse_FLAGS button){
    return (*mouse_buttons & button) != 0;
}

void Get_Mouse_Pos(short* x, short* y){
    if(x)*x = *mouse_x;
    if(y)*y = *mouse_y;
}



uint8_t mouse_icon[4] = {
    0x3F, 0x1E, 0x2C, 0x08 //4x6 mouse icon
};
uint8_t *mouse_bg = MOUSE_PREV_BG; // 4x6 = 24 bytes

void Redraw_Mouse_Cursor() {
    for (int dy = 0; dy < 6; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            int px = *MOUSE_X_POS_PREV + dx;
            int py = *MOUSE_Y_POS_PREV + dy;

    
            if (px >= 0 && px < 320 && py >= 0 && py < 200)
                Force_put_pixel(px, py, mouse_bg[dy * 4 + dx]);
        }
    }

    for (int dy = 0; dy < 6; dy++) {
        for (int dx = 0; dx < 4; dx++) {
            int px = *mouse_x + dx;
            int py = *mouse_y + dy;

    
            if (px >= 0 && px < 320 && py >= 0 && py < 200)
                mouse_bg[dy * 4 + dx] = get_pixel(px, py);
        }
    }

    draw_bitmap_char(32, *mouse_x, *mouse_y, 4, 6, 0x1A, mouse_icon, false, true);

    *MOUSE_X_POS_PREV = *mouse_x;
    *MOUSE_Y_POS_PREV = *mouse_y;
}


void enable_mouse_display(){
    *mouse_buttons |= MOUSE_DISPLAYED_FLAG;
}

void disable_mouse_display(){
    *mouse_buttons &= ~MOUSE_DISPLAYED_FLAG;
}

