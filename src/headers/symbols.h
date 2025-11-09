#ifndef SYMBOLS_H
#define SYMBOLS_H

#include <stdint.h>

typedef struct {
    void *addr;     
    char type;      
    uint16_t str_len;  
    char str[];      
} __attribute__((__packed__)) BacktraceSymbol;

typedef enum {
    BacktraceSymbol_TEXT       = 'T',  // Global code (functions)
    BacktraceSymbol_LOCAL_TEXT = 't',  // Local/static functions
    BacktraceSymbol_DATA       = 'D',  // Global initialized data
    BacktraceSymbol_LOCAL_DATA = 'd',  // Local initialized data
    BacktraceSymbol_BSS        = 'B',  // Global uninitialized data
    BacktraceSymbol_LOCAL_BSS  = 'b',  // Local uninitialized data
    BacktraceSymbol_RODATA     = 'R',  // Global read-only data
    BacktraceSymbol_LOCAL_RO   = 'r',  // Local read-only data
    BacktraceSymbol_WEAK       = 'W',  // Weak symbol (global)
    BacktraceSymbol_LOCAL_WEAK = 'w'   // Weak symbol (local) 
} BacktraceSymbol_Type;

int Setup_Kernel_Syms();

BacktraceSymbol Get_Symbol(uintptr_t addr);

#endif // SYMBOLS_H

