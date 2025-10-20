// #include "headers/disk_installer.h"

// #include "headers/multiboot_info.h"
// #include "headers/io.h"
// #include "headers/memory.h"
// #include "headers/ATA_IO.h"
// #include "headers/FileSystem.h"
// #include "headers/string.h"
// #include "data/globals.h"

// int Install_OS_to_disk(multiboot_module_t* bl_file) {
//     FIL file;
//     FRESULT res;
//     UINT bytesWritten;
//     int ret = 0;

//     Sys_log("Starting OS ISO installation to disk");

//     if (!ata_drive_exists()) {
//         Sys_log("No ATA drive detected");
//         return DISK_INSTALLER_RET_NO_DISK;
//     }

//     // === Step 1: Write ISO to beginning of disk ===
//     uint32_t iso_size_bytes = bl_file->mod_end - bl_file->mod_start;
//     uint32_t iso_sectors = (iso_size_bytes + 511) / 512;

//     Sys_log("ISO size: %u bytes (%u sectors)", iso_size_bytes, iso_sectors);

//     uint16_t steps_per_char = iso_sectors / 10;
//     char text_offset = strlen("Writing ISO to disk: [");
//     printf("Writing ISO to disk: [          ]");

//     for (uint32_t i = 0; i < iso_sectors; i++) {
//         if (i % steps_per_char == 0) {
//             put_char(text_offset + i / steps_per_char ,vgaY,'#', 0x0A);
//         }
//         uint8_t* sector_data = (uint8_t*)(bl_file->mod_start + i * 512);
//         ata_pio_write_sector(i, sector_data);
//         Sys_log("Written ISO sector %u", i);
//     }

//     // === Step 2: Create a new FAT32 partition AFTER ISO ===
//     Disk_MBR_t* MBR_buff = malloc(512);
//     if (!MBR_buff) {
//         Sys_log("Failed to allocate MBR buffer");
//         return DISK_INSTALLER_RET_MALLOC_FAIL;
//     }

//     memcpy(MBR_buff, (void*)bl_file->mod_start, 512);
//     MBR_buff->signature = 0xAA55;// Ensure valid signature

//     uint32_t drive_sectors = ata_get_sector_count();
//     uint32_t fat32_start_lba = iso_sectors;

//     // Align to next 2048-sector boundary (1MB alignment)
//     if (fat32_start_lba < 2048) {
//         fat32_start_lba = 2048;
//     } else if (fat32_start_lba % 2048 != 0) {
//         fat32_start_lba = ((fat32_start_lba / 2048) + 1) * 2048;
//     }

//     // Determine size of FAT32 partition
//     uint32_t fat32_size = drive_sectors - fat32_start_lba;
//     uint32_t fat32_max = (drive_sectors * Fat_SYS_Main_Part_Drive_Percentage) / 100;
//     if (fat32_size > Fat_SYS_Main_Part_Max_Size)
//         fat32_size = Fat_SYS_Main_Part_Max_Size;
//     else if (fat32_size > fat32_max)
//         fat32_size = fat32_max;

//     Sys_log("Creating FAT32 partition: start LBA = %u, size = %u", fat32_start_lba, fat32_size);

//     // Fill partition entry
//     Disk_Partition_Entry_t* part = &MBR_buff->partition_table[1]; // use second slot to avoid overwriting ISO
//     part->boot_indicator = 0x80;         // Mark active
//     part->system_id = 0x0C;              // FAT32 LBA
//     part->relative_sector = fat32_start_lba;
//     part->total_sectors = fat32_size;

//     ata_pio_write_sector(0, (uint8_t*)MBR_buff);
//     Sys_log("Written new MBR with FAT32 partition");

//     // === Step 3: Format the FAT32 partition ===
//     MKFS_PARM mkfs_param = {
//         .fmt = FM_FAT32,
//         .n_fat = 0,
//         .align = 0,
//         .n_root = 0,
//         .au_size = 0
//     };

//     // Set volume to partition index 1
//     PARTITION Main_Partit = { .pd = 0, .pt = 1 };
//     Set_vol_to_part_index(0, Main_Partit);

//     res = f_mkfs("0:", &mkfs_param, MBR_buff, 512);
//     if (res != FR_OK) {
//         Sys_log("Failed to format FAT32 partition (f_mkfs = %d)", res);
//         ret = DISK_INSTALLER_RET_MKFS_FAIL;
//         goto cleanup;
//     }

//     f_setlabel(OS_PARTITION_LABEL);
//     Sys_log("FAT32 partition formatted and labeled");

// cleanup:
//     if (MBR_buff)
//         free(MBR_buff);

//     Sys_log("Installation finished with code %d", ret);
//     return ret;
// }
