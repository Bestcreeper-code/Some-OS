#pragma once

#include "bootloader_conf.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "compiler_defs.h"
int bootloader_init();
extern void bootloader_asm_entry();

struct bl_info {
    char bootloader_name[32];
    char boot_protocol[32];
    char cmdline[128];
    
    uint32_t boot_flags;
    #define BL_BOOT_FLAG_MEMORY                   0x00000001
    /* is there a boot device set? */
    #define BL_BOOT_FLAG_BOOTDEV                  0x00000002
    /* is the command-line defined? */
    #define BL_BOOT_FLAG_CMDLINE                  0x00000004
    /* are there modules to do something with? */
    #define BL_BOOT_FLAG_MODS                     0x00000008
    
    /* These next two are mutually exclusive */
    
    /* is there a symbol table loaded? */
    #define BL_BOOT_FLAG_AOUT_SYMS                0x00000010
    /* is there an ELF section header table? */
    #define BL_BOOT_FLAG_ELF_SHDR                 0X00000020
    
    /* is there a full memory map? */
    #define BL_BOOT_FLAG_MEM_MAP                  0x00000040
    
    /* Is there drive info? */
    #define BL_BOOT_FLAG_DRIVE_INFO               0x00000080
    
    /* Is there a config table? */
    #define BL_BOOT_FLAG_CONFIG_TABLE             0x00000100
    
    /* Is there a boot loader name? */
    #define BL_BOOT_FLAG_BOOT_LOADER_NAME         0x00000200
    
    /* Is there a APM table? */
    #define BL_BOOT_FLAG_APM_TABLE                0x00000400
    
    /* Is there video information? */
    #define BL_BOOT_FLAG_VBE_INFO                 0x00000800
    #define BL_BOOT_FLAG_FRAMEBUFFER_INFO         0x00001000
};

struct bl_framebuffer {
    uintptr_t addr;
	size_t size;
    size_t width;
    size_t height;
    size_t pitch;
    uint16_t bits_per_pixels;

    uint8_t red_field_position;
    uint8_t red_mask_size;
    uint8_t green_field_position;
    uint8_t green_mask_size;
    uint8_t blue_field_position;
    uint8_t blue_mask_size;
};

struct bl_mem_info{
    size_t mem_lower;
    size_t mem_upper;

    size_t mmap_length;
    struct bootloader_mmap_entry* mmap_addr;
};


struct bootloader_mmap_entry 
{
  	uint32_t size;
  	uint64_t addr;
  	uint64_t len;
#define BL_MMAP_MEMORY_AVAILABLE              1
#define BL_MMAP_MEMORY_RESERVED               2
#define BL_MMAP_MEMORY_ACPI_RECLAIMABLE       3
#define BL_MMAP_MEMORY_NVS                    4
#define BL_MMAP_MEMORY_BADRAM                 5
 	uint32_t type;
} GCC_ATTR((packed));

extern bool check_bl_flag(uint32_t index);

extern struct bl_info boot_info;
extern struct bl_mem_info mem_info;
extern struct bl_framebuffer framebuffer_info;

static inline struct bl_info* get_bootloader_generic_info(){
	return &boot_info;
}
static inline struct bl_framebuffer* get_bootloader_fb_info(){
  	return &framebuffer_info;
}
static inline struct bl_mem_info* get_bootloader_mem_info(){
  	return &mem_info;
}
