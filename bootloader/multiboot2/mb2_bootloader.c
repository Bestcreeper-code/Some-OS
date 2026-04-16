#include "bootloader.h"
#include "compiler_defs.h"
#include "multiboot2/multiboot2_info.h"
#include "string.h"
#include "config.h"
#include "Logger.h"
#include <stdint.h>



static struct bootloader_mmap_entry boot_mmap_buffer[64];

struct bl_info boot_info = {.boot_protocol= "Multiboot2"};
struct bl_mem_info mem_info;
struct bl_framebuffer framebuffer_info;

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
            Sys_log ("Module at 0x%x-0x%x. Command line %s\n",
                    ((struct multiboot_tag_module *) tag)->mod_start,
                    ((struct multiboot_tag_module *) tag)->mod_end,
                    ((struct multiboot_tag_module *) tag)->cmdline);
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
            
                boot_info.boot_flags |= BL_BOOT_FLAG_MEM_MAP;
                mem_info.mmap_addr = (struct bootloader_mmap_entry*)((struct multiboot_tag_mmap *) tag)->entries;
                mem_info.mmap_length = ((struct multiboot_tag_mmap *) tag)->size;

                multiboot_memory_map_t *mmap;
                
                struct bootloader_mmap_entry* mmap_out = boot_mmap_buffer; 

                for (mmap = ((struct multiboot_tag_mmap *) tag)->entries;
                    (multiboot_uint8_t *) mmap < (multiboot_uint8_t *) tag + tag->size;
                    mmap = (multiboot_memory_map_t *) ((unsigned long) mmap + ((struct multiboot_tag_mmap *) tag)->entry_size)){
                    
                    multiboot_memory_map_t tmp_entry= *mmap;
                    
                    
                    mmap_out->addr = tmp_entry.addr;
                    mmap_out->len = tmp_entry.len;
                    mmap_out->type = tmp_entry.type;

                    mmap_out->size = ((struct multiboot_tag_mmap *) tag)->entry_size;

                    Sys_log(" base_addr = 0x%x%x,"
                        " length = 0x%x%x, type = 0x%x\n",
                        (unsigned) (mmap->addr >> 32),
                        (unsigned) (mmap->addr & 0xffffffff),
                        (unsigned) (mmap->len >> 32),
                        (unsigned) (mmap->len & 0xffffffff),
                        (unsigned) mmap->type);//Sys_Breakpoint();

                    Sys_log("out base_addr = 0x%x%x,"
                        " length = 0x%x%x, type = 0x%x\n",
                        (unsigned) (mmap_out->addr >> 32),
                        (unsigned) (mmap_out->addr & 0xffffffff),
                        (unsigned) (mmap_out->len >> 32),
                        (unsigned) (mmap_out->len & 0xffffffff),
                        (unsigned) mmap_out->type);//Sys_Breakpoint();
                }
                
                break;
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

                // multiboot_uint32_t color;
                // unsigned i;
                
                // void *fb = (void *) (unsigned long) tagfb->common.framebuffer_addr;

                // switch (tagfb->common.framebuffer_type)
                // {
                // case MULTIBOOT_FRAMEBUFFER_TYPE_INDEXED:
                //     {
                //     unsigned best_distance, distance;
                //     struct multiboot_color *palette;
                
                //     palette = tagfb->framebuffer_palette;

                //     color = 0;
                //     best_distance = 4*256*256;
                
                //     for (i = 0; i < tagfb->framebuffer_palette_num_colors; i++)
                //         {
                //         distance = (0xff - palette[i].blue) 
                //             * (0xff - palette[i].blue)
                //             + palette[i].red * palette[i].red
                //             + palette[i].green * palette[i].green;
                //         if (distance < best_distance)
                //             {
                //             color = i;
                //             best_distance = distance;
                //             }
                //         }
                //     }
                //     break;

                // case MULTIBOOT_FRAMEBUFFER_TYPE_RGB:
                //     color = ((1 << tagfb->framebuffer_blue_mask_size) - 1) 
                //     << tagfb->framebuffer_blue_field_position;
                //     break;

                // case MULTIBOOT_FRAMEBUFFER_TYPE_EGA_TEXT:
                //     color = '\\' | 0x0100;
                //     break;

                // default:
                //     color = 0xffffffff;
                //     break;
                // }
                
                // for (i = 0; i < tagfb->common.framebuffer_width
                //     && i < tagfb->common.framebuffer_height; i++)
                // {
                //     switch (tagfb->common.framebuffer_bpp)
                //     {
                //     case 8:
                //         {
                //         multiboot_uint8_t *pixel = fb
                //             + tagfb->common.framebuffer_pitch * i + i;
                //         *pixel = color;
                //         }
                //         break;
                //     case 15:
                //     case 16:
                //         {
                //         multiboot_uint16_t *pixel
                //             = fb + tagfb->common.framebuffer_pitch * i + 2 * i;
                //         *pixel = color;
                //         }
                //         break;
                //     case 24:
                //         {
                //         multiboot_uint32_t *pixel
                //             = fb + tagfb->common.framebuffer_pitch * i + 3 * i;
                //         *pixel = (color & 0xffffff) | (*pixel & 0xff000000);
                //         }
                //         break;

                //     case 32:
                //         {
                //         multiboot_uint32_t *pixel
                //             = fb + tagfb->common.framebuffer_pitch * i + 4 * i;
                //         *pixel = color;
                //         }
                //         break;
                //     }
                // }
                // break;
            }

        }
    }
    tag = (struct multiboot_tag *) ((multiboot_uint8_t *) tag 
                                    + ((tag->size + 7) & ~7));
    Sys_log ("Total mbi size 0x%x\n", (unsigned) tag - mb_struct_addr);
}
