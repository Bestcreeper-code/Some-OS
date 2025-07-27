#include "headers/console.h"  // for print()
#include "headers/ATA_IO.h"
#include "headers/asm.h"

#include <stdint.h>
#include "../FatFs/diskio.h"

static void ata_wait_busy() {
    while (inb(ATA_STATUS) & ATA_SR_BSY) { /* spin */ }
}

static void ata_wait_drq() {
    while (!(inb(ATA_STATUS) & ATA_SR_DRQ)) {
        // optionally add a timeout here to avoid infinite loop
    }
}

void ata_pio_write_sector(uint32_t lba, const uint8_t *buffer) {
    ata_wait_busy();

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));

    outb(ATA_COMMAND, 0x30);

    ata_wait_busy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        uint16_t word = buffer[i*2] | (buffer[i*2 + 1] << 8);
        outw(ATA_DATA, word);
    }

    ata_wait_busy();
}

void ata_pio_read_sector(uint32_t lba, uint8_t *buffer) {
    ata_wait_busy();

    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    outb(ATA_SECTOR_CNT, 1);
    outb(ATA_LBA_LOW, (uint8_t)(lba & 0xFF));
    outb(ATA_LBA_MID, (uint8_t)((lba >> 8) & 0xFF));
    outb(ATA_LBA_HIGH, (uint8_t)((lba >> 16) & 0xFF));

    outb(ATA_COMMAND, 0x20);

    ata_wait_busy();
    ata_wait_drq();

    for (int i = 0; i < 256; i++) {
        uint16_t word = inw(ATA_DATA);
        buffer[i*2] = word & 0xFF;
        buffer[i*2 + 1] = (word >> 8) & 0xFF;
    }
}

uint32_t ata_get_sector_count() {
    outb(0x1F6, 0xA0);         // master drive
    outb(0x1F7, 0xEC);         // Send IDENTIFY command

    if (inb(0x1F7) == 0) return 0;  // No drive

    while (inb(0x1F7) & 0x80);  // Wait for BSY to clear
    if (!(inb(0x1F7) & 0x08)) return 0;  // DRQ must be set

    uint16_t data[256];
    for (int i = 0; i < 256; i++) {
        data[i] = inw(0x1F0);
    }

    return (uint32_t)data[60] | ((uint32_t)data[61] << 16);
}

#define ATA_CMD_IDENTIFY 0xEC
#define ATA_SR_BSY 0x80
#define ATA_SR_DRDY 0x40
#define ATA_SR_ERR 0x01

bool ata_drive_exists() {
    return true;
    outb(0x1F6, 0xA0);                // Select master drive
    outb(0x1F7, ATA_CMD_IDENTIFY);   // Send IDENTIFY DEVICE command

    // Wait for status to be non-zero or timeout
    uint8_t status = inb(0x1F7);
    for (int i = 0; i < 100000; i++) {  // Simple timeout loop
        status = inb(0x1F7);
        if (status != 0)
            break;
    }
    if (status == 0) return false;      // No device

    // Wait for BSY to clear
    while (status & ATA_SR_BSY)
        status = inb(0x1F7);

    // Check for ERR bit
    if (status & ATA_SR_ERR)
        return false;

    // Wait for DRQ bit (data ready)
    while (!(status & ATA_SR_DRQ))
        status = inb(0x1F7);

    return true;
}
