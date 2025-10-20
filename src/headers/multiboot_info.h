#ifndef MULTIBOOT_INFO_H
#define MULTIBOOT_INFO_H

#include <stdint.h>
#include <stdbool.h>
#include "addresses.h"
#define MULTIBOOT_MMAP_FREE_MEMORY 1

// #define multiboot_info_storing_adress 0x26FA


typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

#define MULTIBOOT_MEMORY_AVAILABLE              1
#define MULTIBOOT_MEMORY_RESERVED               2
#define MULTIBOOT_MEMORY_ACPI_RECLAIMABLE       3
#define MULTIBOOT_MEMORY_NVS                    4
#define MULTIBOOT_MEMORY_BADRAM                 5
typedef struct {
    uint32_t flags;        // flags
    uint32_t mem_lower;    // lower memory (KB)
    uint32_t mem_upper;    // upper memory (KB)
    uint32_t boot_device;  // boot device ID
    uint32_t cmdline;      // address of kernel command line
    uint32_t mods_count;   // number of modules loaded
    uint32_t mods_addr;    // address of first module structure

    uint32_t irrelevant[4];

    uint32_t mmap_length;  // memory map length
    uint32_t mmap_addr;    // memory map address
    uint32_t drives_length; // drive info length
    uint32_t drives_addr;   // drive info address
    uint32_t config_table;  // ROM configuration table
    uint32_t boot_loader_name; // bootloader name string address
    uint32_t apm_table;     // APM table address
    uint32_t vbe_control_info; // VBE control information
    uint32_t vbe_mode_info; // VBE mode information
    uint16_t vbe_mode;      // VBE mode
    uint16_t vbe_interface_seg;  // VBE interface segment
    uint16_t vbe_interface_off;  // VBE interface offset
    uint16_t vbe_interface_len;  // VBE interface length

    // FRAMEBUFFER INFO (GRUB2 ONLY)
    uint64_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint8_t  framebuffer_bpp;
    uint8_t  framebuffer_type;
    uint8_t  reserved[2];
} __attribute__((__packed__)) multiboot_info_t;



typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    uint32_t cmdline;
    uint32_t pad;
} multiboot_module_t;

#define Multiboot_info (Get_multiboot_info())


bool checkFlag(multiboot_info_t mb_info, uint8_t index);

multiboot_info_t* Get_multiboot_info();

multiboot_module_t* Multiboot_Get_loaded_module(multiboot_info_t* mbinfo, const char* name);

#endif // MULTIBOOT_INFO_H
