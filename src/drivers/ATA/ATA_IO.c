#include "ATA_IO.h"
#include "Logger.h"
#include "blkdev.h"
#include "helpers.h"
#include "ioctl.h"
#include "memory.h"
#include <asm.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_SECTOR_CNT 0x1F2
#define ATA_LBA_LOW    0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HIGH   0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7

#define ATA_CMD_READ   0x20
#define ATA_CMD_WRITE  0x30
#define ATA_CMD_IDENTIFY 0xEC
#define ATA_CMD_FLUSH  0xE7

#define ATA_SR_BSY  0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_DRQ  0x08
#define ATA_SR_ERR  0x01

static void ata_io_wait()
{
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
    inb(ATA_STATUS);
}

static int ata_wait_busy()
{
    uint8_t status;

    do {
        status = inb(ATA_STATUS);
    } while (status & ATA_SR_BSY);

    if (status & ATA_SR_ERR)
        return -1;

    return 0;
}

static int ata_wait_drq()
{
    uint8_t status;

    while (1)
    {
        status = inb(ATA_STATUS);

        if (status & ATA_SR_ERR)
            return -1;

        if (status & ATA_SR_DRQ)
            return 0;
    }
}

bool ata_drive_exists()
{
    outb(ATA_DRIVE, 0xA0);
    ata_io_wait();

    outb(ATA_SECTOR_CNT, 0);
    outb(ATA_LBA_LOW, 0);
    outb(ATA_LBA_MID, 0);
    outb(ATA_LBA_HIGH, 0);

    outb(ATA_COMMAND, ATA_CMD_IDENTIFY);

    uint8_t status = inb(ATA_STATUS);

    if (status == 0)
        return false;

    while (status & ATA_SR_BSY)
        status = inb(ATA_STATUS);

    if (status & ATA_SR_ERR)
        return false;

    while (!(status & ATA_SR_DRQ))
        status = inb(ATA_STATUS);

    for (int i = 0; i < 256; i++)
        inw(ATA_DATA);

    return true;
}

void ata_pio_read_sector(uint32_t lba, uint8_t *buffer)
{
    ata_wait_busy();

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_wait();

    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW,  (uint8_t)(lba));
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, ATA_CMD_READ);

    if (ata_wait_drq() != 0)
        return;

    for (int i = 0; i < 256; i++)
    {
        uint16_t word = inw(ATA_DATA);
        buffer[i * 2]     = word & 0xFF;
        buffer[i * 2 + 1] = word >> 8;
    }
}

void ata_pio_write_sector(uint32_t lba, const uint8_t *buffer)
{
    ata_wait_busy();

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    ata_io_wait();

    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW,  (uint8_t)(lba));
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, ATA_CMD_WRITE);

    if (ata_wait_drq() != 0)
        return;

    for (int i = 0; i < 256; i++)
    {
        uint16_t word =
            buffer[i * 2] |
            (buffer[i * 2 + 1] << 8);

        outw(ATA_DATA, word);
    }

    ata_wait_busy();

    outb(ATA_COMMAND, ATA_CMD_FLUSH);
    ata_wait_busy();
}


// int (*read)(struct block_device *, void *buf, size_t data_size, loff_t read_addr);
int ATA_blkdev_disk_read(struct block_device *dev, void* buffer, size_t data_size, loff_t read_addr) {
    RET_IF(!dev, -1);
}

// int (*write)(struct block_device *, const void *buf, size_t data_size, loff_t write_addr);
int ATA_blkdev_disk_write(struct block_device *dev, const void *buf, size_t data_size, loff_t write_addr) {
    RET_IF(!dev, -1);
}

uint32_t ATA_get_sector_count_from_dev(struct block_device *dev) {
    uint16_t *data = (uint16_t*)dev->private_data;
    return ((uint32_t)data[61] << 16) | data[60];
}

uint32_t ATA_get_sector_size_from_dev(struct block_device *dev) {
    uint16_t *data = (uint16_t*)dev->private_data;
    uint16_t words_per_sector = data[106] & 0xFFFF;
    // return words_per_sector ? words_per_sector * 2 : 512;
    return  512;
}

int ATA_blkdev_ioctl(struct block_device * blkdev, int op,...) {
    RET_IF(_IOC_TYPE(op) != 0x12, 1);
    switch (_IOC_NR(op)) {
        case BLKROSET:{
            return 0;
        }
        case BLKROGET:{
            return 0;
        }
        case BLKRRPART:{
            return 0;
        }
        case BLKGETSIZE:{
            return ATA_get_sector_count_from_dev(blkdev);
        }
        case BLKFLSBUF:{
            return 0;
        }
        case BLKRASET:{
            return 0;
        }
        case BLKRAGET:{
            return 0;
        }
        case BLKFRASET:{
            return 0;
        }
        case BLKFRAGET:{
            return 0;
        }
        case BLKSECTSET:{
            return 0;
        }
        case BLKSECTGET:{
            return 0;
        }
        case BLKSSZGET:{
            return ATA_get_sector_size_from_dev(blkdev);
        }
        default:
            return 0;
    }
}


static struct block_device_ops ata_ops = {
    .read  = ATA_blkdev_disk_read,
    .write = ATA_blkdev_disk_write,
    .ioctl = ATA_blkdev_ioctl,
};

int ata_init() {
    const char *names[4] = {"ata0", "ata1", "ata2", "ata3"};
    uint8_t drives[4] = {0xA0, 0xB0, 0xE0, 0xF0}; 

    for (int i = 0; i < 4; i++) {
        struct Ata_blkdev *ata = kmalloc(sizeof(struct Ata_blkdev));
        if (!ata) continue;

        ata->drive = drives[i];

        outb(ATA_DRIVE, ata->drive);
        ata_io_wait();
        outb(ATA_SECTOR_CNT, 0);
        outb(ATA_LBA_LOW, 0);
        outb(ATA_LBA_MID, 0);
        outb(ATA_LBA_HIGH, 0);
        outb(ATA_COMMAND, ATA_CMD_IDENTIFY);

        
        if (ata_wait_drq() != 0) {
            kfree(ata);
            continue; // perhaps no drive 
        }

        for (int j = 0; j < 256; j++)
            ata->info_data[j] = inw(ATA_DATA);

        
        ata->sector_count =
            ((uint32_t)ata->info_data[61] << 16) | ata->info_data[60];

        ata->sector_size =512;

        

        Register_Block_Device(
            names[i],
            (uint64_t)ata->sector_count * (uint64_t)ata->sector_size,
            ata->sector_size,
            ata_ops,
            ata
        );
    }

    return 0;
}