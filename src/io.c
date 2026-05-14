#include "io.h"
#include "Logger.h"
#include "bootloader.h"
#include "string.h"
#include "asm.h"
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

int printstr(const char* str) {
    int char_count=0;
    while (*str) {
        if (*str == '\n') {
            vgaX = 0;
            vgaY++;
            if (vgaY >= K_TERMINAL_HEIGHT) {
                vgaY = K_TERMINAL_HEIGHT -1; 
                Scroll_Down();
            }
        } else {
            put_char(vgaX, vgaY, *str, print_color);
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
            if (vgaY >= K_TERMINAL_HEIGHT) {
                vgaY = K_TERMINAL_HEIGHT -1; 
                Scroll_Down();
            }
        } else {
            put_char(vgaX, vgaY, buffer[i], print_color);
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
    }
    move_cursor(vgaX, vgaY);
    return length;
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
    while(*str != '\0' && *str != '\n' && vgaX < K_TERMINAL_WIDTH) {
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

int write_uint32(char* buffer, int pos, uint32_t num, int size) {
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

int write_hex64(char* buffer, int pos, uint64_t num, int size) {
    char temp[17];
    int i = 0;

    if (num == 0)
        return write_char(buffer, pos, '0', size);

    while (num) {
        uint8_t nibble = num & 0xF;
        temp[i++] = (nibble < 10) ? ('0' + nibble) : ('a' + nibble - 10);
        num >>= 4;
    }

    int cur_pos = pos;
    for (int j = 0; j < i; j++) {
        if (buffer && (size == 0 || cur_pos < size - 1))
            buffer[cur_pos] = temp[i - j - 1];
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
int write_hex32_fixed_width(char* buffer, int pos, uint32_t val, int width, int uppercase) {
    char tmp[16];
    int len = 0;
    uint32_t v = val;

    do {
        int digit = v & 0xF;
        tmp[len++] = digit < 10 ? '0' + digit
            : (uppercase ? 'A' : 'a') + (digit - 10);
        v >>= 4;
    } while (v);

    if (len > width) len = width;

    while (len < width) tmp[len++] = '0';

    for (int i = len - 1; i >= 0; i--) buffer[pos++] = tmp[i];
    return len;
}

int write_float_sci(char* buffer, int pos, double val, int precision, int size, int uppercase) {
    int start = pos;

    if (val == 0.0) {
        pos += write_char(buffer, pos, '0', size);
        if (precision > 0) {
            pos += write_char(buffer, pos, '.', size);
            for (int i = 0; i < precision; i++)
                pos += write_char(buffer, pos, '0', size);
        }
        pos += write_char(buffer, pos, uppercase ? 'E' : 'e', size);
        pos += write_char(buffer, pos, '+', size);
        pos += write_char(buffer, pos, '0', size);
        pos += write_char(buffer, pos, '0', size);
        return pos - start;
    }

    if (val < 0.0) {
        pos += write_char(buffer, pos, '-', size);
        val = -val;
    }

    int exp = 0;
    while (val >= 10.0) {
        val /= 10.0;
        exp++;
    }
    while (val < 1.0) {
        val *= 10.0;
        exp--;
    }

    double rounding = 0.5;
    for (int i = 0; i < precision; i++) rounding /= 10.0;
    val += rounding;

    if (val >= 10.0) {
        val /= 10.0;
        exp++;
    }

    int first = (int)val;
    pos += write_char(buffer, pos, '0' + first, size);
    val -= first;

    if (precision > 0) {
        pos += write_char(buffer, pos, '.', size);
        for (int i = 0; i < precision; i++) {
            val *= 10.0;
            int digit = (int)val;
            pos += write_char(buffer, pos, '0' + digit, size);
            val -= digit;
        }
    }

    pos += write_char(buffer, pos, uppercase ? 'E' : 'e', size);

    if (exp >= 0) {
        pos += write_char(buffer, pos, '+', size);
    } else {
        pos += write_char(buffer, pos, '-', size);
        exp = -exp;
    }

    if (exp < 10) {
        pos += write_char(buffer, pos, '0', size);
        pos += write_char(buffer, pos, '0' + exp, size);
    } else {
        pos += write_uint32(buffer, pos, exp, size);
    }

    return pos - start;
}

int vsnprintf(char* buffer, int size, const char* format, va_list args) {
    int pos = 0;

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == '\0') break;

            
            if (*format == '0') {
                format++;
                int width = 0;
                while (*format >= '0' && *format <= '9') {
                    width = width * 10 + (*format - '0');
                    format++;
                }
                if (*format == 'd' || *format == 'D') {
                    int val = va_arg(args, int);
                    pos += write_number_fixed_width(buffer, pos, val, width, size);
                    format++;
                    continue;
                } else if (*format == 'x' || *format == 'X') {
                    int val = va_arg(args, int);
                    pos += write_hex32_fixed_width(buffer, pos, val, width, true);
                    format++;
                    continue;
                }
                
                else {
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
                    pos += write_uint32(buffer, pos, val, size);
                    break;
                }
                case 'x':case 'p':
                case 'X': {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_hex32(buffer, pos, val, size);
                    break;
                }
                case 'l': {  
                    
                    if (*(format + 1) == 'l' && (*(format + 2) == 'x' || *(format + 2) == 'X')) {
                        uint64_t val = va_arg(args, uint64_t);
                        int uppercase = (*(format + 2) == 'X');
                        pos += write_hex64(buffer, pos, val, uppercase);
                        format += 2; 
                    } else {
                        pos += write_char(buffer, pos, '%', 0);
                        pos += write_char(buffer, pos, *format, 0);
                    }
                    break;
                }
                case 'e': case 'E': {
                    double val = va_arg(args, double);
                    pos += write_float_sci(buffer, pos, val, 6, size, *format == 'E');
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

    return pos;
}



int vsprintf(char* buffer, const char* format, va_list args) {
    int pos = 0;

    while (*format) {
        if (*format == '%') {
            format++;
            if (*format == '\0') break;

            
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
                } else if (*format == 'x' || *format == 'X') {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_hex32_fixed_width(buffer, pos, val, width, *format == 'X');
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
                    pos += write_uint32(buffer, pos, val, 0);
                    break;
                }
                case 'x':
                case 'X':
                case 'p': {
                    uint32_t val = va_arg(args, uint32_t);
                    pos += write_hex32(buffer, pos, val, 0);
                    break;
                }
                case 'l': {  
                    
                    if (*(format + 1) == 'l' && (*(format + 2) == 'x' || *(format + 2) == 'X')) {
                        uint64_t val = va_arg(args, uint64_t);
                        int uppercase = (*(format + 2) == 'X');
                        pos += write_hex64(buffer, pos, val, uppercase);
                        format += 2; 
                    } else {
                        pos += write_char(buffer, pos, '%', 0);
                        pos += write_char(buffer, pos, *format, 0);
                    }
                    break;
                }
                case 'e': case 'E': {
                    double val = va_arg(args, double);
                    pos += write_float_sci(buffer, pos, val, 6, 0, *format == 'E');
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

    return pos;
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




