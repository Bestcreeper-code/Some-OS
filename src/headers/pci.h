#ifndef PCI_H
#define PCI_H

#include <stdint.h>

#define PCI_CLASS_SERIAL     0x0C
#define PCI_SUBCLASS_USB     0x03
#define PCI_PROGIF_XHCI      0x30

#define OHCI_REVISION          0x00
#define OHCI_CONTROL           0x04
#define OHCI_COMMAND_STATUS    0x08
#define OHCI_INTERRUPT_STATUS  0x0C
#define OHCI_INTERRUPT_ENABLE  0x10
#define OHCI_HCCA              0x18
#define OHCI_CONTROL_HEAD_ED   0x20
#define OHCI_BULK_HEAD_ED      0x24
#define OHCI_FM_INTERVAL       0x34
#define OHCI_PERIODIC_START    0x40
#define OHCI_LS_THRESHOLD      0x44
#define OHCI_RH_DESCRIPTOR_A   0x48
#define OHCI_RH_STATUS         0x50


uint32_t pci_config_read32(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
uint16_t pci_config_read16(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_scan();




#endif // PCI_H
