#include "../headers/Logger.h"

#include "../headers/string.h"
#include "../headers/asm.h"
#include "../headers/io.h"
#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

static uint8_t current_fg = ANSI_WHITE;
static uint8_t current_bg = ANSI_BG_BLACK;


void serial_init() {
	outb(COM1_PORT + 1, 0x00); // Disable interrupts
    outb(COM1_PORT + 3, 0x80); // Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03); // Set divisor to 3 (38400 baud)
    outb(COM1_PORT + 1, 0x00); //                  (high byte)
    outb(COM1_PORT + 3, 0x03); // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7); // Enable FIFO, clear them, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

int serial_is_transmit_ready() {
	return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
	while (!serial_is_transmit_ready());
    outb(COM1_PORT, c);
}

void serial_write_string(const char* str) {
	while (*str) {
		if (*str == '\n') {
			serial_set_color(ANSI_WHITE, ANSI_BG_BLACK);
			serial_write_char('\r');
			serial_set_color(current_fg, current_bg);
        }
        serial_write_char(*str++);
    }
}

void sys_serial_vlogf(const char* format, const char* file, const char* func, int line, va_list args) {
    char frmt[512];
    int size = strlen(format) < 512 ? strlen(format) : 511;
    strncpy(frmt, format, size);
    frmt[size] = '\0';

    char prefix[128];
    
    if (func != NULL && line != 0) {
        snprintf(prefix, sizeof(prefix), "< %s:%d(%s)> ", file, line, func);
    } else {
        snprintf(prefix, sizeof(prefix), "<%s> ", file);
    }

    char msg[512];
    vsnprintf(msg, sizeof(msg), frmt, args);

    char output[640];
    snprintf(output, sizeof(output), "%s%s", prefix, msg);

    serial_write_string(output);

    if (Get_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON) && Get_Kernel_Flag(KDATA_FLAG_PAGING_ON)) {
        printstr(output);
    }
}

	
void sys_serial_logf(const char* format, const char* file, const char* func, int line, ...) {
	va_list args;
    va_start(args, line);
    sys_serial_vlogf(format, file, func, line, args);
    va_end(args);
}


void serial_log_hex(const char* label, uint32_t val) {
	sys_serial_logf("%s: 0x%x\n", "", "", 0, label, val );
}




void serial_set_color(uint8_t fg, uint8_t bg) {
    serial_write_char(KEY_ESCAPE);
    serial_write_char('[');

    // foreground
    if (fg >= 100) {
        serial_write_char('0' + (fg / 100));
        serial_write_char('0' + ((fg / 10) % 10));
        serial_write_char('0' + (fg % 10));
    } else if (fg >= 10) {
        serial_write_char('0' + (fg / 10));
        serial_write_char('0' + (fg % 10));
    } else {
        serial_write_char('0' + fg);
    }

    serial_write_char(';');

    // background
    if (bg >= 100) {
        serial_write_char('0' + (bg / 100));
        serial_write_char('0' + ((bg / 10) % 10));
        serial_write_char('0' + (bg % 10));
    } else if (bg >= 10) {
        serial_write_char('0' + (bg / 10));
        serial_write_char('0' + (bg % 10));
    } else {
        serial_write_char('0' + bg);
    }

    serial_write_char('m');

    current_fg = fg;
    current_bg = bg;
}
void serial_set_fg(uint8_t fg) {
    serial_set_color(fg, current_bg);
}
void serial_set_bg(uint8_t bg) {
    serial_set_color(current_fg, bg);
}

static const uint8_t ansi_to_vga[16] = {
    0,  // black
    4,  // red
    2,  // green
    6,  // brown 
    1,  // blue
    5,  // magenta
    3,  // cyan
    7,  // white

    8,  // bright black
    12, // bright red
    10, // bright green
    14, // bright yellow
    9,  // bright blue
    13, // bright magenta
    11, // bright cyan
    15  // bright white
};

void sys_color_serial_logf(const char* format, uint8_t fg, uint8_t bg, const char* file, const char* func, int line, ...) {
    va_list args;
    va_start(args, line);

    uint8_t old_fg = current_fg;
    uint8_t old_bg = current_bg;

    int vga_color = ansi_to_vga[(fg >= 90) ? (fg - 90 + 8) : (fg - 30)];
    serial_set_color(fg, bg);
    set_print_color(vga_color);

    sys_serial_vlogf(format, file, func, line, args);

    // previous colors
    serial_set_color(old_fg, old_bg);
    set_print_color(0xF);

    va_end(args);
}
