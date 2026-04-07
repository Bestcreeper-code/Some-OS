#ifndef DEBUG_H
#define DEBUG_H

#include <stdbool.h>
#include <stdarg.h>
#include <stdint.h>

#include "config.h"


#define COM1_PORT 0x3F8  
    

#define Sys_log_Pos(frmt , ...) sys_color_serial_logf(frmt, ANSI_WHITE, ANSI_BG_BLACK, __FILE_NAME__, __func__, __LINE__ ,##__VA_ARGS__)
#define Sys_color_log_Pos(frmt, fg_color, bg_color, ...) sys_color_serial_logf(frmt, fg_color, bg_color, __FILE_NAME__, __func__, __LINE__ ,##__VA_ARGS__)

#define Sys_log_NoPos(frmt , ...) sys_color_serial_logf(frmt, ANSI_WHITE, ANSI_BG_BLACK, NULL, NULL, 0, ##__VA_ARGS__)
#define Sys_color_log_NoPos(frmt, fg_color, bg_color, ...) sys_color_serial_logf(frmt, fg_color, bg_color, NULL, NULL, 0,##__VA_ARGS__)


#if POS_DEBUG_LOGS
#define Sys_log(frmt , ...) Sys_log_Pos(frmt, ##__VA_ARGS__)
#define Sys_color_log(frmt, fg_color, bg_color, ...) Sys_color_log_Pos(frmt, ##__VA_ARGS__)

#define Sys_Error(frmt , ...) Sys_color_log_Pos(frmt,ANSI_BLACK, ANSI_BG_BRIGHT_RED ,##__VA_ARGS__)
#define Sys_Success(frmt , ...) Sys_color_log_Pos(frmt, ANSI_BRIGHT_GREEN, ANSI_BG_BLACK ,##__VA_ARGS__)
#define Sys_Warning(frmt , ...) Sys_color_log_Pos(frmt,ANSI_WHITE, ANSI_BG_BRIGHT_YELLOW ,##__VA_ARGS__)

#else
#define Sys_log(frmt , ...) Sys_log_NoPos(frmt, ##__VA_ARGS__)
#define Sys_color_log(frmt, fg_color, bg_color, ...) Sys_color_log_NoPos(frmt, ##__VA_ARGS__)

#define Sys_Error(frmt , ...) Sys_color_log_NoPos(frmt,ANSI_BLACK, ANSI_BG_BRIGHT_RED ,##__VA_ARGS__)
#define Sys_Success(frmt , ...) Sys_color_log_NoPos(frmt, ANSI_BRIGHT_GREEN, ANSI_BG_BLACK ,##__VA_ARGS__)
#define Sys_Warning(frmt , ...) Sys_color_log_NoPos(frmt,ANSI_WHITE, ANSI_BG_BRIGHT_YELLOW ,##__VA_ARGS__)

#endif



#define Sys_Info(frmt , ...) Sys_log(frmt, ##__VA_ARGS__)




#define Sys_Breakpoint() Sys_log("Breakpoint hit at %s:%d\n", __FILE_NAME__, __LINE__); for(;;);
#define Sys_Step_Point() Sys_log("Step Point hit at %s:%d\n", __FILE_NAME__, __LINE__); getc();

void serial_init();


void serial_write_char(char c);
void serial_write_string(const char* str);
void serial_log_hex(const char* label, uint32_t val);

void sys_serial_logf(const char* frmt, const char* file, const char* func, int line, ...) __attribute__ ((format (printf, 1, 5)));
void sys_color_serial_logf(const char* format, uint8_t fg, uint8_t bg, const char* file, const char* func, int line, ...) __attribute__((format(printf, 1, 7)));

typedef enum {
    
    ANSI_RESET              = 0,

    
    ANSI_BLACK              = 30,
    ANSI_RED                = 31,
    ANSI_GREEN              = 32,
    ANSI_YELLOW             = 33,
    ANSI_BLUE               = 34,
    ANSI_MAGENTA            = 35,
    ANSI_CYAN               = 36,
    ANSI_WHITE              = 37,

    
    ANSI_BRIGHT_BLACK       = 90,
    ANSI_BRIGHT_RED         = 91,
    ANSI_BRIGHT_GREEN       = 92,
    ANSI_BRIGHT_YELLOW      = 93,
    ANSI_BRIGHT_BLUE        = 94,
    ANSI_BRIGHT_MAGENTA     = 95,
    ANSI_BRIGHT_CYAN        = 96,
    ANSI_BRIGHT_WHITE       = 97,

    
    ANSI_BG_BLACK           = 40,
    ANSI_BG_RED             = 41,
    ANSI_BG_GREEN           = 42,
    ANSI_BG_YELLOW          = 43,
    ANSI_BG_BLUE            = 44,
    ANSI_BG_MAGENTA         = 45,
    ANSI_BG_CYAN            = 46,
    ANSI_BG_WHITE           = 47,

    
    ANSI_BG_BRIGHT_BLACK    = 100,
    ANSI_BG_BRIGHT_RED      = 101,
    ANSI_BG_BRIGHT_GREEN    = 102,
    ANSI_BG_BRIGHT_YELLOW   = 103,
    ANSI_BG_BRIGHT_BLUE     = 104,
    ANSI_BG_BRIGHT_MAGENTA  = 105,
    ANSI_BG_BRIGHT_CYAN     = 106,
    ANSI_BG_BRIGHT_WHITE    = 107
} ansi_color_t;


void serial_set_color(uint8_t fg, uint8_t bg);
void serial_set_fg(uint8_t fg);
void serial_set_bg(uint8_t bg);

#endif // DEBUG_H
