#include "symbols.h"
#include "Logger.h"
#include "string.h"
#include <stdint.h>


#ifndef __NO_KSYMS
extern const uint8_t _syms_bin_start[];
extern const uint8_t _syms_bin_end[];
#else
const uint8_t _syms_bin;
const uint8_t* _syms_bin_start = &_syms_bin;
const uint8_t* _syms_bin_end = &_syms_bin;
#endif

static const uint8_t* k_syms_buffer = NULL;
static size_t sym_list_size = 0;


int Setup_Kernel_Syms() {
    Sys_Info("setting up kernel syms\n");
        
    
    k_syms_buffer = _syms_bin_start;
    sym_list_size = _syms_bin_end - _syms_bin_start;

#if KSYMS_DEBUG
    const uint8_t* current = k_syms_buffer;
    const uint8_t* end = k_syms_buffer + sym_list_size;

    Sys_color_log_NoPos("symbols list:\n", ANSI_BRIGHT_CYAN, ANSI_BG_BLACK);
    while (current + offsetof(BacktraceSymbol, str) <= end) {
        BacktraceSymbol* sym = (BacktraceSymbol*)current;

        // if (current + offsetof(BacktraceSymbol, str) + sym->str_len > end) break;

        Sys_color_log_NoPos("     func: %s          addr:0x%x\n", ANSI_BRIGHT_CYAN, ANSI_BG_BLACK, sym->str, (uintptr_t)sym->addr);
        
        current += offsetof(BacktraceSymbol, str) + sym->str_len;
    }
#endif

    return 0;
}


BacktraceSymbol* Get_Symbol(uintptr_t addr, void* buffer) {
    if (!k_syms_buffer || sym_list_size == 0) return NULL;

    const uint8_t* current = k_syms_buffer;
    const uint8_t* end = k_syms_buffer + sym_list_size;
    BacktraceSymbol* previous = NULL;

    while (current + offsetof(BacktraceSymbol, str) <= end) {
        BacktraceSymbol* sym = (BacktraceSymbol*)current;

        if (current + offsetof(BacktraceSymbol, str) + sym->str_len > end) break;

        if ((uint32_t)sym->addr > addr) {
            break; 
        }

        previous = sym;
        current += offsetof(BacktraceSymbol, str) + sym->str_len;
    }

    if (!previous) return NULL;

    
    BacktraceSymbol* res = buffer;
    if (!res) return NULL; 

    memcpy(res, previous, offsetof(BacktraceSymbol, str)); // copy struct fields except str
    res->str_len = previous->str_len;
    memcpy(res->str, previous->str, previous->str_len); // copy string

    return res;
}

