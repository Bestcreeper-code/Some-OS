#ifndef ATA_IO
#define ATA_IO

#define ATA_DATA        0x1F0
#define ATA_ERROR       0x1F1
#define ATA_FEATURES    0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_STATUS      0x1F7
#define ATA_COMMAND     0x1F7

#define ATA_SR_BSY      0x80
#define ATA_SR_DRQ      0x08


#define SECTOR_SIZE 512


#include <stdint.h>
#include <stdbool.h>


struct ata_blkdev {
    uint8_t drive;
    uint32_t sector_count;
    uint32_t sector_size;
    uint16_t info_data[256];
};
int ata_dev_init();

void ata_pio_write_sector(struct ata_blkdev *ata, uint32_t lba, const uint8_t *buffer);
void ata_pio_read_sector(struct ata_blkdev *ata, uint32_t lba, uint8_t *buffer);
uint32_t ata_get_sector_count();
bool ata_drive_exists();



#endif