#include "../headers/io.h"
#include "../headers/video.h"
#include "../headers/Logger.h"
#include "../headers/memory.h"
#include "../headers/string.h"
#include "../headers/FileSystem.h"
#include "../headers/time.h"
#include "../headers/vga_modes.h"
#include "../headers/power.h"
#include "../headers/asm.h"
#include "../headers/panic.h"
#include "../headers/symbols.h"
#include "../headers/scheduler.h"
#include "../data/textconsts.h"
#include "kernel_data.h"
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


cpu_registers_t* _cpu_regs;

volatile char panic_count = 0;

void _panic_handler(int argc, uint32_t* argv) {

    if(panic_count >= MAX_KPANIK_COUNT){
        Sys_color_log("Double Fault\n",ANSI_RED,ANSI_BG_BLACK);
        Sys_color_log("Fix your shit\n",ANSI_RED,ANSI_BG_BLACK);
        for(;;);
    }

    panic_count++;


    task_switching_flag = false;
    int isr_index = (int)argv[0];
    Sys_log("Kernel panic (%d | %d | CR2:0x%x)\n",(int)argv[0],(int)argv[1],((cpu_registers_t*)argv[2])->cr2);
    
    
    if(Get_Kernel_Flag(KDATA_FLAG_KERNEL_TERMINAL_ON)){
        ClearScreen();
        size_t fb_size = Multiboot_info->framebuffer_width * Multiboot_info->framebuffer_height;
        dw_memset((void*)graph_mode_fb, 0x000000FF, fb_size);
    }
    

    uint32_t* call_stack = NULL;
    if (argc >= 4) call_stack = (uint32_t*)argv[3];

    if (argc < 4) {
        Sys_log("Crash Handler: Not enough panic info provided\n");
       
        return;
    }

    uint32_t err_code = argv[1];
    _cpu_regs = (cpu_registers_t*)argv[2];

    const char* error_name = (isr_index >= 0 && isr_index < (int)(sizeof(crash_messages) / sizeof(crash_messages[0])))
                             ? crash_messages[isr_index]
                             : "Unknown Crash";

    
    Sys_log("=======================================================================\n");
    Sys_log("KERNEL PANIK -> ISR Index: %d (%s), Error Code: %u\n", isr_index, error_name, err_code);

   
    Sys_log(" A critical error has occurred:\n");

    char full_str[128];
    sprintf(full_str, "Error Code: %s (%03d: %u)", error_name, isr_index, err_code);
    
    Sys_log(" %s\n", full_str);

   
    Sys_log(" Regs Dump:\n");

    char buf[128];
    sprintf(buf, "EAX: 0x%x  EBX: 0x%x  ECX: 0x%x  EDX: 0x%x", 
            _cpu_regs->eax, _cpu_regs->ebx, _cpu_regs->ecx, _cpu_regs->edx);
    
    Sys_log(" %s\n", buf);

    sprintf(buf, "ESI: 0x%x  EDI: 0x%x  EBP: 0x%x  ESP: 0x%x", 
            _cpu_regs->esi, _cpu_regs->edi, _cpu_regs->ebp, _cpu_regs->esp);
   
    Sys_log(" %s\n", buf);

    sprintf(buf, "EIP: 0x%x  EFLAGS: 0x%x", _cpu_regs->eip, _cpu_regs->eflags);
    
    Sys_log(" %s\n", buf);

    // Segment Registers
    sprintf(buf, "CS:  0x%x  DS:  0x%x  ES:  0x%x", 
            _cpu_regs->cs, _cpu_regs->ds, _cpu_regs->es);
    
    Sys_log(" %s\n", buf);

    sprintf(buf, "FS:  0x%x  GS:  0x%x  SS:  0x%x", 
            _cpu_regs->fs, _cpu_regs->gs, _cpu_regs->ss);
    
    Sys_log(" %s\n", buf);

    // Control Registers
    sprintf(buf, "CR0: 0x%x  CR2: 0x%x", _cpu_regs->cr0, _cpu_regs->cr2);
    
    Sys_log(" %s\n", buf);

    sprintf(buf, "CR3: 0x%x ", _cpu_regs->cr3);
    
    Sys_log(" %s\n", buf);


    


    // Instruction bytes at EIP
    uint8_t* instr_ptr = (uint8_t*)_cpu_regs->eip;
    
    
    // _DecodedInst instructions[16]; // Enough space for all instructions
    // unsigned int count = 0;
    
    // Decode buffer (assume 32-bit mode)
    // _DecodeResult res = distorm_decode(
    //     0,               // codeOffset (virtual address, 0 if not needed)
    //     instr_ptr,            // pointer to code
    //     sizeof(16),    // length of buffer
    //     Decode32Bits,    // decode mode: 32-bit (i386)
    //     instructions,    // output array
    //     16,              // max instructions
    //     &count           // actual number of instructions decoded
    // );

                                       
                                       
                                       
    // char instr_bytes[64]; 
    // for (int i = 0; i < 16; i++) {
    //     sprintf(instr_bytes + i * 3, "%02x ", instructions[i].size );
    // }
    // sprintf(buf, "Code: %s", instr_bytes);


    // // draw_bitmap_string(buf, 50, 200, 8, 16, 0xFFFFFFFF, font8x16, false, true, 0);
    // Sys_log(" %s\n", buf);

    // Call Stack Trace
    if (call_stack) {
        // draw_bitmap_string("Call Stack Trace:", 0, 220, 8, 16, 0xFFFFFFFF, font8x16, false, true, 0);
        Sys_log(" Call Stack Trace:\n");
        for (int i = 0; i < MAX_STACK_TRACE_SIZE; i++) {
            char tmp_buffer[64];
            sprintf(buf, "%s (%x)", Get_Symbol(call_stack[i],tmp_buffer)->str, call_stack[i]);
            
            Sys_color_log(" %s\n",ANSI_BRIGHT_YELLOW, ANSI_BG_BLACK, buf);
        }
    }

    // Reboot Countdown
    
    Sys_log(" Rebooting in 10 sec...\n");
    
asm volatile("sti");
    for (int i = 0; i < 10; i++) {
        draw_bitmap_char('#', 60 + (8 * i), 440, 8, 16, 0xFF00FF00, font8x16, false, true, true);
        sleep(1000);
    }

    pc_reboot();
}


void _manual_panic(const char* error, const char* info) {
    

    
    graph_mode_fb = (volatile uint32_t*)(uint32_t)Multiboot_info->framebuffer_addr;
    int fb_size = Multiboot_info->framebuffer_height * Multiboot_info->framebuffer_pitch;
    memset((void*)graph_mode_fb, 0, fb_size);

    Sys_log("kernel panic triggered!\n");
    Sys_log("  Error: %s\n", error ? error : "(null)");
    Sys_log("  Info : %s\n", info ? info : "(null)");

    // Header
    // draw_bitmap_string("MANUAL PANIC", 20, 20, 8, 16, 0xFFFFFFFF, font8x16, false, true, 0);

    
    if (error) {
        Sys_Error(error, 20, 60, 8, 16, 0xFFFFFFFF, font8x16, false, true, 0);
    }

    
    if (info) {
        Sys_Warning(info, 20, 90, 8, 16, 0xFFFFFFFF, font8x16, false, true, 0);
    }

    
    
    Sys_log(" Rebooting in 5 sec...\n");

    

    asm volatile("sti");
    for (int i = 0; i < 5; i++) {
        draw_bitmap_char('#', 20 + (8 * i), 160, 8, 16,
            0xFF00FF00, font8x16, false, true, true);
        sleep(1000);
    }

    pc_reboot();
}
