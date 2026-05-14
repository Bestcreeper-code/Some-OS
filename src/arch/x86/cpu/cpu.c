#include "cpu.h"
#include "Logger.h"
#include "cpuid.h"
#include "string.h"
#include <stdint.h>
#include <sys/types.h>

#define FLAG_PER_ROW 8

short hfp;


char ecx_cpuid_1_flags[][32] = {
    "SSE3"       ,
    "PCLMUL"     ,
    "DTES64"     ,
    "MONITOR"    ,
    "DS_CPL"     ,
    "VMX"        ,
    "SMX"        ,
    "EST"        ,
    "TM2"        ,
    "SSSE3"      ,
    "CID"        ,
    "SDBG"       ,
    "FMA"        ,
    "CX16"       ,
    "XTPR"       ,
    "PDCM"       ,
    ""                          ,
    "PCID"       ,
    "DCA"        ,
    "SSE4_1"     ,
    "SSE4_2"     ,
    "X2APIC"     ,
    "MOVBE"      ,
    "POPCNT"     ,
    "TSC"        ,
    "AES"        ,
    "XSAVE"      ,
    "OSXSAVE"    ,
    "AVX"        ,
    "F16C"       ,
    "RDRAND"     ,
    "HYPERVISOR" ,
};


char edx_cpuid_1_flags[][32]= {
    "FPU"        ,
    "VME"        ,
    "DE"         ,
    "PSE"        ,
    "TSC"        ,
    "MSR"        ,
    "PAE"        ,
    "MCE"        ,
    "CX8"        ,
    "APIC"       ,
    ""                          ,
    "SEP"        ,
    "MTRR"       ,
    "PGE"        ,
    "MCA"        ,
    "CMOV"       ,
    "PAT"        ,
    "PSE36"      ,
    "PSN"        ,
    "CLFLUSH"    ,
    ""                          ,
    "DS"         ,
    "ACPI"       ,
    "MMX"        ,
    "FXSR"       ,
    "SSE"        ,
    "SSE2"       ,
    "SS"         ,
    "HTT"        ,
    "TM"         ,
    "IA64"       ,
    "PBE"               
};

uint64_t cpu_features = 0;

int cpu_log_specs(){
    register_t eax = 0;
    register_t ebx = 0;
    register_t ecx = 0;
    register_t edx = 0;
    uint32_t name_string[4];

    name_string[3] = 0;
    __cpuid(0, hfp, name_string[0], name_string[2], name_string[1]);//name and max 
    
    sys_color_serial_logf("Cpu Manufacturer: %s\n", ANSI_BRIGHT_CYAN, ANSI_BG_BLACK,"","",0, 
        (char*)name_string);

    sys_color_serial_logf("Highest Function Parameter: %u\n", ANSI_BRIGHT_CYAN, ANSI_BG_BLACK,"","",0, 
        hfp);
        
    
        //fetch cache sizes
        int cache_index = 0;

        while (1) {
            __cpuid_count(4, cache_index, eax, ebx, ecx, edx);

            int cache_type = eax & 0x1F; 
            if (cache_type == 0)
                break; 

            int cache_level = (eax >> 5) & 0x7;
            if (cache_level <= 3) {
                unsigned int line_size = (ebx & 0xFFF) + 1;
                unsigned int partitions = ((ebx >> 12) & 0x3FF) + 1;
                unsigned int ways = ((ebx >> 22) & 0x3FF) + 1;
                unsigned int sets = ecx + 1;

                unsigned int cache_size = ways * partitions * line_size * sets;
                char cache_size_text[8];
                byte_nb_simplify(cache_size, cache_size_text,0);
                sys_color_serial_logf("L%d cache size = %s\n", ANSI_BRIGHT_CYAN, ANSI_BG_BLACK,"","",0, 
                    cache_level, cache_size_text);
            }

            cache_index++;
        }
    
        __cpuid(0x1, eax, ebx, ecx, edx);

        unsigned int brand_index        = ebx & 0xFF;
        unsigned int clflush_line_size  = (ebx >> 8) & 0xFF;
        unsigned int apic_id            = (ebx >> 16) & 0xFF;
        unsigned int initial_apic_id    = (ebx >> 24) & 0xFF;

        sys_color_serial_logf(
            "BRAND INDEX: %x CLFLUSH_LINE_SIZE: %x APIC_ID: %x INITIAL_APIC_ID: %x\n",
            ANSI_BRIGHT_CYAN, ANSI_BG_BLACK, "", "", 0,
            brand_index,
            clflush_line_size,
            apic_id,
            initial_apic_id
        );
    
        // ECX flags
        int count = 0;
        for (int i = 0; i < 32; i++) {
            if (ecx_cpuid_1_flags[i][0] == '\0')
                continue;

            int enabled = (ecx >> i) & 1;

            sys_color_serial_logf("%s:%d  ",
                enabled ? ANSI_GREEN : ANSI_RED,
                ANSI_BG_BLACK,
                "",
                "",
                0,
                ecx_cpuid_1_flags[i],
                enabled);

            count++;

            if (count % FLAG_PER_ROW == 0)
                sys_color_serial_logf("\n", ANSI_RESET, ANSI_BG_BLACK, "", "", 0);
        }

        if (count % FLAG_PER_ROW != 0)
            sys_color_serial_logf("\n", ANSI_RESET, ANSI_BG_BLACK, "", "", 0);


        // EDX flags
        count = 0;
        for (int i = 0; i < 32; i++) {
            if (edx_cpuid_1_flags[i][0] == '\0')
                continue;

            int enabled = (edx >> i) & 1;

            sys_color_serial_logf("%s:%d  ",
                enabled ? ANSI_GREEN : ANSI_RED,
                ANSI_BG_BLACK,
                "",
                "",
                0,
                edx_cpuid_1_flags[i],
                enabled);

            count++;

            if (count % FLAG_PER_ROW == 0)
                sys_color_serial_logf("\n", ANSI_RESET, ANSI_BG_BLACK, "", "", 0);
        }

        if (count % FLAG_PER_ROW != 0)
            sys_color_serial_logf("\n", ANSI_RESET, ANSI_BG_BLACK, "", "", 0);

    return 0;
}


void register_cpu_features() {
    register_t eax = 0;
    register_t ebx = 0;
    register_t ecx = 0;
    register_t edx = 0;
    __cpuid(0x1, eax, ebx, ecx, edx);

    cpu_features = ((uint64_t)edx << 32) | (uint32_t)ecx;
}