#include "headers/pci.h"
#include "headers/io.h"
#include "headers/Logger.h"
// #include "headers/usb.h"

#define PCI_CLASS_SERIAL 0x0C
#define PCI_SUBCLASS_USB 0x03


#define PCI_PROGIF_UHCI  0x00
#define PCI_PROGIF_OHCI  0x10
#define PCI_PROGIF_EHCI  0x20
#define PCI_PROGIF_XHCI  0x30

void pci_scan() {
    for (uint8_t bus = 0; bus < 256; bus++) {
        for (uint8_t device = 0; device < 32; device++) {
            for (uint8_t function = 0; function < 8; function++) {
                uint16_t vendor_id = pci_config_read16(bus, device, function, 0x00);
                if (vendor_id == 0xFFFF) continue;

                uint32_t class_info = pci_config_read32(bus, device, function, 0x08);
                uint8_t class_code = (class_info >> 24) & 0xFF;
                uint8_t subclass = (class_info >> 16) & 0xFF;
                uint8_t prog_if = (class_info >> 8) & 0xFF;

                uint32_t bar = pci_config_read32(bus, device, function, 0x10);
                // Mask off the I/O/memory indicator bits
                uint32_t mmio_base = bar & ~0xF;


                if (class_code == PCI_CLASS_SERIAL && subclass == PCI_SUBCLASS_USB) {
                    printf("USB Host Controller found at %02x:%02x.%x - ", bus, device, function);

                    switch (prog_if) {
                        case PCI_PROGIF_UHCI:
                            Sys_log("[PCI]UHCI detected\n");
                            break;
                        case PCI_PROGIF_OHCI:
                            Sys_log("[PCI]OHCI detected\n");
                            break;
                        case PCI_PROGIF_EHCI:
                            Sys_log("[PCI]EHCI detected\n");
                            break;
                        case PCI_PROGIF_XHCI:
                            Sys_log("[PCI]xHCI detected\n");
                            // xhci_setup(mmio_base);
                            break;
                        default:
                            Sys_log("[PCI]Unknown ProgIF 0x%02x\n", prog_if);
                            break;
                    }
                }

                uint32_t header = pci_config_read32(bus, device, function, 0x0C);
                uint8_t header_type = (header >> 16) & 0xFF;
                if ((header_type & 0x80) == 0)
                    break; // single function device, no more functions here
            }
        }
    }
}



#include <stdint.h>

// PCI config address port and data port
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

// Build PCI config address for a given bus/device/function/register
uint32_t pci_config_address(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    return (uint32_t)(
        (1U << 31) |                 // Enable bit
        ((uint32_t)bus << 16) |
        ((uint32_t)device << 11) |
        ((uint32_t)function << 8) |
        (offset & 0xFC)              // Align offset to 4 bytes
    );
}

uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_config_address(bus, device, function, offset);
    asm volatile ("outl %0, %1" : : "a"(address), "Nd"(PCI_CONFIG_ADDRESS));
    uint32_t value;
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(PCI_CONFIG_DATA));
    return value;
}

uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_config_address(bus, device, function, offset);
    asm volatile ("outl %0, %1" : : "a"(address), "Nd"(PCI_CONFIG_ADDRESS));
    uint32_t value;
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(PCI_CONFIG_DATA));
    // Extract 16 bits depending on offset
    uint16_t word = (value >> ((offset & 2) * 8)) & 0xFFFF;
    return word;
}

static inline uint8_t pci_config_read8(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset) {
    uint32_t address = pci_config_address(bus, device, function, offset);
    asm volatile ("outl %0, %1" : : "a"(address), "Nd"(PCI_CONFIG_ADDRESS));
    uint32_t value;
    asm volatile ("inl %1, %0" : "=a"(value) : "Nd"(PCI_CONFIG_DATA));
    uint8_t byte = (value >> ((offset & 3) * 8)) & 0xFF;
    return byte;
}
