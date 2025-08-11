#include "headers/loader.h"
#include "headers/addresses.h"
#include "headers/FileSystem.h"
#include "headers/memory.h"
#include "headers/video.h"
#include "headers/vga_modes.h"


void* new_process(char* path){
    FIL file;
    FRESULT fr;
    void* res = NULL;

    fr = f_open(&file, path, FA_READ);

    if (fr == FR_OK) {
        DWORD file_size = f_size(&file);  
        Process_Table* table = (Process_Table*) PROCESS_TABLE;
        for(int i = 0;i < MAX_PROCESSES;i++){
            Process_Table_Entry process = table->entries[i];
            if(process.base == 0){
                //allocation of file+stack
                void* process_addr = malloc(file_size + PROCESS_STACK_SIZE);
                if(!process_addr)break;

                uintptr_t process_entry = (uintptr_t)(process_addr + PROCESS_STACK_SIZE);

                //loading the file
                uint32_t byte_read = 0;
                if (f_read(&file, (void*)process_entry, file_size, (UINT *)&byte_read) != FR_OK || byte_read != file_size) {
                    free(process_addr);
                    break;
                }
                // add to process table
                table->entries[i].name = path;
                table->entries[i].base = process_entry;
                table->entries[i].size = file_size;
                table->entries[i].stack_base = process_entry-1;
                table->entries[i].stack_end = (uint32_t)process_addr;


                res = (void*)process_entry;
                break;
            }
        }
        if (res == NULL){
            printf("Too many processes running\n", fr);
        }
        
    } else {
        printf("Failed to open file. Error: %d\n", fr);
    }

    f_close(&file);
    return (void*)res;
}

void switch_process(uint32_t old_proc_esp){
    Process_Table* table = (Process_Table*) PROCESS_TABLE;
    table->entries[table->current_process].stack_end = old_proc_esp;

    vga_set_mode(0x03);
}