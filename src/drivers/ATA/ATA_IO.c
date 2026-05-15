#include "ATA/ATA_IO.h"
#include "Logger.h"
#include "blkdev.h"
#include "drivers/drivers.h"
#include "helpers.h"
#include "ioctl.h"
#include "memory.h"
#include "asm-defs/asm.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define ATA_DEBUG 1

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

static int ata_wait_ready(void)
{
    for (int i = 0; i < 100000; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (s & ATA_SR_ERR) {
            Sys_log("ATA ERROR: %x\n", inb(ATA_ERROR));
            return -1;
        }

        if (!(s & ATA_SR_BSY))
            return 0;
    }

    Sys_log("ATA BSY timeout\n");
    return -2;
}

static int ata_wait_drq(void)
{
    for (int i = 0; i < 100000; i++) {
        uint8_t s = inb(ATA_STATUS);

        if (s & ATA_SR_ERR) {
            Sys_log("ATA ERROR: %x\n", inb(ATA_ERROR));
            return -1;
        }

        if (!(s & ATA_SR_BSY) && (s & ATA_SR_DRQ))
            return 0;
    }

    Sys_log("ATA DRQ timeout\n");
    return -2;
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

    if (ata_wait_drq() != 0)
        return false;

    for (int i = 0; i < 256; i++)
        inw(ATA_DATA);

    return true;
}




void ata_pio_read_sector(struct ata_blkdev *ata, uint32_t lba, uint8_t *buffer)
{
    if (ata_wait_ready() != 0)
        return;

    uint8_t drive = 0xE0 | ((lba >> 24) & 0x0F);
    outb(ATA_DRIVE, drive);
    ata_io_wait();

    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW,  (uint8_t)lba);
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, ATA_CMD_READ);

    if (ata_wait_drq() != 0)
        return;

    for (int i = 0; i < 256; i++) {
        uint16_t w = inw(ATA_DATA);
        buffer[i * 2]     = w & 0xFF;
        buffer[i * 2 + 1] = w >> 8;
    }
}




void ata_pio_write_sector(struct ata_blkdev *ata, uint32_t lba, const uint8_t *buffer)
{
    if (ata_wait_ready() != 0)
        return;

    uint8_t drive = 0xE0 | ((lba >> 24) & 0x0F);
    outb(ATA_DRIVE, drive);
    ata_io_wait();

    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW,  (uint8_t)lba);
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, ATA_CMD_WRITE);

    if (ata_wait_drq() != 0)
        return;

    for (int i = 0; i < 256; i++) {
        uint16_t w =
            buffer[i * 2] |
            (buffer[i * 2 + 1] << 8);

        outw(ATA_DATA, w);
    }

    ata_wait_ready();

    outb(ATA_COMMAND, ATA_CMD_FLUSH);
    ata_wait_ready();
}




int ATA_blkdev_disk_read(struct block_device *dev,
                         void* buffer,
                         size_t data_size,
                         loff_t read_addr)
{
    if (!dev || !buffer) return -1;

    uint8_t sector_buf[512];

    size_t remaining = data_size;
    size_t offset_buf = 0;

    uint32_t lba = read_addr / 512;
    uint32_t off = read_addr % 512;

    while (remaining > 0) {
        ata_pio_read_sector((struct ata_blkdev*)dev->private_data, lba, sector_buf);

        size_t c = 512 - off;
        if (c > remaining) c = remaining;

        memcpy((uint8_t*)buffer + offset_buf,
               sector_buf + off,
               c);

        remaining -= c;
        offset_buf += c;
        lba++;
        off = 0;
    }

    return data_size;
}




int ATA_blkdev_disk_write(struct block_device *dev,
                          const void *buf,
                          size_t data_size,
                          loff_t write_addr)
{
    if (!dev || !buf) return -1;

    uint8_t sector_buf[512];

    size_t remaining = data_size;
    size_t offset_buf = 0;

    uint32_t lba = write_addr / 512;
    uint32_t off = write_addr % 512;

    while (remaining > 0) {
        size_t c = 512 - off;
        if (c > remaining) c = remaining;

        if (off || c != 512) {
            ata_pio_read_sector((struct ata_blkdev*)dev->private_data, lba, sector_buf);
            memcpy(sector_buf + off,
                   (const uint8_t*)buf + offset_buf,
                   c);
            ata_pio_write_sector((struct ata_blkdev*)dev->private_data, lba, sector_buf);
        } else {
            ata_pio_write_sector((struct ata_blkdev*)dev->private_data, lba,
                                 (uint8_t*)buf + offset_buf);
        }

        remaining -= c;
        offset_buf += c;
        lba++;
        off = 0;
    }

    return data_size;
}




int ATA_blkdev_ioctl(struct block_device *blkdev, int op,...)
{
    switch (_IOC_NR(op)) {
        case BLKGETSIZE:
            return ((uint32_t*)blkdev->private_data)[60];
        case BLKSSZGET:
            return 512;
        default:
            return 0;
    }
}




static struct block_device_ops ata_ops = {
    .read  = ATA_blkdev_disk_read,
    .write = ATA_blkdev_disk_write,
    .ioctl = ATA_blkdev_ioctl,
};

const char *names[] = {"ata0", "ata1"};
uint8_t drives[] = {0xA0, 0xB0};

static struct ata_blkdev ata_devs[sizeof(drives)];

int ata_init() {


    for (int i = 0; i < 2; i++) {

        struct ata_blkdev *ata = &ata_devs[i];
        memset(ata, 0, sizeof(*ata));

        ata->drive = drives[i];

        outb(ATA_DRIVE, ata->drive);
        ata_io_wait();
        outb(ATA_SECTOR_CNT, 0);
        outb(ATA_LBA_LOW, 0);
        outb(ATA_LBA_MID, 0);
        outb(ATA_LBA_HIGH, 0);
        outb(ATA_COMMAND, ATA_CMD_IDENTIFY);

        if (ata_wait_drq() != 0)
            continue;

        for (int j = 0; j < 256; j++)
            ata->info_data[j] = inw(ATA_DATA);

        ata->sector_count =
            ((uint32_t)ata->info_data[61] << 16) | ata->info_data[60];

        ata->sector_size = 512;
    }

    return 0;
}

REGISTER_DRIVER_DEV(ata_drive, ata_init);

int ata_vfs_register_all(void) {
    for (int i = 0; i < 2; i++) {

        struct ata_blkdev *ata = &ata_devs[i];

        if (ata->sector_size == 0 || ata->sector_count == 0)
            continue;

        Register_Block_Device(
            names[i],
            (uint64_t)ata->sector_count * (uint64_t)ata->sector_size,
            ata->sector_size,
            &ata_ops,
            ata
        );
    }
}

REGISTER_DRIVER_FS(ata_drive_vfs, ata_vfs_register_all);