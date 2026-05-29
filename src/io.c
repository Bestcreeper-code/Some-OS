#include "io.h"
#include "Logger.h"
#include "bootloader.h"
#include "string.h"
#include "asm-defs/asm.h"
#include "video.h"
#include "time.h"
#include "FileSystem.h"
#include "kernel_data.h"
#include "video.h"
#include "config/config.h"
#include "data/textconsts.h"
#include "data/KB_Layouts.h"



int vgaX = 0;
int vgaY = 0;

char current_Language = KB_LAY_AZERTY;

static char print_color = 0x0F; //white on black


//  TEXT BASED FUNCS
unsigned int vga_to_32bit_color(unsigned char vga_color) {
    
    

    
    unsigned char index = vga_color & 0x0F;

    
    return k_console_palette[index];
}

const uint8_t default_kterm_font_w = 8;
const uint8_t default_kterm_font_h = 16;

void put_char(int x, int y, uint8_t c, uint8_t color) {
    if (x < 0 || x >= K_TERMINAL_WIDTH || y < 0 || y >= K_TERMINAL_HEIGHT)
        return;

    int px = x * default_kterm_font_w;
    int py = y * default_kterm_font_h;

    Rect r = {.x = px, .y = py, .w = default_kterm_font_w, .h = default_kterm_font_h };

    if (c == 32) {
        draw_rect(r, vga_to_32bit_color(0)); //console bg 
        return;
    }

    draw_bitmap_char(c, px, py, default_kterm_font_w, default_kterm_font_h,
                     vga_to_32bit_color(color), font8x16, false, true, true );
}

void _putchar(char character){
    put_char(vgaX, vgaY, character, print_color);
    vgaX++;
    if (vgaX >= K_TERMINAL_WIDTH) {
        vgaX = 0;
        vgaY++;
        if (vgaY >= K_TERMINAL_HEIGHT) {
            vgaY = K_TERMINAL_HEIGHT -1;
            Scroll_Down();
        }
    }
}



void Scroll_Down(void) {
    
    size_t line_bytes = get_bootloader_fb_info()->pitch ;

    size_t shift = default_kterm_font_h * line_bytes;

    size_t fb_size = get_bootloader_fb_info()->height * get_bootloader_fb_info()->pitch ;

    uint8_t *fb = (uint8_t *)(uintptr_t)get_bootloader_fb_info()->addr;
    memmove(fb, fb + shift, fb_size - shift);
    memset(fb + fb_size - shift, 0, shift);
}





void ClearScreen() {
    move_cursor(0,0);

    size_t fb_size = get_bootloader_fb_info()->width * get_bootloader_fb_info()->height ;
    dw_memset((void*)_display_fb, vga_to_32bit_color(0), fb_size);
}

// short old_cmdline_cursor_pos

void move_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= K_TERMINAL_WIDTH) x = K_TERMINAL_WIDTH - 1;
    if (y < 0) y = 0;
    if (y >= K_TERMINAL_HEIGHT) y = K_TERMINAL_HEIGHT - 1;
    vgaX = x;
    vgaY = y;
    /////////////////////////////////////////////////////////////////////////////////////////////////
}






// Keyboard input handling + character decoding

bool extended = false;

volatile uint8_t input_char_buffer[256] ;





unsigned char GetInputCharNonBlocking(void) {
    uint8_t status;
    __asm__ __volatile__("inb $0x64, %0" : "=a"(status));
    if (!(status & 0x01)) {
        extended = false;  // no data, reset state
        return 0;
    }

    uint8_t scancode;
    __asm__ __volatile__("inb $0x60, %0" : "=a"(scancode));

    if (scancode == 0xE0) {
        extended = true;
        return 0;  // wait for next scancode
    }

    bool released = (scancode & 0x80) != 0;
    uint8_t keycode = scancode & 0x7F;

    if (extended) {
        if (released) {
            if (keycode == 0x1D)
                SET_KEYBOARD_MOD_FLAG(CTRL_PRESSED, false);
            else if (keycode == 0x38)
                SET_KEYBOARD_MOD_FLAG(ALTGR_PRESSED, false);
            extended = false;
            return 0;
        }

        unsigned char c = 0;
        switch (keycode) {
            case 0x48: c = KEY_UP; break;
            case 0x50: c = KEY_DOWN; break;
            case 0x4B: c = KEY_LEFT; break;
            case 0x4D: c = KEY_RIGHT; break;
            case 0x47: c = KEY_HOME; break;
            case 0x1D: SET_KEYBOARD_MOD_FLAG(CTRL_PRESSED, true); break;
            case 0x38: SET_KEYBOARD_MOD_FLAG(ALTGR_PRESSED, true); break;
        }
        extended = false;
        return c;
    }

    // Non-extended modifier keys
    switch (keycode) {
        case 0x2A: // Left Shift
        case 0x36: // Right Shift
            SET_KEYBOARD_MOD_FLAG(SHIFT_PRESSED, !released);
            return 0;
        case 0x3A: // Caps Lock toggle on key press
            if (!released) {
                bool caps_state = (GET_KEYBOARD_MOD_FLAG(CAPSLOCK_ON) != 0);
                SET_KEYBOARD_MOD_FLAG(CAPSLOCK_ON, !caps_state);
            }
            return 0;
        case 0x1D: // Ctrl
            SET_KEYBOARD_MOD_FLAG(CTRL_PRESSED, !released);
            return 0;
        case 0x38: // Alt
            SET_KEYBOARD_MOD_FLAG(ALT_PRESSED, !released);
            return 0;
    }

    if (released) return 0;

    // Determine modifier for keymap lookup
    KeyModifier mod = MOD_Normal;
    if (GET_KEYBOARD_MOD_FLAG(ALTGR_PRESSED)) {
        mod = MOD_AltGr;
    } else if ((GET_KEYBOARD_MOD_FLAG(SHIFT_PRESSED) != 0) ^ (GET_KEYBOARD_MOD_FLAG(CAPSLOCK_ON) != 0)) {
        mod = MOD_Shift;
    }

    unsigned char base_char = keymaps[current_Language][mod][keycode];

    if ((GET_KEYBOARD_MOD_FLAG(CTRL_PRESSED) != 0) && base_char >= 'A' && base_char <= 'z' && !GET_KEYBOARD_MOD_FLAG(ALTGR_PRESSED)) {
        base_char = base_char - 'A' + CTRL_KEY_COMBO;
    }
    // Sys_log("%c %d",base_char,(int)base_char);
    return base_char;
}



unsigned char getc(){
    unsigned char chr = 0;
    while(chr == 0){
        if(input_char_buffer[0]){
            // Sys_log("c:%c  icb:%x \n\n",input_char_buffer[0],(int)input_char_buffer[0]);
            chr = input_char_buffer[0];
            // Sys_log("%c %c %c %c",input_char_buffer[0],input_char_buffer[1], input_char_buffer[2], input_char_buffer[3])
            for (int i = 0; i < INPUT_CHAR_BUFFER_SIZE-1; i++) {
                input_char_buffer[i] = input_char_buffer[i+1];
            }
           

            input_char_buffer[INPUT_CHAR_BUFFER_SIZE-1] = 0;
        }
        sleep(1);//since it fixes it smh
    }
    
    return chr;
}


unsigned char getc_nb(){
    unsigned char chr = 0;

    if(input_char_buffer[0]){
        chr = input_char_buffer[0];
        memcpy((void*)&input_char_buffer[0],(void*)&input_char_buffer[1],INPUT_CHAR_BUFFER_SIZE-1);
    }

    return chr;
}



void get_string_after_index(int start, char* buffer) {
    int length = 0;
    int cursor_index = 0;

    buffer[0] = '\0';  // initialize empty

    while (true) {
        unsigned char c = getc();

        if (c == 0x08) {  // Backspace
            if (cursor_index > 0) {
                memmove(&buffer[cursor_index - 1], &buffer[cursor_index], length - cursor_index);
                length--;
                cursor_index--;
                buffer[length] = '\0';
            }

        } else if (c >= 32 && c <= 126) {  // Printable characters
            if (length < 255) {  // optional max length guard
                memmove(&buffer[cursor_index + 1], &buffer[cursor_index], length - cursor_index);
                buffer[cursor_index] = c;
                length++;
                cursor_index++;
                buffer[length] = '\0';
            }

        } else if (c == KEY_LEFT) {
            if (cursor_index > 0) cursor_index--;

        } else if (c == KEY_RIGHT) {
            if (cursor_index < length) cursor_index++;

        } else if (c == '\n') {
            buffer[length] = '\0';
            return;
        }

        // Update cursor position on screen
        vgaX = start + cursor_index;
        move_cursor(vgaX, vgaY);
    }
}

void get_string(char* buffer) {
    return get_string_after_index(vgaX,buffer);
}



#include <stdint.h>

void set_print_color(char color){
    print_color = color;
}

void init_keyboard(){
    uint8_t master_mask = inb(0x21);  // Master PIC
    master_mask &= ~(1 << 1);         // Unmask IRQ1 (keyboard)
    outb(0x21, master_mask);
}


void reset_input_buffer(){
    memset((void*)input_char_buffer, 0, INPUT_CHAR_BUFFER_SIZE);
}




