#include "headers/pci.h"
#include <stdio.h>

#define PCI_CLASS_SERIAL 0x0C
#define PCI_SUBCLASS_USB 0x03
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

                

                uint32_t header = pci_config_read32(bus, device, function, 0x0C);
                uint8_t header_type = (header >> 16) & 0xFF;
                if ((header_type & 0x80) == 0)
                    break; // single function device
            }
        }
    }
}
