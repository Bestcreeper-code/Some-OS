#include "headers/io.h"
#include "headers/string.h"
#include "headers/asm.h"

volatile uint16_t* video_memory = (volatile uint16_t*)0xB8000;
int vgaX = 0;
int vgaY = 0;

static char print_color = 0x0F; //white on black

void put_char(int x, int y,uint8_t c, uint8_t color) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return;
    video_memory[y * 80 + x] = (uint16_t)c | ((uint16_t)color << 8);
}

char get_char(int x, int y) {
    if (x < 0 || x >= 80 || y < 0 || y >= 25) return 0;
    return (char)(video_memory[y * 80 + x] & 0xFF);
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
            video_memory[i * 80 + j] = video_memory[(i + 1) * 80 + j];
        }
    }
    
    for (j = 0; j < 80; j++) {
        video_memory[24 * 80 + j] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    }
    
    
}

void ClearScreen() {
    move_cursor(0,0);
    for (int i = 0; i < 80*25; i++) {
        video_memory[i] = (uint16_t)' ' | ((uint16_t)0x0F << 8);
    }
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

bool shift_pressed = false;
bool caps_lock_on = false;
bool ctrl_pressed = false;
bool alt_pressed = false;
bool extended = false;


unsigned char scancode_to_char[128] = {
    0, 27, '&', 'e', '"', '\'', '(', '-', 'e', '_', 'c', 'a', ')', '=', '\b', '\t',
    'a', 'z', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\n', 0, 'q', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm', 'u', '`', 0, '*', 'w', 'x', 'c', 'v',
    'b', 'n', ',', ';', ':', '!', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned char scancode_to_char_shift[128] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '0', '+', '\b', '\t',
    'A', 'Z', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', 0, 0, '\n', 0, 'Q', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M', '%', '~', 0, '|', 'W', 'X', 'C', 'V',
    'B', 'N', '?', '.', '/', 0, 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
    '2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};


unsigned char GetInputChar() {
    unsigned char c = 0;
    while (c == 0) {
        uint8_t status;
        __asm__ __volatile__("inb $0x64, %0" : "=a"(status));
        if (!(status & 0x01)) continue;

        uint8_t scancode;
        __asm__ __volatile__("inb $0x60, %0" : "=a"(scancode));

        if (scancode == 0xE0) {
            extended = true;
            continue;
        }

        bool released = (scancode & 0x80) != 0;
        uint8_t keycode = scancode & 0x7F;

        if (extended) {
            switch (keycode) {
                case 0x48: c = KEY_UP; break; // KEY_UP
                case 0x50: c = KEY_DOWN; break; // KEY_DOWN
                case 0x4B: c = KEY_LEFT; break; // KEY_LEFT
                case 0x4D: c = KEY_RIGHT; break; // KEY_RIGHT
                case 0x47: c = KEY_HOME; break; // KEY_HOME
                case 0x1D: ctrl_pressed = !released; break;
                case 0x38: alt_pressed = !released; break;
                default:
                    // unknown extended code, ignore or add handling
                    break;
            }
            extended = false;
            if (released) continue;
            if (c != 0) return c;
        }

        // Handle modifier keys
        switch (keycode) {
            case 0x2A: // Left Shift
            case 0x36: // Right Shift
                shift_pressed = !released;
                break;
            case 0x3A: // Caps Lock
                if (!released) caps_lock_on = !caps_lock_on;
                break;
            case 0x1D: // Ctrl
                ctrl_pressed = !released;
                break;
            case 0x38: // Alt
                alt_pressed = !released;
                break;
        }
        if (released) continue;

        // Translate scancode to char
        unsigned char base_char = shift_pressed ? scancode_to_char_shift[keycode] : scancode_to_char[keycode];

        if (caps_lock_on && base_char >= 'a' && base_char <= 'z' && !shift_pressed)
            base_char -= 32;

        if (ctrl_pressed && base_char >= 'a' && base_char <= 'z')
            base_char = base_char - 'a' + 1;

        c = base_char;
    }
    return c;
}

unsigned char GetInputCharNonBlocking() {
    static bool extended = false;
    static bool shift_pressed = false;
    static bool ctrl_pressed = false;
    static bool alt_pressed = false;
    static bool caps_lock_on = false;

    uint8_t status;
    __asm__ __volatile__("inb $0x64, %0" : "=a"(status));
    if (!(status & 0x01)) {
        // No data available, return 0 immediately (no char)
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
    unsigned char c = 0;

    if (extended) {
        switch (keycode) {
            case 0x48: c = KEY_UP; break;    // KEY_UP
            case 0x50: c = KEY_DOWN; break;  // KEY_DOWN
            case 0x4B: c = KEY_LEFT; break;  // KEY_LEFT
            case 0x4D: c = KEY_RIGHT; break; // KEY_RIGHT
            case 0x47: c = KEY_HOME; break;  // KEY_HOME
            case 0x1D: ctrl_pressed = !released; break;
            case 0x38: alt_pressed = !released; break;
            default: break; // unknown extended
        }
        extended = false;
        if (released) return 0;
        return c;
    }

    // Modifier keys
    switch (keycode) {
        case 0x2A: // Left Shift
        case 0x36: // Right Shift
            shift_pressed = !released;
            return 0;
        case 0x3A: // Caps Lock
            if (!released) caps_lock_on = !caps_lock_on;
            return 0;
        case 0x1D: // Ctrl
            ctrl_pressed = !released;
            return 0;
        case 0x38: // Alt
            alt_pressed = !released;
            return 0;
    }
    if (released) return 0;

    // Translate scancode to char
    unsigned char base_char = shift_pressed ? scancode_to_char_shift[keycode] : scancode_to_char[keycode];

    if (caps_lock_on && base_char >= 'a' && base_char <= 'z' && !shift_pressed)
        base_char -= 32;

    if (ctrl_pressed && base_char >= 'a' && base_char <= 'z')
        base_char = base_char - 'a' + 1;

    return base_char;
}


String get_string_after_index(int start) {
    String string = { .length = 0 };
    int cursor_index = 0; // Cursor position inside string

    while (true) {
        unsigned char c = GetInputChar();

        if (c == 0x08) {  // Backspace
            if (cursor_index > 0) {
                // Shift characters left from cursor position
                for (int i = cursor_index - 1; i < string.length - 1; i++) {
                    string.buffer[i] = string.buffer[i + 1];
                }
                string.length--;
                cursor_index--;
            }

        } else if (c >= 32 && c <= 126) {  // Printable characters
            if (string.length < sizeof(string.buffer) - 1) {
                // Shift chars right to make space for new char
                for (int i = string.length; i > cursor_index; i--) {
                    string.buffer[i] = string.buffer[i - 1];
                }
                string.buffer[cursor_index] = c;
                string.length++;
                cursor_index++;
            }

        } else if (c == KEY_LEFT) {
            if (cursor_index > 0) {
                cursor_index--;
            }

        } else if (c == KEY_RIGHT) {
            if (cursor_index < string.length) {
                cursor_index++;
            }

        } else if (c == '\n') {
            // Null terminate the string before returning
            if (string.length < sizeof(string.buffer)) {
                string.buffer[string.length] = '\0';
            }
            return string;
        }

        // Update cursor position relative to start (prompt length)
        vgaX = start + cursor_index;
        move_cursor(vgaX, vgaY);
    }
}

String get_string() {
    return get_string_after_index(0);
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