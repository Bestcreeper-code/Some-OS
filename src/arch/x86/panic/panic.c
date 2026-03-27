#include "io.h"
#include "video.h"
#include "Logger.h"
#include "memory.h"
#include "string.h"
#include "FileSystem.h"
#include "time.h"

#include "power.h"
#include "arch_asm.h"
#include "panic.h"
#include "symbols.h"
#include "scheduler.h"
#include "../data/textconsts.h"
#include "kernel_data.h"
#include <assert.h>
#include <stdint.h>
// #include "../../distorm/include/distorm.h"



#define MAX_KPANIK_COUNT 1

// CPU Exceptions
static const char* crash_messages[] = {
    "Divide by Zero",                    // 0
    "Debug Exception",                   // 1
    "NMI",                               // 2
    "Breakpoint",                        // 3
    "Overflow",                          // 4
    "Bound Range Exceeded",              // 5
    "Invalid Opcode",                    // 6
    "Device Not Available",              // 7
    "Double Fault",                      // 8
    "Coprocessor Segment Overrun",       // 9
    "Invalid TSS",                       // 10
    "Segment Not Present",               // 11
    "Stack Segment Fault",               // 12
    "General Protection Fault",          // 13
    "Page Fault",                        // 14
    "Reserved",                          // 15

    // x87 / SIMD / CPU-specific
    "x87 Floating-Point Exception",      // 16
    "Alignment Check",                   // 17
    "Machine Check",                     // 18
    "SIMD Floating-Point Exception",     // 19
    "Virtualization Exception",          // 20
    "Control Protection Exception",      // 21

    // Reserved / Unknown
    "Reserved",                          // 22
    "Reserved",                          // 23
    "Reserved",                          // 24
    "Reserved",                          // 25
    "Reserved",                          // 26
    "Reserved",                          // 27

    // Hypervisor
    "Hypervisor Injection Exception",    // 28
    "VMM Communication Exception",       // 29

    // Security / Other
    "Security Exception",                // 30
    "Reserved"                           // 31
};


enum CrashType {
    // CPU Exceptions
    CRASH_DIVIDE_BY_ZERO = 0,           // 0
    CRASH_DEBUG_EXCEPTION,              // 1
    CRASH_NMI,                          // 2 Non-Maskable Interrupt
    CRASH_BREAKPOINT,                   // 3
    CRASH_OVERFLOW,                     // 4
    CRASH_BOUND_RANGE_EXCEEDED,         // 5
    CRASH_INVALID_OPCODE,               // 6
    CRASH_DEVICE_NOT_AVAILABLE,         // 7
    CRASH_DOUBLE_FAULT,                 // 8
    CRASH_COPROCESSOR_SEGMENT_OVERRUN, // 9 Obsolete
    CRASH_INVALID_TSS,                  // 10
    CRASH_SEGMENT_NOT_PRESENT,          // 11
    CRASH_STACK_SEGMENT_FAULT,          // 12
    CRASH_GENERAL_PROTECTION,           // 13
    CRASH_PAGE_FAULT,                   // 14
    CRASH_RESERVED_15,                  // 15

    // x87 / SIMD / CPU-specific Exceptions
    CRASH_X87_FPU_EXCEPTION,            // 16
    CRASH_ALIGNMENT_CHECK,              // 17
    CRASH_MACHINE_CHECK,                // 18
    CRASH_SIMD_FP_EXCEPTION,            // 19
    CRASH_VIRTUALIZATION_EXCEPTION,     // 20
    CRASH_CONTROL_PROTECTION_EXCEPTION, // 21

    // Reserved / Unknown
    CRASH_RESERVED_22,                  // 22
    CRASH_RESERVED_23,                  // 23
    CRASH_RESERVED_24,                  // 24
    CRASH_RESERVED_25,                  // 25
    CRASH_RESERVED_26,                  // 26
    CRASH_RESERVED_27,                  // 27

    // Hypervisor
    CRASH_HYPERVISOR_INJECTION,         // 28
    CRASH_VMM_COMMUNICATION,            // 29

    // Security / Other
    CRASH_SECURITY_EXCEPTION,           // 30
    CRASH_RESERVED_31,                  // 31

    CRASH_CODES_AMOUNT                  // total count
};

char* isr_error_bits[CRASH_CODES_AMOUNT][32] = {

    // 13 - General Protection Fault
    [CRASH_GENERAL_PROTECTION] = {
        "External event (EXT)",            // 0
        "Descriptor location (IDT=1)",     // 1
        "Table indicator (LDT=1)",         // 2
        "Selector index bit 0",            // 3
        "Selector index bit 1",            // 4
        "Selector index bit 2",            // 5
        "Selector index bit 3",            // 6
        "Selector index bit 4",            // 7
        "Selector index bit 5",            // 8
        "Selector index bit 6",            // 9
        "Selector index bit 7",            // 10
        "Selector index bit 8",            // 11
        "Selector index bit 9",            // 12
        "Selector index bit 10",           // 13
        "Selector index bit 11",           // 14
        "Selector index bit 12",           // 15
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    },

    // 14 - Page Fault
    [CRASH_PAGE_FAULT] = {
        "Present",  // 0
        "Write access",                          // 1
        "User mode access",                      // 2
        "Reserved bit violation",                // 3
        "Instruction fetch",                     // 4
        "Protection key violation",              // 5
        "Shadow stack access",                   // 6
        "HLAT violation",                        // 7
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0
    }
};

void _Log_Isr_Error_Code(char isr_idx, uint32_t code){
    char** errcodes_array = isr_error_bits[isr_idx];

    for(int i=0;i < 31;i++){
        if(errcodes_array[i] ){
            bool set = (code & 1 << i);
            Sys_color_log_NoPos("%s: %s\n",set? ANSI_GREEN:ANSI_RED, ANSI_BG_BLACK, errcodes_array[i], set? "Yes":"No");
        }
    }
}




cpu_registers_t* _cpu_regs;

volatile char panic_count = 0;
void _panic_handler(int argc, uint32_t* argv) {

    if (panic_count >= MAX_KPANIK_COUNT) {
        Sys_color_log_NoPos("Double Fault (%d) %x\n", ANSI_RED, ANSI_BG_BLACK, (int)argv[0], (uint32_t)((cpu_registers_t*)argv[2])->cr2);
        Sys_color_log_NoPos("Fix your shit\n", ANSI_RED, ANSI_BG_BLACK);
        for (;;);
    }

    panic_count++;

    task_switching_flag = false;
    int isr_index = (int)argv[0];

    Sys_log("Kernel panic (%d | %d | CR2:0x%x)\n",
        (int)argv[0],
        (int)argv[1],
        ((cpu_registers_t*)argv[2])->cr2);

    if (Get_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON)) {
        ClearScreen();
        // size_t fb_size = Multiboot_info->framebuffer_width *
        //                  Multiboot_info->framebuffer_height;
        // dw_memset((void*)graph_mode_fb, 0x000000FF, fb_size);
    }

    uint32_t* call_stack = NULL;
    if (argc >= 4) call_stack = (uint32_t*)argv[3];

    if (argc < 4) {
        Sys_log_NoPos("Crash Handler: Not enough panic info provided\n");
        return;
    }

    uint32_t err_code = argv[1];
    _cpu_regs = (cpu_registers_t*)argv[2];

    const char* error_name =
        (isr_index >= 0 &&
         isr_index < (int)(sizeof(crash_messages) / sizeof(crash_messages[0])))
            ? crash_messages[isr_index]
            : "Unknown Crash";

    Sys_log_NoPos("=======================================================================\n");
    Sys_log_NoPos("KERNEL PANIK -> ISR Index: %d (%s), Error Code: %u\n",
        isr_index, error_name, err_code);

    Sys_log_NoPos(" A critical error has occurred:\n");

    Sys_log_NoPos(" Error Code: %s (%03d: %u)\n",
        error_name, isr_index, err_code);

        
    Sys_color_log_NoPos("Error Bits:\n", ANSI_CYAN, ANSI_BG_BLACK);
    
    _Log_Isr_Error_Code(isr_index, err_code);

    Sys_log_NoPos(" Regs Dump:\n");

    Sys_log_NoPos(" EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x\n",
        _cpu_regs->eax, _cpu_regs->ebx,
        _cpu_regs->ecx, _cpu_regs->edx);

    Sys_log_NoPos(" ESI: 0x%x  EDI: 0x%x  EBP: 0x%x  ESP: 0x%x\n",
        _cpu_regs->esi, _cpu_regs->edi,
        _cpu_regs->ebp, _cpu_regs->esp);

    Sys_log_NoPos(" EIP: 0x%x  EFLAGS: 0x%x\n",
        _cpu_regs->eip, _cpu_regs->eflags);

    Sys_log_NoPos(" CS:  0x%x  DS:  0x%x  ES:  0x%x\n",
        _cpu_regs->cs, _cpu_regs->ds, _cpu_regs->es);

    Sys_log_NoPos(" FS:  0x%x  GS:  0x%x  SS:  0x%x\n",
        _cpu_regs->fs, _cpu_regs->gs, _cpu_regs->ss);

    Sys_log_NoPos(" CR0: 0x%x  CR2: 0x%x\n",
        _cpu_regs->cr0, _cpu_regs->cr2);

    Sys_log_NoPos(" CR3: 0x%x\n", _cpu_regs->cr3);

    asm volatile("sti");
    if (call_stack) {
        Sys_log_NoPos(" Call Stack Trace:\n");
        for (int i = 0; i < MAX_STACK_TRACE_SIZE; i++) {
            uint32_t addr = call_stack[i];
            if (addr < 0x1000) { 
                Sys_color_log_NoPos(" invalid backtrace addr: 0x%x\n",
                    ANSI_RED, ANSI_BG_BLACK, addr);
                break;
            }
            char tmp_buffer[64];
            BacktraceSymbol* sym = Get_Symbol(addr, tmp_buffer);
            if(!sym) {
                sym = (BacktraceSymbol*)tmp_buffer;
                strcpy(&tmp_buffer[sizeof(*sym)], "unknown symbol");
            }
            Sys_color_log_NoPos("  %s (%x)\n",
                ANSI_BRIGHT_YELLOW, ANSI_BG_BLACK, sym->str, addr);
        }
    }

    Sys_log_NoPos(" Rebooting in 10 sec...\n");

    

    for (int i = 0; i < 10; i++) {
        draw_bitmap_char('#',
            60 + (8 * i), 440,
            8, 16,
            0xFF00FF00,
            font8x16,
            false, true, true);
        sleep(1000);
    }
    Sys_Breakpoint();
    // pc_reboot();
}


void _manual_panic(const char* error, const char* info) {

    graph_mode_fb = (volatile uint32_t*)
        (uint32_t)Multiboot_info->framebuffer_addr;

    int fb_size = Multiboot_info->framebuffer_height *
                  Multiboot_info->framebuffer_pitch;

    memset((void*)graph_mode_fb, 0, fb_size);

    Sys_log_NoPos("kernel panic triggered!\n");
    Sys_log_NoPos("  Error: %s\n", error ? error : "(null)");
    Sys_log_NoPos("  Info : %s\n", info ? info : "(null)");

    if (error) {
        Sys_Error(error, 20, 60, 8, 16,
                  0xFFFFFFFF, font8x16,
                  false, true, 0);
    }

    if (info) {
        Sys_Warning(info, 20, 90, 8, 16,
                    0xFFFFFFFF, font8x16,
                    false, true, 0);
    }

    Sys_log_NoPos(" Rebooting in 5 sec...\n");

    asm volatile("sti");

    for (int i = 0; i < 5; i++) {
        draw_bitmap_char('#',
            20 + (8 * i), 160,
            8, 16,
            0xFF00FF00,
            font8x16,
            false, true, true);
        sleep(1000);
    }

    // pc_reboot();
}

