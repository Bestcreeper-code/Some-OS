#include "bootloader.h"
#include "compiler_defs.h"
#include "multiboot_info.h"
#include "string.h"
#include <stdint.h>


multiboot_info_t mb_info;


struct bl_info boot_info = {.boot_protocol= "Multiboot1"};
struct bl_mem_info mem_info;
struct bl_framebuffer framebuffer_info;
struct bootloader_loaded_module bl_modules_list[16];

int bootloader_c_entry(unsigned int magic, unsigned long mb_struct_addr){
    
    memcpy(&mb_info, (void*)mb_struct_addr, sizeof(multiboot_info_t));  
    

    boot_info.boot_flags = mb_info.flags;
    strcpy(boot_info.bootloader_name, (char*)mb_info.boot_loader_name);
    strcpy(boot_info.cmdline, (char*)mb_info.cmdline);

    mem_info.mmap_addr = (struct bootloader_mmap_entry*)mb_info.mmap_addr;
    mem_info.mmap_length = mb_info.mmap_length;
    mem_info.mem_lower = mb_info.mem_lower;
    mem_info.mem_upper = mb_info.mem_upper;

    framebuffer_info.addr = mb_info.framebuffer_addr;
    framebuffer_info.bits_per_pixels = mb_info.framebuffer_bpp;

    framebuffer_info.height = mb_info.framebuffer_height;
    framebuffer_info.width = mb_info.framebuffer_width;
    framebuffer_info.pitch = mb_info.framebuffer_pitch;

    framebuffer_info.red_field_position   = mb_info.framebuffer_red_field_position;
    framebuffer_info.red_mask_size        = mb_info.framebuffer_red_mask_size;
    framebuffer_info.green_field_position = mb_info.framebuffer_green_field_position;
    framebuffer_info.green_mask_size      = mb_info.framebuffer_green_mask_size;
    framebuffer_info.blue_field_position  = mb_info.framebuffer_blue_field_position;
    framebuffer_info.blue_mask_size       = mb_info.framebuffer_blue_mask_size;
    
    return 0;
}
