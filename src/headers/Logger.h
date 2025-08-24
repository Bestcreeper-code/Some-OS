#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdarg.h>


#define COM1_PORT 0x3F8  

#define Kern_log(frmt, ...) k_serial_logf(frmt,__FILE__, __func__, __LINE__ ,##__VA_ARGS__)

void serial_init();


void serial_write_char(char c);
void serial_write_string(const char* str);
void k_serial_logf(const char* frmt, const char* file, const char* func, int line, ...);



#endif // DEBUG_H
