#include "arch_paging.h"
#include "bootloader.h"

#include "asm.h"
#include "compiler_defs.h"
#include "multiboot2/multiboot2_info.h"
#include "paging.h"
#include "string.h"
#include "config.h"
#include "Logger.h"
#include <stdint.h>



static struct bootloader_mmap_entry boot_mmap_buffer[64];

struct bl_info boot_info = {.boot_protocol= "Multiboot2"};
struct bl_mem_info mem_info;
struct bl_framebuffer framebuffer_info;
struct bootloader_loaded_module bl_modules_list[16];
uint8_t bl_modules_list_space_left = sizeof(bl_modules_list)/ sizeof(bl_modules_list[0]);
uint8_t* _rsdp_ptr;

int bootloader_c_entry(unsigned int magic, unsigned long mb_struct_addr){
    struct multiboot_tag *tag;
    unsigned size;

#if VERY_EARLY_SERIAL
    serial_init();
#endif
    

    /*  Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
        {
        Sys_log ("Invalid magic number: 0x%x\n", (unsigned) magic);
        return 1;
        }

    if (mb_struct_addr & 7)
        {
        Sys_log ("Unaligned mbi: 0x%x\n", mb_struct_addr);
        return 1;
        }

    size = *(unsigned *) mb_struct_addr;
    Sys_log ("Announced mbi size 0x%x\n", size);
    for (tag = (struct multiboot_tag *) (mb_struct_addr + 8);
        tag->type != MULTIBOOT_TAG_TYPE_END;
        tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                        + ((tag->size + 7) & ~7)))
    {
      Sys_log ("Tag 0x%x, Size 0x%x\n", tag->type, tag->size);
      switch (tag->type)
            {
            case MULTIBOOT_TAG_TYPE_CMDLINE:
                strcpy(boot_info.cmdline, ((struct multiboot_tag_string*)tag)->string);
                boot_info.boot_flags |= BL_BOOT_FLAG_CMDLINE;
                break;




            case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
                strcpy(boot_info.bootloader_name, ((struct multiboot_tag_string*)tag)->string);
                boot_info.boot_flags |= BL_BOOT_FLAG_BOOT_LOADER_NAME;
                break;




            case MULTIBOOT_TAG_TYPE_MODULE:
                boot_info.boot_flags |= BL_BOOT_FLAG_MODS;
                struct multiboot_tag_module* module_tag = (struct multiboot_tag_module *) tag;
                Sys_log ("Module at 0x%x-0x%x. Command line %s\n",
                    module_tag->mod_start,
                    module_tag->mod_end,
                    module_tag->cmdline);
                if(bl_modules_list_space_left) {
                    struct bootloader_loaded_module* curr = &bl_modules_list[arr_lengthof(bl_modules_list) - bl_modules_list_space_left];

                    strncpy(curr->cmdline, module_tag->cmdline, sizeof(curr->cmdline));
                    curr->mod_start = module_tag->mod_start;
                    curr->mod_end = module_tag->mod_end;
                    bl_modules_list_space_left--;
                }

                        page_reserve_page_early(ADDR_TO_PAGE(module_tag->mod_start), 
                            ADDR_TO_PAGE(module_tag->mod_end - module_tag->mod_start)+1);

                Sys_log("%d modules detected\n", arr_lengthof(bl_modules_list) - bl_modules_list_space_left);
                break;




            case MULTIBOOT_TAG_TYPE_BASIC_MEMINFO:
                boot_info.boot_flags |= BL_BOOT_FLAG_MEMORY;
                mem_info.mem_lower = ((struct multiboot_tag_basic_meminfo *) tag)->mem_lower;
                mem_info.mem_upper = ((struct multiboot_tag_basic_meminfo *) tag)->mem_upper;
                break;




            case MULTIBOOT_TAG_TYPE_BOOTDEV:
                boot_info.boot_flags |= BL_BOOT_FLAG_BOOTDEV;
                Sys_log ("Boot device 0x%x,%u,%u\n",
                    ((struct multiboot_tag_bootdev *) tag)->biosdev,
                    ((struct multiboot_tag_bootdev *) tag)->slice,
                    ((struct multiboot_tag_bootdev *) tag)->part);
                break;



            
            case MULTIBOOT_TAG_TYPE_MMAP:
            {
                boot_info.boot_flags |= BL_BOOT_FLAG_MEM_MAP;
            
                struct multiboot_tag_mmap *tag_mmap = (struct multiboot_tag_mmap *) tag;
            
                multiboot_memory_map_t *mmap;
                struct bootloader_mmap_entry *mmap_out = boot_mmap_buffer;
            
                uint8_t *end = (uint8_t *)tag + tag->size;
                size_t mmap_count = 0;
            
                for (mmap = tag_mmap->entries;
                        (uint8_t *)mmap < end;
                        mmap = (multiboot_memory_map_t *)((uint8_t *)mmap + tag_mmap->entry_size))
                {
                    if (mmap_out >= boot_mmap_buffer + 64)
                        break;
            
                    mmap_out->addr = mmap->addr;
                    mmap_out->len  = mmap->len;
                    mmap_out->type = mmap->type;
            
                    mmap_out->size = sizeof(struct bootloader_mmap_entry)- sizeof(mmap_out->size);
            
                    Sys_log(" base_addr = 0x%x%x, length = 0x%x%x, type = 0x%x\n",
                        (unsigned)(mmap->addr >> 32),
                        (unsigned)(mmap->addr & 0xffffffff),
                        (unsigned)(mmap->len >> 32),
                        (unsigned)(mmap->len & 0xffffffff),
                        (unsigned)mmap->type);
                    mmap_out++;
                    mmap_count++;
                }
            
                mem_info.mmap_addr = boot_mmap_buffer;
                mem_info.mmap_length = tag->size - sizeof(struct multiboot_tag_mmap);
                
            
                break;
            }




            case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
            {
                boot_info.boot_flags |= BL_BOOT_FLAG_FRAMEBUFFER_INFO;
                struct multiboot_tag_framebuffer *tagfb
                = (struct multiboot_tag_framebuffer *) tag;

                framebuffer_info.addr = tagfb->common.framebuffer_addr;
                framebuffer_info.width = tagfb->common.framebuffer_width;
                framebuffer_info.height = tagfb->common.framebuffer_height;
                framebuffer_info.pitch = tagfb->common.framebuffer_pitch;
                framebuffer_info.size = tagfb->common.size;
                framebuffer_info.bits_per_pixels = tagfb->common.framebuffer_bpp;
                
                framebuffer_info.red_field_position = tagfb->framebuffer_red_field_position;
                framebuffer_info.red_mask_size = tagfb->framebuffer_red_mask_size;
                framebuffer_info.green_field_position = tagfb->framebuffer_green_field_position;
                framebuffer_info.green_mask_size = tagfb->framebuffer_green_mask_size;
                framebuffer_info.blue_field_position = tagfb->framebuffer_blue_field_position;
                framebuffer_info.blue_mask_size = tagfb->framebuffer_blue_mask_size;

                break;
            }




            case MULTIBOOT_TAG_TYPE_ACPI_OLD:
            {
                struct multiboot_tag_old_acpi* acpi_tag = (struct multiboot_tag_old_acpi*)tag;

                _rsdp_ptr = acpi_tag->rsdp;
            }

        }
    }
    tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                    + ((tag->size + 7) & ~7));
    Sys_log ("Total mbi size 0x%x\n", (unsigned) tag - mb_struct_addr);
}
