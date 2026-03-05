#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CLASS_SERIAL     0x0C
#define PCI_SUBCLASS_USB     0x03
#define PCI_PROGIF_XHCI      0x30



uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_scan();




#endif // PCI_H
