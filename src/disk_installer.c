#include "headers/disk_installer.h"

#include "headers/multiboot_info.h"
#include "headers/memory.h"
#include "headers/ATA_IO.h"
#include "data/globals.h"

int Install_OS_to_disk(multiboot_module_t* bl_file, multiboot_module_t* kernel_file){
    
    Disk_MBR_t* MBR_buff = malloc(512);
    if (!MBR_buff) {
        return DISK_INSTALLER_RET_MALLOC_FAIL;
    }

    if( !ata_drive_exists()){
        free(MBR_buff);
        return DISK_INSTALLER_RET_NO_DISK;
    }

    ata_pio_read_sector(0, MBR_buff);

    if(MBR_buff->signature == 0xAA55){
        goto fs_setup_label;
    }

    //write Grub Bootloader to disk
    uint32_t bl_sector_count = (bl_file->mod_end - bl_file->mod_start + 511) / 512;
    for(int i=0;i<bl_sector_count;i++){
        ata_pio_write_sector(i, (uint8_t*)(bl_file->mod_start + i * 512));
    }

    ata_pio_read_sector(0, MBR_buff);//update MBR Buffer

fs_setup_label:
    uint32_t drive_size = ata_get_sector_count();

    Disk_Partition_Entry_t* main_os_partition = NULL;
    uint32_t start_lba = 2048;

    for(int i=0;i<4;i++){
        uint32_t partit_end = MBR_buff->partition_table[i].relative_sector + MBR_buff->partition_table[i].total_sectors;
        if(MBR_buff->partition_table[i].boot_indicator != 0x80){
            main_os_partition = &MBR_buff->partition_table[i];
            break;
        }
        else if(MBR_buff->partition_table[i].relative_sector == 0 && MBR_buff->partition_table[i].total_sectors == 0){
            main_os_partition = &MBR_buff->partition_table[i];
            break;
        }
        else if( partit_end > start_lba && partit_end < drive_size ){
            start_lba = partit_end;
        }// finds the first available space after all other partitions
    }
    if(!main_os_partition){
        free(MBR_buff);
        return DISK_INSTALLER_RET_NO_AVAILABLE_PARTIT;
    }

    //Calculate partition size
    uint32_t part_size = drive_size - start_lba;
    if(part_size > Fat_SYS_Main_Part_Max_Size){
        part_size = Fat_SYS_Main_Part_Max_Size;
    }
    else if (part_size > (drive_size * Fat_SYS_Main_Part_Drive_Percentage) / 100){
        part_size = (drive_size * Fat_SYS_Main_Part_Drive_Percentage) / 100;
    }
    
    main_os_partition->boot_indicator = 0x80;
    main_os_partition->system_id = 0x0B; //FAT32
    main_os_partition->relative_sector = start_lba; //start sector
    main_os_partition->total_sectors = part_size; //size in sectors

    main_os_partition->start_head_sec_cyl = (head_sector_cylinder_t){0};// unused these days
    main_os_partition->end_head_sec_cyl = (head_sector_cylinder_t){0};// unused

    




    return 0;
}