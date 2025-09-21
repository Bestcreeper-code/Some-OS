#include "../src/headers/io.h"
#include "../src/headers/video.h"
#include "../src/headers/Logger.h"
#include "../src/headers/memory.h"
#include "../src/headers/string.h"
#include "../src/headers/FileSystem.h"
#include "../src/headers/time.h"
#include "../src/headers/vga_modes.h"
#include "../src/headers/power.h"





static const char* crash_messages[] = {
    "Divide by Zero",                 // ISR 0
    "Debug Exception",               // ISR 1
    "Unknown Error",                 // ISR 2 - NMI (usually not a crash)
    "Breakpoint",                   // ISR 3
    "Overflow",                    // ISR 4
    "Bound Range Exceeded",         // ISR 5
    "Invalid Opcode",               // ISR 6
    "Device Not Available",         // ISR 7
    "Double Fault",                // ISR 8
    "Unknown Error",               // ISR 9 - Coprocessor Segment Overrun (obsolete)
    "Invalid TSS",                 // ISR 10
    "Segment Not Present",          // ISR 11
    "Stack Segment Fault",          // ISR 12
    "General Protection Fault",     // ISR 13
    "Page Fault",                  // ISR 14
    "Unknown Error",               // ISR 15 - Reserved
    "x87 Floating-Point Exception", // ISR 16
    "Alignment Check",              // ISR 17
    "Machine Check",               // ISR 18
    "SIMD Floating-Point Exception", // ISR 19
    "Virtualization Exception",     // ISR 20
    "Control Protection Exception", // ISR 21
    "Unknown Error",               // ISR 22 - Reserved
    "Unknown Error",               // ISR 23 - Reserved
    "Unknown Error",               // ISR 24 - Reserved
    "Unknown Error",               // ISR 25 - Reserved
    "Unknown Error",               // ISR 26 - Reserved
    "Unknown Error",               // ISR 27 - Reserved
    "Hypervisor Injection Exception", // ISR 28
    "VMM Communication Exception",  // ISR 29
    "Security Exception",           // ISR 30
    "Unknown Error"                // ISR 31 - Reserved
};

enum CrashType {
    CRASH_DIVIDE_BY_ZERO = 0,
    CRASH_DEBUG_EXCEPTION,
    CRASH_NMI,  // Non Maskable Interrupt (usually not a crash)
    CRASH_BREAKPOINT,
    CRASH_OVERFLOW,
    CRASH_BOUND_RANGE_EXCEEDED,
    CRASH_INVALID_OPCODE,
    CRASH_DEVICE_NOT_AVAILABLE,
    CRASH_DOUBLE_FAULT,
    CRASH_COPROCESSOR_SEGMENT_OVERRUN, // Obsolete
    CRASH_INVALID_TSS,
    CRASH_SEGMENT_NOT_PRESENT,
    CRASH_STACK_SEGMENT_FAULT,
    CRASH_GENERAL_PROTECTION,
    CRASH_PAGE_FAULT,
    CRASH_RESERVED_15,
    CRASH_X87_FPU_EXCEPTION,
    CRASH_ALIGNMENT_CHECK,
    CRASH_MACHINE_CHECK,
    CRASH_SIMD_FP_EXCEPTION,
    CRASH_VIRTUALIZATION_EXCEPTION,
    CRASH_CONTROL_PROTECTION_EXCEPTION,
    CRASH_RESERVED_22,
    CRASH_RESERVED_23,
    CRASH_RESERVED_24,
    CRASH_RESERVED_25,
    CRASH_RESERVED_26,
    CRASH_RESERVED_27,
    CRASH_HYPERVISOR_INJECTION,
    CRASH_VMM_COMMUNICATION,
    CRASH_SECURITY_EXCEPTION,
    CRASH_RESERVED_31,
    CRASH_CODES_AMOUNT
};

void app_main(int argc, char** argv) {
    int crash_code = 0; 
    if (argc <2) {
        Sys_log("Crash Handler:System crashed -> No crash info provided\n");
        crash_code = -2147483648;
    }
    char* error_code = "no_err_code";
    if(argc >=3 && strcmp(argv[2],"2147483648"))error_code = argv[2];
    Sys_log(" System crashed -> Crash info: %s\n", argv[1]);

    vga_set_mode(0X13);
    clear_13h_screen(0x4); 
    draw_bitmap_string("A critical error has occurred:", 20, 20, 4, 6, 0x3F, NULL, true,false, 0);

    if(crash_code != -2147483648)crash_code = atoi(argv[1]);
    char* error_name;
    if (crash_code >= 0 && crash_code < (sizeof(crash_messages) / sizeof(crash_messages[0]))){
        error_name = (char*)crash_messages[crash_code];
    } else error_name = "Unknown Crash";

    char full_str[128];
    sprintf(full_str, "Error Code: %s (%03d: %s)", error_name, crash_code, error_code);

    draw_bitmap_string(full_str, 20, 40, 4, 6, 0x3F, NULL, true,false, 0);
    
    reset_input_buffer();
    draw_bitmap_string("Rebooting in 10 sec...", 20, 60, 4, 6, 0x3F, NULL, true, false, 0);
    draw_bitmap_string("##########", 60, 80, 4, 6, 0x0, NULL, true, false, 0);
    for(int i=0;i<10;i++){
        draw_bitmap_char('#', 60+(4*i), 80, 4, 6, 0x2, NULL, true, true,false);
        sleep(1000);
    }
    pc_reboot();
}
