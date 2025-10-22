#ifndef ADRESSES_H
#define ADRESSES_H

// #include "loader.h"
#include "video.h"
#include "paging.h"

#include <stdint.h>
#include <stddef.h>

#define INPUT_CHAR_BUFFER_SIZE  256
#define CONSOLE_REQUEST_QUEUE_SIZE 32

#define KERNEL_STACK_BASE 0xFFFFF
#define KERNEL_STACK_SIZE (32 * _PAGE_SIZE)


typedef struct __attribute__((packed)) {

    int16_t mouse_x_pos_prev;                         
    int16_t mouse_y_pos_prev;                        

    uint8_t mouse_flags_addr;                         

    int16_t mouse_y_pos_addr;                         
    int16_t mouse_x_pos_addr;                        

    char* console_request_queue[32];                
                                                    

    uint8_t task_switching_flag;                      
    uint8_t keyboard_mod_keys_flags;                 

    char input_char_buffer[256];                    

    uint8_t fatfs_sys_addr[0x234];                  

    uint32_t kernel_stack_base_store_address;        
    uint32_t kernel_stack_pointer_store_address;     

    uint8_t process_table[381];                      
                                                    

    uint32_t multiboot_info_address;               
                                                    

    uint64_t ticks_amount;                          
                                                   

    uint8_t free_region_map[0x804];                
                                                   
} KernelData_t;

extern KernelData_t kernel_data;



//fields access macros




#define MOUSE_PREV_BG                                    (kernel_data.mouse_prev_bg)    // 4x6=24 bytes (uint8_t[24])
#define MOUSE_X_POS_PREV                                 (kernel_data.mouse_x_pos_prev)    // int16_t
#define MOUSE_Y_POS_PREV                                 (kernel_data.mouse_y_pos_prev)    // int16_t
#define MOUSE_FLAGS                                      (kernel_data.mouse_flags_addr)    // uint8_t
#define MOUSE_Y_POS                                      (kernel_data.mouse_y_pos_addr)    // int16_t
#define MOUSE_X_POS                                      (kernel_data.mouse_x_pos_addr)    // int16_t
#define CONSOLE_REQUEST_QUEUE                            (kernel_data.console_request_queue)    // 32 char*(commands) that apps can request to the console on exit
#define TASK_SWITCHING_FLAG                              (kernel_data.task_switching_flag)    // activate/deactivate task switching between kernel and running app
#define KEYBOARD_MOD_KEYS_FLAGS                          (kernel_data.keyboard_mod_keys_flags)    // Modifier keys flags (shift,ctrl,...)
#define INPUT_CHAR_BUFFER_ADDRESS                        (kernel_data.input_char_buffer)    // Buffer for 256 input chars (start)
#define FATFS_SYS                                        (kernel_data.fatfs_sys_addr)    // FATFS system data (0x0234 bytes size)
#define KERNEL_STACK_BASE_STORE_ADDRESS                  (kernel_data.kernel_stack_base_store_address)    // uint32, address storing kernel EbP
#define KERNEL_STACK_POINTER_STORE_ADDRESS               (kernel_data.kernel_stack_pointer_store_address)    // uint32 | the address of the kernel ESP
#define PROCESS_TABLE                                    (kernel_data.process_table)    // 381 Bytes (Process_Table)
#define MULTIBOOT_INFO_ADDRESS                           (kernel_data.multiboot_info_address)    // uint32
#define TICKS_AMOUNT                                     (kernel_data.ticks_amount)    // uint64
#define FREE_REGION_MAP                                  (kernel_data.free_region_map)    // 2052 Bytes or 0x804 Bytes (free_region_map_t)




// typedef struct {
//     uintptr_t mb_info_addr;
//     uintptr_t tick_amount_ptr;
//     uintptr
// } __attribute__((packed)) Sys_Info_Struct; scrapped for now since somehow breaks the adresses

#endif // ADRESSES_H
