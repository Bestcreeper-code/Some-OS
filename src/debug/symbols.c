#include "headers/symbols.h"
#include "headers/memory.h"
#include "headers/string.h"


BacktraceSymbol* k_syms_buffer = 0;
uint32_t sym_list_size = 0;


int Setup_Kernel_Syms(BacktraceSymbol* buffer, size_t size){
    k_syms_buffer = malloc(size);
    
    memcpy(k_syms_buffer, buffer, size);
}











BacktraceSymbol Get_Symbol(uintptr_t addr){
    char* current_sym = k_syms_buffer;
    BacktraceSymbol* previous_sym = 0;
    
    for(int i=0;i<sym_list_size;i++){
        if(((BacktraceSymbol*)current_sym)->addr > addr){
            if(!previous_sym)break;
            return *previous_sym;
        }
        previous_sym = (BacktraceSymbol*)current_sym;

        current_sym += ((BacktraceSymbol*)current_sym)->str_len + offsetof(BacktraceSymbol,str);
    }

    return (BacktraceSymbol){0};
}