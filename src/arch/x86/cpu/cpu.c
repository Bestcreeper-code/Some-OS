#include "cpu.h"
#include "Logger.h"
#include "cpuid.h"
#include "string.h"
#include <stdint.h>
#include <sys/types.h>

short hfp;

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
                byte_nb_simplify(cache_size, cache_size_text);
                sys_color_serial_logf("L%d cache size = %s\n", ANSI_BRIGHT_CYAN, ANSI_BG_BLACK,"","",0, 
                    cache_level, cache_size_text);
            }

            cache_index++;
        }

    


    return 0;
}