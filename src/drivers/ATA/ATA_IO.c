#include "ATA_IO.h"
#include <asm.h>
#include <stdint.h>
#include <stdbool.h>

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

uint32_t ata_get_sector_count()
{
    outb(ATA_DRIVE, 0xA0);
    ata_io_wait();

    outb(ATA_SECTOR_CNT, 0);
    outb(ATA_LBA_LOW, 0);
    outb(ATA_LBA_MID, 0);
    outb(ATA_LBA_HIGH, 0);

    outb(ATA_COMMAND, ATA_CMD_IDENTIFY);

    if (ata_wait_drq() != 0)
        return 0;

    uint16_t data[256];

    for (int i = 0; i < 256; i++)
        data[i] = inw(ATA_DATA);

    return ((uint32_t)data[61] << 16) | data[60];
}