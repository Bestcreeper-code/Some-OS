#include "headers/symbols.h"
#include "headers/memory.h"
#include "headers/string.h"

// Embedded syms.bin
extern const uint8_t _binary_syms_bin_start[];
extern const uint8_t _binary_syms_bin_end[];

static const uint8_t* k_syms_buffer = NULL;
static size_t sym_list_size = 0;

int Setup_Kernel_Syms() {
    k_syms_buffer = _binary_syms_bin_start;
    sym_list_size = _binary_syms_bin_end - _binary_syms_bin_start;
    return 0;
}


BacktraceSymbol* Get_Symbol(uintptr_t addr) {
    if (!k_syms_buffer || sym_list_size == 0) return NULL;

    const uint8_t* current = k_syms_buffer;
    const uint8_t* end = k_syms_buffer + sym_list_size;
    BacktraceSymbol* previous = NULL;

    while (current + offsetof(BacktraceSymbol, str) <= end) {
        BacktraceSymbol* sym = (BacktraceSymbol*)current;

        if (current + offsetof(BacktraceSymbol, str) + sym->str_len > end) break;

        if (sym->addr > addr) {
            break; // found the symbol that is just after addr
        }

        previous = sym;
        current += offsetof(BacktraceSymbol, str) + sym->str_len;
    }

    if (!previous) return NULL;

    // Allocate a new BacktraceSymbol and copy it
    BacktraceSymbol* res = malloc(sizeof(BacktraceSymbol) + previous->str_len);
    if (!res) return NULL; // malloc failed

    memcpy(res, previous, offsetof(BacktraceSymbol, str)); // copy struct fields except str
    res->str_len = previous->str_len;
    memcpy(res->str, previous->str, previous->str_len); // copy string

    return res;
}

