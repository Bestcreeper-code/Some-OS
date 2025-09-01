#ifndef USB_H
#define USB_H

#include "addresses.h"
#include <stdint.h>

typedef struct {
    uint32_t interrupt_table[32];
    uint16_t frame_number;
    uint16_t pad1;
    uint32_t done_head;
    uint8_t  reserved[116];
} __attribute__((packed, aligned(256))) hcca_t;

typedef struct {
    uint8_t bInterfaceClass;    // 0x08
    uint8_t bInterfaceSubClass; // 0x06
    uint8_t bInterfaceProtocol; // 0x50
} interface_descriptor_t;

typedef struct {
    uint32_t control;
    uint32_t tail_td;
    uint32_t head_td;
    uint32_t next_ed;
} __attribute__((packed)) ohci_ed_t;

typedef struct {
    uint32_t control;
    uint32_t cbp;
    uint32_t next_td;
    uint32_t be;
} __attribute__((packed)) ohci_td_t;

typedef struct {
    uint8_t bmRequestType;
    uint8_t bRequest;
    uint16_t wValue;
    uint16_t wIndex;
    uint16_t wLength;
} __attribute__((packed)) usb_setup_packet_t;

typedef struct {
    uint8_t bLength;
    uint8_t bDescriptorType;
    uint16_t bcdUSB;
    uint8_t bDeviceClass;
    uint8_t bDeviceSubClass;
    uint8_t bDeviceProtocol;
    uint8_t bMaxPacketSize0;
    uint16_t idVendor;
    uint16_t idProduct;
    uint16_t bcdDevice;
    uint8_t iManufacturer;
    uint8_t iProduct;
    uint8_t iSerialNumber;
    uint8_t bNumConfigurations;
} __attribute__((packed)) usb_device_descriptor_t;

typedef struct {
    uint8_t ep_in;
    uint8_t ep_out;
} usb_bulk_endpoints_t;



typedef struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bmCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} __attribute__((packed)) cbw_t;

typedef struct {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
} __attribute__((packed)) csw_t;


typedef struct {
    int used;
    uint32_t mmio_base;
    uint8_t device_addr;
    uint8_t ep_in;
    uint8_t ep_out;
} __attribute__((packed)) usb_stick_t;//11b


#define mmio_read32(addr)      (*(volatile uint32_t *)(addr))
#define mmio_write32(addr, v)  (*(volatile uint32_t *)(addr) = (v))
#define OHCI_RH_PORT_STATUS(n) (0x54 + 4 * ((n) - 1))

#define USB_DESCRIPTOR_TYPE_DEVICE        1
#define USB_DESCRIPTOR_TYPE_CONFIGURATION 2

#define CBW_SIGNATURE 0x43425355
#define CSW_SIGNATURE 0x53425355

#define MAX_USB_STICKS 4

extern usb_stick_t* usb_sticks;

void ohci_setup(uint32_t mmio_base);

int usb_control_transfer(uint32_t mmio_base, uint8_t device_addr,
                         usb_setup_packet_t* setup,
                         uint8_t* data_buffer, uint16_t length);

int usb_bulk_out_transfer(uint32_t mmio_base, uint8_t device_addr,
                          uint8_t endpoint, uint8_t* data, uint32_t length);

int usb_bulk_in_transfer(uint32_t mmio_base, uint8_t device_addr,
                         uint8_t endpoint, uint8_t* buffer, uint32_t length);

int usb_enumerate_device(uint32_t mmio_base);

int usb_parse_bulk_endpoints(uint8_t* config_desc, size_t len,
                             usb_bulk_endpoints_t* out);

int usb_scsi_read_capacity(uint32_t mmio_base, uint8_t device_addr,
                           uint8_t ep_in, uint8_t ep_out,
                           uint32_t* out_block_count, uint32_t* out_block_size);

int usb_scsi_read10(uint32_t mmio_base, uint8_t device_addr,
                    uint8_t ep_in, uint8_t ep_out,
                    uint32_t lba, uint16_t num_blocks,
                    uint32_t block_size, uint8_t* out_buffer);

#endif // USB_H