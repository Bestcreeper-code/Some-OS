#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/Logger.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/string.h"
#include "../../src/headers/FileSystem.h"
#include "../../src/headers/time.h"
#include "../../src/headers/vga_modes.h"
#include "../../src/headers/power.h"



#define REBOOT_TIMEOUT 6000

const char* crash_messages[] = {
    "Null Pointer Dereference",
    "Stack Overflow",
    "Division by Zero",
    "Invalid Opcode",
    "General Protection Fault",
    "Page Fault",
    "Double Fault",
    "Segment Not Present",
    "Alignment Check",
    "Machine Check",
    "SIMD Floating-Point Exception"
};

enum CrashType {
    CRASH_NULL_POINTER,
    CRASH_STACK_OVERFLOW,
    CRASH_DIV_BY_ZERO,
    CRASH_INVALID_OPCODE,
    CRASH_GENERAL_PROTECTION,
    CRASH_PAGE_FAULT,
    CRASH_DOUBLE_FAULT,
    CRASH_SEGMENT_NOT_PRESENT,
    CRASH_ALIGNMENT_CHECK,
    CRASH_MACHINE_CHECK,
    CRASH_SIMD_FP_EXCEPTION,
    CRASH_CODES_AMOUNT
};

void app_main(int argc, char** argv) {
    if (argc <2) Sys_log("Crash Handler:System crashed -> No crash info provided\n");
    Sys_log(" System crashed -> Crash info: %s\n", argv[1]);

    vga_set_mode(0X13);
    clear_13h_screen(0x4); 
    draw_bitmap_string("A critical error has occurred:", 20, 20, 4, 6, 0x3F, NULL, true, 0);

    int crash_code = atoi(argv[1]);
    char* error_name;
    if (crash_code >= 0 && crash_code < (sizeof(crash_messages) / sizeof(crash_messages[0]))){
        error_name = (char*)crash_messages[crash_code];
    } else error_name = "Unknown Crash";

    char full_str[70];
    sprintf(full_str, "Error Code: %s (%03d)", error_name, crash_code);

    draw_bitmap_string(full_str, 20, 40, 4, 6, 0x3F, NULL, true, 0);
    
    reset_input_buffer();
    draw_bitmap_string("Press Any Key To Reboot...", 20, 60, 4, 6, 0x3F, NULL, true, 0);
    while(!getc_nb()){
        sleep(10);
    }
    pc_reboot();
}
