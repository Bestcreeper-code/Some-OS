#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>


#define COM1_PORT 0x3F8  

#define Sys_log(frmt , ...) sys_serial_logf(frmt, __FILE_NAME__, __func__, __LINE__ ,##__VA_ARGS__)

#define Sys_Breakpoint() Sys_log("Breakpoint hit at %s:%d\n", __FILE_NAME__, __LINE__); for(;;);

void serial_init();


void serial_write_char(char c);
void serial_write_string(const char* str);
void serial_log_hex(const char* label, uint32_t val);
void sys_serial_logf(const char* frmt, const char* file, const char* func, int line, ...) __attribute__ ((format (printf, 1, 5)));


#endif // DEBUG_H
