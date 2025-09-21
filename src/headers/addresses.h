#ifndef ADRESSES_H
#define ADRESSES_H

// #include "loader.h"
#include "video.h"

#include <stdint.h>
#include <stddef.h>

#define INPUT_CHAR_BUFFER_SIZE  256
#define CONSOLE_REQUEST_QUEUE_SIZE 32

// #define KERNEL_STACK_BASE 0x101FFF//      |
// #define KERNEL_STACK_SIZE 0x1FFF //8Kb    | - goes from 0x101FFF to 0x100000

//field addresses
#define MODE13H_COLOR_PALETTE_SIZE_ADDR          (KERNEL_DATA_START + offsetof(KernelData_t, mode13h_color_palette_size))
#define MODE13H_COLOR_PALETTE_ADDR               (KERNEL_DATA_START + offsetof(KernelData_t, mode13h_color_palette))

#define MOUSE_PREV_BG_ADDR                       (KERNEL_DATA_START + offsetof(KernelData_t, mouse_prev_bg))
#define MOUSE_X_POS_PREV_ADDR                    (KERNEL_DATA_START + offsetof(KernelData_t, mouse_x_pos_prev))
#define MOUSE_Y_POS_PREV_ADDR                    (KERNEL_DATA_START + offsetof(KernelData_t, mouse_y_pos_prev))
#define MOUSE_FLAGS_ADDR                         (KERNEL_DATA_START + offsetof(KernelData_t, mouse_flags_addr))
#define MOUSE_Y_POS_ADDR                         (KERNEL_DATA_START + offsetof(KernelData_t, mouse_y_pos_addr))
#define MOUSE_X_POS_ADDR                         (KERNEL_DATA_START + offsetof(KernelData_t, mouse_x_pos_addr))

#define CONSOLE_REQUEST_QUEUE_ADDR               (KERNEL_DATA_START + offsetof(KernelData_t, console_request_queue))
#define TASK_SWITCHING_FLAG_ADDR                 (KERNEL_DATA_START + offsetof(KernelData_t, task_switching_flag))
#define KEYBOARD_MOD_KEYS_FLAGS_ADDR             (KERNEL_DATA_START + offsetof(KernelData_t, keyboard_mod_keys_flags))

#define INPUT_CHAR_BUFFER_ADDRESS_ADDR           (KERNEL_DATA_START + offsetof(KernelData_t, input_char_buffer))
#define FATFS_SYS_ADDR                           (KERNEL_DATA_START + offsetof(KernelData_t, fatfs_sys_addr))

#define KERNEL_STACK_BASE_STORE_ADDRESS_ADDR     (KERNEL_DATA_START + offsetof(KernelData_t, kernel_stack_base_store_address))
#define KERNEL_STACK_POINTER_STORE_ADDRESS_ADDR  (KERNEL_DATA_START + offsetof(KernelData_t, kernel_stack_pointer_store_address))

#define PROCESS_TABLE_ADDR                       (KERNEL_DATA_START + offsetof(KernelData_t, process_table))
#define MULTIBOOT_INFO_ADDRESS_ADDR              (KERNEL_DATA_START + offsetof(KernelData_t, multiboot_info_address))
#define TICKS_AMOUNT_ADDR                        (KERNEL_DATA_START + offsetof(KernelData_t, ticks_amount))
#define FREE_REGION_MAP_ADDR                     (KERNEL_DATA_START + offsetof(KernelData_t, free_region_map))


//fields access macros
#define KERNEL_DATA_START 0x1E6D



#define MODE13H_COLOR_PALETTE_SIZE                       ((KernelData_t*)KERNEL_DATA_START)->mode13h_color_palette_size    // 
#define MODE13H_COLOR_PALETTE                            ((KernelData_t*)KERNEL_DATA_START)->mode13h_color_palette    // the color palette of mode 13h(256 entries of 3 bytes each)
#define MOUSE_PREV_BG                                    ((KernelData_t*)KERNEL_DATA_START)->mouse_prev_bg    // 4x6=24 bytes (uint8_t[24])
#define MOUSE_X_POS_PREV                                 ((KernelData_t*)KERNEL_DATA_START)->mouse_x_pos_prev    // int16_t
#define MOUSE_Y_POS_PREV                                 ((KernelData_t*)KERNEL_DATA_START)->mouse_y_pos_prev    // int16_t
#define MOUSE_FLAGS                                      ((KernelData_t*)KERNEL_DATA_START)->mouse_flags_addr    // uint8_t
#define MOUSE_Y_POS                                      ((KernelData_t*)KERNEL_DATA_START)->mouse_y_pos_addr    // int16_t
#define MOUSE_X_POS                                      ((KernelData_t*)KERNEL_DATA_START)->mouse_x_pos_addr    // int16_t
#define CONSOLE_REQUEST_QUEUE                            ((KernelData_t*)KERNEL_DATA_START)->console_request_queue    // 32 char*(commands) that apps can request to the console on exit
#define TASK_SWITCHING_FLAG                              ((KernelData_t*)KERNEL_DATA_START)->task_switching_flag    // activate/deactivate task switching between kernel and running app
#define KEYBOARD_MOD_KEYS_FLAGS                          ((KernelData_t*)KERNEL_DATA_START)->keyboard_mod_keys_flags    // Modifier keys flags (shift,ctrl,...)
#define INPUT_CHAR_BUFFER_ADDRESS                        ((KernelData_t*)KERNEL_DATA_START)->input_char_buffer    // Buffer for 256 input chars (start)
#define FATFS_SYS                                        ((KernelData_t*)KERNEL_DATA_START)->fatfs_sys_addr    // FATFS system data (0x0234 bytes size)
#define KERNEL_STACK_BASE_STORE_ADDRESS                  ((KernelData_t*)KERNEL_DATA_START)->kernel_stack_base_store_address    // uint32, address storing kernel EbP
#define KERNEL_STACK_POINTER_STORE_ADDRESS               ((KernelData_t*)KERNEL_DATA_START)->kernel_stack_pointer_store_address    // uint32 | the address of the kernel ESP
#define PROCESS_TABLE                                    ((KernelData_t*)KERNEL_DATA_START)->process_table    // 381 Bytes (Process_Table)
#define MULTIBOOT_INFO_ADDRESS                           ((KernelData_t*)KERNEL_DATA_START)->multiboot_info_address    // uint32
#define TICKS_AMOUNT                                     ((KernelData_t*)KERNEL_DATA_START)->ticks_amount    // uint64
#define FREE_REGION_MAP                                  ((KernelData_t*)KERNEL_DATA_START)->free_region_map    // 2052 Bytes or 0x804 Bytes (free_region_map_t)

#define KERNEL_DATA_END   0x2EFD


typedef struct __attribute__((packed)) {
    char mode13h_color_palette_size;                  // 0x0000

    RGBColor mode13h_color_palette[256];              // 0x0001 
                                                     // ends at 0x0301

    uint8_t mouse_prev_bg[256];                       // 0x0301
                                                     // ends at 0x0401

    int16_t mouse_x_pos_prev;                         // 0x0401
    int16_t mouse_y_pos_prev;                         // 0x0403

    uint8_t mouse_flags_addr;                         // 0x0405

    int16_t mouse_y_pos_addr;                         // 0x0406
    int16_t mouse_x_pos_addr;                         // 0x0408

    char* console_request_queue[32];                  // 0x040A (32 * 4 = 128 bytes)
                                                     // ends at 0x048A

    uint8_t task_switching_flag;                      // 0x048A
    uint8_t keyboard_mod_keys_flags;                  // 0x048B

    char input_char_buffer[256];                      // 0x048C
                                                     // ends at 0x058C

    uint8_t fatfs_sys_addr[0x234];                    // 0x058C
                                                     // ends at 0x07C0

    uint32_t kernel_stack_base_store_address;         // 0x07C0
    uint32_t kernel_stack_pointer_store_address;      // 0x07C4

    uint8_t process_table[381];                       // 0x07C8
                                                     // ends at 0x093D

    uint32_t multiboot_info_address;                  // 0x093D
                                                     // 0x0941

    uint64_t ticks_amount;                            // 0x0941
                                                     // 0x0949

    uint8_t free_region_map[0x804];                   // 0x0949
                                                     // ends at 0x114D
} KernelData_t;

// typedef struct {
//     uintptr_t mb_info_addr;
//     uintptr_t tick_amount_ptr;
//     uintptr
// } __attribute__((packed)) Sys_Info_Struct; scrapped for now since somehow breaks the adresses

#endif // ADRESSES_H
