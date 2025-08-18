// #include "headers/loader.h"
// #include "headers/addresses.h"
// #include "headers/FileSystem.h"
// #include "headers/memory.h"
// #include "headers/video.h"
// #include "headers/vga_modes.h"
// #include "headers/string.h"

// // extern uintptr_t setup_process_stack_asm(uint32_t esp,uintptr_t entry);

// void* new_process(char* path){
//     FIL file;
//     FRESULT fr;
//     void* res = NULL;

//     fr = f_open(&file, path, FA_READ);

//     if (fr == FR_OK) {
//         DWORD file_size = f_size(&file);  
//         Process_Table* table = (Process_Table*) PROCESS_TABLE;
//         for(int i = 0;i < MAX_PROCESSES;i++){
//             Process_Table_Entry process = table->entries[i];
//             if(process.name == NULL){
//                 //allocation of file+stack
//                 void* process_addr = malloc(file_size + PROCESS_STACK_SIZE);
//                 if(!process_addr)break;

//                 uintptr_t process_entry = (uintptr_t)(process_addr + PROCESS_STACK_SIZE);

//                 //loading the file
//                 uint32_t byte_read = 0;
//                 if (f_read(&file, (void*)process_entry, file_size, (UINT *)&byte_read) != FR_OK || byte_read != file_size) {
//                     free(process_addr);
//                     break;
//                 }
//                 // add to process table
//                 char* name = malloc(strlen(path) + 1);
//                 table->entries[i].name = name;
//                 strcpy(table->entries[i].name, path);
//                 table->entries[i].base = process_entry;
//                 table->entries[i].size = file_size;
//                 table->entries[i].stack_base = process_entry-1;
//                 table->entries[i].stack_end = process_entry-1;


                

//                 res = (void*)process_entry;
//                 break;
//             }
//         }
//         if (res == NULL){
//             printf("Too many processes running\n", fr);
//         }
        
//     } else {
//         printf("Failed to open file. Error: %d\n", fr);
//     }

//     f_close(&file);
//     return (void*)res;
// }

// void switch_process(uint32_t old_proc_esp) {
//     Process_Table* table = (Process_Table*) PROCESS_TABLE;
//     table->entries[table->current_process].stack_end = old_proc_esp;

//     vga_set_mode(0x03);
//     ClearScreen();
//     uint8_t current = 0;
//     while(true){
//         for (int i = 0; i < MAX_PROCESSES; i++)
//         {
//             if (i == current)set_print_color(14);
//             else set_print_color(15);
//             if (table->entries[i].name){
//                 printf("ID %d: %s\n",i,table->entries[i].name);
//             }
//             set_print_color(15);
//         }
//         uint8_t in = getc();
//         switch (in)
//         {
//         case KEY_DOWN:
//             if(current > 0)current--;
//             break;
//         case KEY_UP:
//             if(current < MAX_PROCESSES-1)current++;
//             break;
        
//         default:
//             break;
//         }
        

//     }
// }

// void clear_processes(){
//     Process_Table* table = (Process_Table*) PROCESS_TABLE;
//     for (int i = 0; i < 16; i++)
//     {
//         free(table->entries[i].name);
//     }
    
//     memset(table->entries,0,sizeof(table->entries));
//     table->current_process = 0;

//     // char* name = malloc(strlen("Kernel")+1);
//     // strcpy(name, "Kernel");
//     // table->entries[0].name = name;

//     // table->entries->stack_base = KERNEL_STACK_BASE;
//     // table->entries->stack_end = KERNEL_STACK_BASE;//gets overwritten when
// }
