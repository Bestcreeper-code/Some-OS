#include "headers/Logger.h"

#include "headers/time.h"
#include "headers/memory.h"
#include "headers/asm.h"
#include "headers/io.h"
#include <stdbool.h>
#include <stdarg.h>

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
            serial_write_char('\r');
        }
        serial_write_char(*str++);
    }
}



void sys_serial_logf(const char* frmt, const char* file, const char* func, int line, ...) {
    va_list args;
    va_start(args, line);

    // Create the prefix: "<file:line(func)> "
    char prefix[128];
    int prefix_len = snprintf(prefix, sizeof(prefix), "<%s:%d(%s)> ", file, line, func);

    // Format the message body
    char msg[512];
    vsnprintf(msg, sizeof(msg), frmt, args);
    va_end(args);

    
    char output[640];  
    snprintf(output, sizeof(output), "%s%s", prefix, msg);

    serial_write_string(output);
}
