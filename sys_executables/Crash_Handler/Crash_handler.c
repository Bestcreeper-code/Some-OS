#include "../../src/headers/io.h"
#include "../../src/headers/video.h"
#include "../../src/headers/Logger.h"
#include "../../src/headers/memory.h"
#include "../../src/headers/string.h"
#include "../../src/headers/FileSystem.h"
#include "../../src/headers/time.h"
#include "../../src/headers/vga_modes.h"
#include "../../src/headers/power.h"
#include "../../src/headers/asm.h"





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
cpu_registers_t* gp_regs;

void app_main(int argc, uint32_t* argv) {
    graph_mode_fb = (volatile uint32_t*)(uint32_t)Multiboot_info->framebuffer_addr;
    asm volatile("sti");//incase
    
    uint32_t* call_stack = NULL;
    if (argc >= 4) {
        call_stack = (uint32_t*)argv[3];
    }
    
    
    if (argc < 3) {
        Sys_log("Crash Handler: Not enough crash info provided\n");
        // handle error or early return
        return;
    }

    int isr_index = (int)argv[0];
    uint32_t err_code = argv[1];

    char* error_name;
    if (isr_index >= 0 && isr_index < (int)(sizeof(crash_messages) / sizeof(crash_messages[0]))) {
        error_name = (char*)crash_messages[isr_index];
    } else {
        error_name = "Unknown Crash";
    }

    gp_regs = (cpu_registers_t*)argv[2];
    //padding
    Sys_log("\n");Sys_log("\n");Sys_log("\n");Sys_log("\n");Sys_log("\n");
    Sys_log("=======================================================================");

    Sys_log("System crashed -> ISR Index: %d(%s), Error Code: %u\n", isr_index, error_name, err_code);

    draw_bitmap_string("A critical error has occurred:", 20, 20, 4, 6, 0x3F, NULL, true, false, 0);


    char full_str[128];
    sprintf(full_str, "Error Code: %s (%03d: %u)", error_name, isr_index, err_code);

    draw_bitmap_string(full_str, 20, 40, 4, 6, 0x3F, NULL, true, false, 0);

    // =====================Regs dump===============
    draw_bitmap_string("Regs Dump:", 0, 60, 4, 6, 0x3F, NULL, true, false, 0);
    Sys_log("Regs Dump:\n");
    char buf[128];
    
    sprintf(buf, "EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x", 
            gp_regs->eax, gp_regs->ebx, gp_regs->ecx, gp_regs->edx);
    Sys_log("%s\n",buf);
    draw_bitmap_string(buf, 50, 60, 4, 6, 0x3F, NULL, true, false, 0);

    sprintf(buf, "ESI: 0x%x  EDI: 0x%x  EBP: 0x%x  ESP: 0x%x", 
            gp_regs->esi, gp_regs->edi, gp_regs->ebp, gp_regs->esp);
    Sys_log("%s\n",buf);
    draw_bitmap_string(buf, 50, 80, 4, 6, 0x3F, NULL, true, false, 0);

    sprintf(buf, "EIP: 0x%x  EFLAGS: 0x%x", 
            gp_regs->eip, gp_regs->eflags);
    Sys_log("%s\n",buf);
    draw_bitmap_string(buf, 50, 100, 4, 6, 0x3F, NULL, true, false, 0);
    
    if (call_stack) {
        Sys_log("Call stack trace:\n");
        draw_bitmap_string("Call Stack Trace:", 0, 140, 4, 6, 0x3F, NULL, true, false, 0);
        
        char buf[128];
        for (int i = 0; i < 8; i++) {
            sprintf(buf, "0x%x", call_stack[i]);
            Sys_log("%s\n", buf);
            draw_bitmap_string(buf, 20, 160 + i * 20, 4, 6, 0x3F, NULL, true, false, 0);
        }
    }

    
    draw_bitmap_string("Rebooting in 10 sec...", 20, 400, 4, 6, 0x3F, NULL, true, false, 0);
    draw_bitmap_string("##########", 60, 420, 4, 6, 0x0, NULL, true, false, 0);

    for (int i = 0; i < 10; i++) {
        draw_bitmap_char('#', 60 + (4 * i), 420, 4, 6, 0x2, NULL, true, true, false);
        sleep(1000);
    }
    pc_reboot();
}

void _start(int argc, uint32_t* argv){
    app_main(argc, argv);
}