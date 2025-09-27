#include "headers/io.h"
#include "headers/string.h"
#include "headers/asm.h"
#include "headers/video.h"
#include "headers/time.h"
#include "headers/FileSystem.h"
#include "headers/Logger.h"
#include "headers/addresses.h"
#include "data/globals.h"
#include "data/textconsts.h"
#include "data/KB_Layouts.h"
#include "headers/multiboot_info.h"

volatile uint16_t* text_mode_memory = (volatile uint16_t*)0xB8000;
int vgaX = 0;
int vgaY = 0;

char current_Language = KB_LAY_AZERTY;

static char print_color = 0x0F; //white on black


// VGA TEXT BASED FUNCS


void put_char(int x, int y,uint8_t c, uint8_t color) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
    // switch(graphics_mode){
    //     case 0x03:
            text_mode_memory[y * 80 + x] = (uint16_t)c | ((uint16_t)color << 8);
            Rect window = {.h=768, .w=1024, .x=0, .y=0};
            vga_txt_to_gfx(window);
            // break;

        // case 0x13:
        //     draw_bitmap_char(c,x * 4,y * 8,4,6,color,NULL,true);
        //     break;
    // }
}

char get_char(int x, int y) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return 0;
    return (char)(text_mode_memory[y * 80 + x] & 0xFF);
}

void enable_cursor(uint8_t start, uint8_t end) {
    outb(0x3D4, 0x0A);
    outb(0x3D5, (inb(0x3D5) & 0xC0) | start);

    outb(0x3D4, 0x0B);
    outb(0x3D5, (inb(0x3D5) & 0xE0) | end);
}


void Scroll_Down() {
    int i, j;
    
    for (i = 0; i < 24; i++) { 
        for (j = 0; j < 80; j++) { 
            text_mode_memory[i * 80 + j] = text_mode_memory[(i + 1) * 80 + j];
        }
    }
    
    for (j = 0; j < 80; j++) {
        text_mode_memory[24 * 80 + j] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    }
    
    
}

void ClearScreen() {
    move_cursor(0,0);
    for (int i = 0; i < 80*25; i++) {
        text_mode_memory[i] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    }
    clear_13h_screen(1);
}

void move_cursor(int x, int y) {
    if (x < 0) x = 0;
    if (x >= 80) x = 79;
    if (y < 0) y = 0;
    if (y >= 25) y = 24;
    vgaX = x;
    vgaY = y;

    uint16_t pos = y * 80 + x;
    __asm__ volatile ("outb %0, %1" : : "a"((char)0x0F), "d"((uint16_t)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((char)(pos & 0xFF)), "d"((uint16_t)0x3D5));
    __asm__ volatile ("outb %0, %1" : : "a"((char)0x0E), "d"((uint16_t)0x3D4));
    __asm__ volatile ("outb %0, %1" : : "a"((char)((pos >> 8) & 0xFF)), "d"((uint16_t)0x3D5));

}

int printstr(const char* str) {
    int char_count=0;
    while (*str) {
        if (*str == '\n') {
            vgaX = 0;
            vgaY++;
            if (vgaY >= 25) {
                vgaY = 24; 
                Scroll_Down();
            }
        } else {
            put_char(vgaX, vgaY, *str, print_color);
            vgaX++;
            if (vgaX >= 80) {
                vgaX = 0;
                vgaY++;
                if (vgaY >= 25) {
                    vgaY = 24; 
                    Scroll_Down();
                }
            }
        }
        str++;
        char_count++;
    }
    move_cursor(vgaX, vgaY);  
    return char_count;
}
int printlen(const char *buffer, unsigned int length) {
    for (unsigned int i = 0; i < length; i++) {
        if (buffer[i] == '\n') {
            vgaX = 0;
            vgaY++;
            if (vgaY >= 25) {
                vgaY = 24; 
                Scroll_Down();
            }
        } else {
            put_char(vgaX, vgaY, buffer[i], print_color);
            vgaX++;
            if (vgaX >= 80) {
                vgaX = 0;
                vgaY++;
                if (vgaY >= 25) {
                    vgaY = 24; 
                    Scroll_Down();
                }
            }
        }
    }
    move_cursor(vgaX, vgaY);
    return length;
}






// Keyboard input handling + character decoding

bool extended = false;

volatile uint8_t* input_char_buffer = (volatile uint8_t*)INPUT_CHAR_BUFFER_ADDRESS;




// unsigned char GetInputChar() {
//     unsigned char c = 0;
//     while (c == 0) {
//         uint8_t status;
//         __asm__ __volatile__("inb $0x64, %0" : "=a"(status));
//         if (!(status & 0x01)) continue;

//         uint8_t scancode;
//         __asm__ __volatile__("inb $0x60, %0" : "=a"(scancode));

//         if (scancode == 0xE0) {
//             extended = true;
//             continue;
//         }

//         bool released = (scancode & 0x80) != 0;
//         uint8_t keycode = scancode & 0x7F;

//         if (extended) {
//             switch (keycode) {
//                 case 0x1D: ctrl_pressed = !released; break;   // Right Ctrl (extended)
//                 case 0x38: altgr_pressed = !released; break;  // Right Alt (AltGr)
//             }
//             if (released) {
//                 extended = false;
//                 continue; // skip extended key release codes (if not caught with the switchable keys switch statement )
//             }
//             switch (keycode) {
//                 case 0x48: c = KEY_UP; break;
//                 case 0x50: c = KEY_DOWN; break;
//                 case 0x4B: c = KEY_LEFT; break;
//                 case 0x4D: c = KEY_RIGHT; break;
//                 case 0x47: c = KEY_HOME; break;

//                 default:
//                     // Unknown extended code: ignore or handle here
//                     break;
//             }
//             extended = false;
//             if (c != 0) return c;
//             continue;
//         }

//         // Handle modifier keys (non-extended)
//         switch (keycode) {
//             case 0x2A: // Left Shift
//             case 0x36: // Right Shift
//                 GET_KEYBOARD_MOD_FLAG() = !released;
//                 break;
//             case 0x3A: // Caps Lock
//                 if (!released) caps_lock_on = !caps_lock_on;
//                 break;
//             case 0x1D: // Left Ctrl
//                 ctrl_pressed = !released;
//                 break;
//             case 0x38: // Left Alt
//                 alt_pressed = !released;
//                 break;
//         }
//         if (released) continue; // ignore key releases for chars

//         // Determine current modifier state
//         KeyModifier mod = MOD_Normal;
//         if (altgr_pressed) {
//             mod = MOD_AltGr;
//         } else if (shift_pressed ^ caps_lock_on) {
//             mod = MOD_Shift;
//         }

//         unsigned char base_char = keymaps[current_Language][mod][keycode];

//         // Ctrl modifies only a-z chars and only if AltGr NOT pressed
//         if (ctrl_pressed && base_char >= 'a' && base_char <= 'z' && !altgr_pressed) {
//             base_char = base_char -'a' + CTRL_KEY_COMBO; // Ctrl + letter → control char
//         }

//         c = base_char;
//     }
//     return c;
// }



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

    return base_char;
}



unsigned char getc(){
    unsigned char chr = 0;
    while(chr == 0){
        if(input_char_buffer[0]){
            // Sys_log("[GETC] c:%c  icb:%x",input_char_buffer[0],input_char_buffer);
            chr = input_char_buffer[0];
            memcpy((void*)&input_char_buffer[0],(void*)&input_char_buffer[1],INPUT_CHAR_BUFFER_SIZE-1);
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


int print_hex32(uint32_t value) {
    int char_count = 0;
    const char* hex = "0123456789ABCDEF";
    
    for (int i = 7; i >= 0; i--) {
        uint8_t nibble = (value >> (i * 4)) & 0xF;
        char_count += printlen(&hex[nibble],1);
    }
    return char_count;
}

int print_number(int num) {
    char buffer[12];  
    int i = 0;
    bool isNegative = false;

    if (num == 0) {
        printstr("0");
        return 1;
    }

    if (num < 0) {
        isNegative = true;
        
        if (num == -2147483648) {
            
            printstr("-2147483648");
            return 11;
        }
        num = -num;
    }

    while (num > 0 && i < (int)(sizeof(buffer) - 1)) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }

    if (isNegative) {
        buffer[i++] = '-';
    }

    buffer[i] = '\0';

    
    for (int j = 0; j < i / 2; j++) {
        char temp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = temp;
    }

    printstr(buffer);
    return i;  
}

int print_unsigned_number(uint32_t num) {
    char buffer[11];
    int i = 0;
    if (num == 0) {
        printstr("0");
        return 1;
    }
    while (num > 0 && i < (int)(sizeof(buffer) - 1)) {
        buffer[i++] = (num % 10) + '0';
        num /= 10;
    }
    buffer[i] = '\0';

    
    for (int j = 0; j < i / 2; j++) {
        char tmp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = tmp;
    }
    printstr(buffer);
    return i;
}

#include <stdint.h>


int print_unsigned_long_long(uint64_t num) {
    char buffer[21]; 
    int i = 0;

    if (num == 0) {
        printstr("0");
        return 1;
    }

    while (num != 0) {
        // Manual division by 10 using shifts and subtracts
        uint64_t q = 0, r = 0;
        for (int bit = 63; bit >= 0; bit--) {
            r <<= 1;
            r |= (num >> bit) & 1;

            if (r >= 10) {
                r -= 10;
                q |= (1ULL << bit);
            }
        }

        buffer[i++] = '0' + (char)r;
        num = q;
    }

    // Reverse buffer
    for (int j = 0; j < i / 2; j++) {
        char tmp = buffer[j];
        buffer[j] = buffer[i - j - 1];
        buffer[i - j - 1] = tmp;
    }

    buffer[i] = '\0';
    printstr(buffer);
    return i;
}


int printf(const char* frmt, ...){
    int char_count=0;
    va_list args;
    va_start(args, frmt);
    while (*frmt)
    {
        if (*frmt == '%'){
            frmt++;
            switch (*frmt)
            {
            case 'd':
                char_count += print_number(va_arg(args,int));
                break;
            case 'u':
                char_count += print_unsigned_number(va_arg(args,uint32_t));
                break;
            case 'l':
                if (*(frmt+1) == 'l' && *(frmt+2) == 'u'){
                    char_count += print_unsigned_long_long(va_arg(args,uint64_t));
                    frmt+=2;
                    break;
                }
            case 'x':
                char_count += print_hex32(va_arg(args,int));
                break;
            case 'c':
                char c = (char)va_arg(args, int);
                char_count += printlen(&c, 1);
                break;
            case 's':
                char_count += printstr(va_arg(args,char*));
                break;
            case 'p':
                char_count += printstr("0x");
                char_count += print_hex32((uint32_t)(uintptr_t)va_arg(args, void*));
                break;
            case '%':
                char_count += printstr("%%");
                break;    
            default:
                return 0;
            }
        }
        else 
        {
            printlen(frmt, 1);
        }
        
        frmt++;
        
    }
    return char_count;
}

int printLine(const char* str, int line){
    int amount = 0;
    while (amount < line){
        if(*str=='\n')amount++;
        if(*str=='\0')return 0;
        str++;
    }
    if(*str =='\n')str++;
    amount = 0;
    while(*str != '\0' && *str != '\n' && vgaX < 80) {
        put_char(vgaX, vgaY, *str, 0x0F);
        str++;
        vgaX++;
        amount++;
    }
    put_char(vgaX++, vgaY, '\0', 0x0F); 
    return amount;
}

void set_print_color(char color){
    print_color = color;
}

// int load_Language(){
//     f_
//     f_read()
// } MAKE LATEr

void init_keyboard(){
    uint8_t master_mask = inb(0x21);  // Master PIC
    master_mask &= ~(1 << 1);         // Unmask IRQ1 (keyboard)
    outb(0x21, master_mask);
}





//sprintf HELPER FUNCTIONS



int write_char(char* buffer, int pos, char c, int size) {
    // If size == 0, ignore size limits (unbounded)
    if (buffer && (size == 0 || pos < size - 1)) {
        buffer[pos] = c;
    }
    return 1;
}

int write_str(char* buffer, int pos, const char* s, int size) {
    int i = 0;
    while (s[i]) {
        if (buffer && (size == 0 || pos + i < size - 1)) {
            buffer[pos + i] = s[i];
        }
        i++;
    }
    return i;
}

int write_number(char* buffer, int pos, int num, int size) {
    char temp[12];
    int i = 0;
    bool negative = false;

    if (num == 0) {
        return write_char(buffer, pos, '0', size);
    }

    if (num < 0) {
        negative = true;
        if (num == (int)0x80000000) {
            // Special case for INT_MIN
            return write_str(buffer, pos, "-2147483648", size);
        }
        num = -num;
    }

    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }

    int total_len = i + (negative ? 1 : 0);

    int cur_pos = pos;
    if (negative) {
        if (buffer && (size == 0 || cur_pos < size - 1)) buffer[cur_pos] = '-';
        cur_pos++;
    }

    for (int j = 0; j < i; j++) {
        if (buffer && (size == 0 || cur_pos < size - 1)) {
            buffer[cur_pos] = temp[i - j - 1];
        }
        cur_pos++;
    }

    return total_len;
}

int write_unsigned(char* buffer, int pos, uint32_t num, int size) {
    char temp[11];
    int i = 0;

    if (num == 0) {
        return write_char(buffer, pos, '0', size);
    }

    while (num > 0) {
        temp[i++] = (num % 10) + '0';
        num /= 10;
    }

    int cur_pos = pos;
    for (int j = 0; j < i; j++) {
        if (buffer && (size == 0 || cur_pos < size - 1)) {
            buffer[cur_pos] = temp[i - j - 1];
        }
        cur_pos++;
    }
    return i;
}

int write_hex32(char* buffer, int pos, uint32_t num, int size) {
    const char* hex = "0123456789ABCDEF";
    char temp[8];
    int i = 0;

    if (num == 0) {
        return write_char(buffer, pos, '0', size);
    }

    while (num > 0) {
        temp[i++] = hex[num & 0xF];
        num >>= 4;
    }

    int cur_pos = pos;
    for (int j = 0; j < i; j++) {
        if (buffer && (size == 0 || cur_pos < size - 1)) {
            buffer[cur_pos] = temp[i - j - 1];
        }
        cur_pos++;
    }
    return i;
}

int write_number_fixed_width(char* buffer, int pos, int num, int width, int size) {
    char temp[20];
    int i = 0;
    bool negative = false;

    if (num == 0) {
        i = 1;
        temp[0] = '0';
    } else {
        if (num < 0) {
            negative = true;
            num = -num;
        }

        while (num > 0) {
            temp[i++] = (num % 10) + '0';
            num /= 10;
        }
    }

    int digits_to_print = (width > i) ? width : i;
    int total_len = digits_to_print + (negative ? 1 : 0);

    int cur_pos = pos;

    if (negative) {
        if (buffer && (size == 0 || cur_pos < size - 1)) buffer[cur_pos] = '-';
        cur_pos++;
    }

    for (int pad = digits_to_print - i; pad > 0; pad--) {
        if (buffer && (size == 0 || cur_pos < size - 1)) buffer[cur_pos] = '0';
        cur_pos++;
    }

    for (int j = i - 1; j >= 0; j--) {
        if (buffer && (size == 0 || cur_pos < size - 1)) {
            buffer[cur_pos] = temp[j];
        }
        cur_pos++;
    }

    return total_len;
}

// buffer be NULL to just calculate size 
int vsnprintf(char* buffer, int size, const char* format, va_list args) {
    int pos = 0;

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == '\0') break;

            // Handle %0Nd for fixed width integers
            if (*format == '0') {
                format++;
                int width = 0;
                while (*format >= '0' && *format <= '9') {
                    width = width * 10 + (*format - '0');
                    format++;
                }
                if (*format == 'd') {
                    int val = va_arg(args, int);
                    pos += write_number_fixed_width(buffer, pos, val, width, size);
                    format++;
                    continue;
                } else {
                    if (pos < size - 1 && buffer) buffer[pos] = '%';
                    pos++;
                    if (pos < size - 1 && buffer) buffer[pos] = '0';
                    pos++;
                    const char* rewind_fmt = format;
                    while (rewind_fmt > format - 10 && *(rewind_fmt - 1) >= '0' && *(rewind_fmt - 1) <= '9') {
                        if (pos < size - 1 && buffer) buffer[pos] = *(rewind_fmt - 1);
                        pos++;
                        rewind_fmt--;
                    }
                    if (pos < size - 1 && buffer) buffer[pos] = *format;
                    pos++;
                    format++;
                    continue;
                }
            }

            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    pos += write_number(buffer, pos, val, size);
                    break;
                }
                case 'u': {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_unsigned(buffer, pos, val, size);
                    break;
                }
                case 'x':case 'p':
                case 'X': {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_hex32(buffer, pos, val, size);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    pos += write_char(buffer, pos, c, size);
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    if (!s) s = "(null)";
                    pos += write_str(buffer, pos, s, size);
                    break;
                }
                case '%': {
                    pos += write_char(buffer, pos, '%', size);
                    break;
                }
                default: {
                    pos += write_char(buffer, pos, '%', size);
                    pos += write_char(buffer, pos, *format, size);
                    break;
                }
            }
        } else {
            pos += write_char(buffer, pos, *format, size);
        }
        format++;
    }

    if (buffer && size > 0) {
        if (pos >= size) pos = size - 1;
        buffer[pos] = '\0';
    }

    return pos; // characters that would have been written (excluding null)
}



int vsprintf(char* buffer, const char* format, va_list args) {
    int pos = 0;

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == '\0') break;

            // Handle %0Nd for fixed width integers
            if (*format == '0') {
                format++;
                int width = 0;
                while (*format >= '0' && *format <= '9') {
                    width = width * 10 + (*format - '0');
                    format++;
                }
                if (*format == 'd') {
                    int val = va_arg(args, int);
                    pos += write_number_fixed_width(buffer, pos, val, width, 0);
                    format++;
                    continue;
                } else {
                    buffer[pos++] = '%';
                    buffer[pos++] = '0';
                    const char* rewind_fmt = format;
                    while (rewind_fmt > format - 10 && *rewind_fmt >= '0' && *rewind_fmt <= '9') {
                        buffer[pos++] = *(rewind_fmt - 1);
                        rewind_fmt--;
                    }
                    buffer[pos++] = *format;
                    format++;
                    continue;
                }
            }

            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    pos += write_number(buffer, pos, val, 0);
                    break;
                }
                case 'u': {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_unsigned(buffer, pos, val, 0);
                    break;
                }
                case 'x':
                case 'X': {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_hex32(buffer, pos, val, 0);
                    break;
                }
                case 'c': {
                    char c = (char)va_arg(args, int);
                    pos += write_char(buffer, pos, c, 0);
                    break;
                }
                case 's': {
                    const char* s = va_arg(args, const char*);
                    pos += write_str(buffer, pos, s, 0);
                    break;
                }
                case '%': {
                    pos += write_char(buffer, pos, '%', 0);
                    break;
                }
                default: {
                    pos += write_char(buffer, pos, '%', 0);
                    pos += write_char(buffer, pos, *format, 0);
                    break;
                }
            }
        } else {
            pos += write_char(buffer, pos, *format, 0);
        }
        format++;
    }

    buffer[pos] = '\0';

    return pos; // number of characters written
}


int snprintf(char* buffer, int size, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsnprintf(buffer, size, format, args);
    va_end(args);
    return ret;
}

int sprintf(char* buffer, const char* format, ...) {
    va_list args;
    va_start(args, format);
    int ret = vsprintf(buffer, format, args);
    va_end(args);
    return ret;
}

void reset_input_buffer(){
    memset((void*)input_char_buffer, 0, INPUT_CHAR_BUFFER_SIZE);
}





// GRAPHICAL BASED FUNCS(modified vga text ones)


void decode_vga_colors(uint8_t attr, uint32_t* fg, uint32_t* bg) {
    static const uint32_t vga_colors[16] = {
        0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
        0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
        0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
        0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF
    };

    *fg = vga_colors[attr & 0x0F];
    *bg = vga_colors[(attr >> 4) & 0x0F];
}

void vga_txt_to_gfx(Rect area) {
    for (int y = 0; y < VGA_03_HEIGHT; y++) {
        for (int x = 0; x < VGA_03_WIDTH; x++) {
            int index = y * VGA_03_WIDTH + x;
            uint16_t entry = text_mode_memory[index];

            char c = entry & 0xFF;
            uint8_t attr = (entry >> 8) & 0xFF;

            uint32_t fg_color, bg_color;
            decode_vga_colors(attr, &fg_color, &bg_color);

            int pixel_x = area.x + x * 4;
            int pixel_y = area.y + y * 6;

            
            // Draw the character
            draw_bitmap_char(
                c,
                pixel_x,
                pixel_y,
                8,
                16,
                fg_color,
                font8x16,
                false,  
                false,  
                true    
            );
        }
    }
}