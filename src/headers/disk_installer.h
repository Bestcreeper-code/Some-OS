#ifndef DISK_INSTALLER_H
#define DISK_INSTALLER_H

#include <stdint.h>

typedef struct {
    uint8_t head;
    uint16_t start_sector : 6;
    uint16_t start_cylinder : 10;
} __attribute__((packed)) head_sector_cylinder_t;

typedef struct {
    uint8_t boot_indicator; // 0x80 - active, 0x00 - inactive
    
    head_sector_cylinder_t start_head_sec_cyl;// unused
    
    uint8_t system_id;
    
    head_sector_cylinder_t end_head_sec_cyl;// unused
    
    uint32_t relative_sector; // starting sector counting from 0 (= start lba)
    
    uint32_t total_sectors;   // size of the partition in sectors
} __attribute__((packed)) Disk_Partition_Entry_t;






typedef struct {
    uint8_t boot_code[446];
    Disk_Partition_Entry_t partition_table[4];
    uint8_t signature[2]; //AA55h
} __attribute__((packed)) Disk_MBR_t;


typedef enum {
    DISK_INSTALLER_RET_NO_AVAILABLE_PARTIT = -3,
    DISK_INSTALLER_RET_NO_DISK = -2,
    DISK_INSTALLER_RET_MALLOC_FAIL = -1,
    DISK_INSTALLER_RET_SUCCESS = 0,
    // DISK_INSTALLER_RET_,
} DiskInstallerRetCode;

#endif // DISK_INSTALLER_H
