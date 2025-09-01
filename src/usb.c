#include "headers/usb.h"

#include <stdint.h>
#include <stddef.h>
#include "headers/string.h"
#include "headers/pci.h"
#include "headers/time.h"



#define CBW_SIGNATURE 0x43425355
#define CSW_SIGNATURE 0x53425355

usb_stick_t* usb_sticks = USB_STICKS_DATA_ARRAY_ADDRESS;

volatile hcca_t hcca;

void ohci_setup(uint32_t mmio_base) {
    memset((void*)&hcca, 0, sizeof(hcca));

    // Reset controller
    mmio_write32(mmio_base + OHCI_COMMAND_STATUS, (1 << 0));
    while (mmio_read32(mmio_base + OHCI_COMMAND_STATUS) & (1 << 0));

    mmio_write32(mmio_base + OHCI_HCCA, (uint32_t)&hcca);
    mmio_write32(mmio_base + OHCI_CONTROL_HEAD_ED, 0);
    mmio_write32(mmio_base + OHCI_BULK_HEAD_ED, 0);
    mmio_write32(mmio_base + OHCI_FM_INTERVAL, 0x2EDF);

    uint32_t control = mmio_read32(mmio_base + OHCI_CONTROL);
    control &= ~0x3F;
    control |= (0x01 << 6); // USB Operational
    mmio_write32(mmio_base + OHCI_CONTROL, control);

    mmio_write32(mmio_base + OHCI_INTERRUPT_ENABLE, 0x8000000F);

    mmio_write32(mmio_base + OHCI_RH_STATUS, mmio_read32(mmio_base + OHCI_RH_STATUS) | (1 << 8));
    mmio_write32(mmio_base + OHCI_RH_PORT_STATUS(1), (1 << 8));

    for (int i = 0; i < 100; i++) {
        uint32_t port_status = mmio_read32(mmio_base + OHCI_RH_PORT_STATUS(1));
        if (port_status & (1 << 0)) {
            printf("Device connected on OHCI port 1\n");

            mmio_write32(mmio_base + OHCI_RH_PORT_STATUS(1), (1 << 4));
            for (int j = 0; j < 100000; j++) asm volatile ("pause");
            break;
        }
        sleep(10);
    }

    int addr = usb_enumerate_device(mmio_base);
    if (addr > 0) {
        printf("USB mass storage device ready at address %d\n", addr);
    }
}

int usb_control_transfer(uint32_t mmio_base, uint8_t device_addr,
                         usb_setup_packet_t* setup,
                         uint8_t* data_buffer, uint16_t length) {
    static ohci_ed_t ed __attribute__((aligned(16)));
    static ohci_td_t td_setup __attribute__((aligned(16)));
    static ohci_td_t td_data __attribute__((aligned(16)));
    static ohci_td_t td_status __attribute__((aligned(16)));

    memset(&ed, 0, sizeof(ed));
    memset(&td_setup, 0, sizeof(td_setup));
    memset(&td_data, 0, sizeof(td_data));
    memset(&td_status, 0, sizeof(td_status));

    // Prepare setup TD
    td_setup.control = (0x2 << 19) | (0x1 << 24) | (0x1 << 21); // Setup PID, delay interrupt
    td_setup.cbp = (uint32_t)setup;
    td_setup.next_td = (uint32_t)&td_data;
    td_setup.be = td_setup.cbp + sizeof(usb_setup_packet_t) - 1;

    // Prepare data TD (IN or OUT)
    if (length > 0) {
        td_data.control = (((setup->bmRequestType & 0x80) ? 0x1 : 0x0) << 19) |
                          (0x1 << 24); // PID: IN/OUT, delay int
        td_data.cbp = (uint32_t)data_buffer;
        td_data.next_td = (uint32_t)&td_status;
        td_data.be = td_data.cbp + length - 1;
    } else {
        td_data.next_td = (uint32_t)&td_status;
        td_data.cbp = 0;
        td_data.be = 0;
    }

    // Status TD (always opposite direction of data)
    td_status.control = (((setup->bmRequestType & 0x80) ? 0x0 : 0x1) << 19) |
                        (0x1 << 24); // PID: opposite, delay int
    td_status.cbp = 0;
    td_status.next_td = 0;
    td_status.be = 0;

    // Configure ED
    ed.control = (device_addr << 0) | (0 << 7) | (0 << 11); // addr, endpoint=0, speed=full
    ed.tail_td = (uint32_t)&td_status;
    ed.head_td = (uint32_t)&td_setup;
    ed.next_ed = 0;

    // Point control list to ED
    mmio_write32(mmio_base + OHCI_CONTROL_HEAD_ED, (uint32_t)&ed);

    // Enable control list processing
    uint32_t ctrl = mmio_read32(mmio_base + OHCI_CONTROL);
    ctrl |= (1 << 4); // Control list enable
    mmio_write32(mmio_base + OHCI_CONTROL, ctrl);

    // Wait for transfer to complete
    for (int i = 0; i < 100000; i++) {
        if (td_status.control & (1 << 18)) { // Check if ConditionCode != not accessed
            break;
        }
    }

    // Check for success (condition code = 0)
    uint32_t cc = (td_status.control >> 28) & 0xF;
    return cc == 0 ? 0 : -1;
}




int usb_enumerate_device(uint32_t mmio_base) {
    uint8_t device_addr = 1; // Assign new address
    uint8_t buffer[256];

    usb_setup_packet_t setup;

    // STEP 1: Get device descriptor (first 8 bytes only)
    setup.bmRequestType = 0x80;
    setup.bRequest = 6; // GET_DESCRIPTOR
    setup.wValue = (USB_DESCRIPTOR_TYPE_DEVICE << 8) | 0;
    setup.wIndex = 0;
    setup.wLength = 8;

    if (usb_control_transfer(mmio_base, 0, &setup, buffer, 8) < 0) {
        printf("Failed to get device descriptor (8 bytes)\n");
        return -1;
    }

    uint8_t max_packet_size = buffer[7]; // bMaxPacketSize0

    // STEP 2: Set device address
    setup.bmRequestType = 0x00;
    setup.bRequest = 5; // SET_ADDRESS
    setup.wValue = device_addr;
    setup.wIndex = 0;
    setup.wLength = 0;

    if (usb_control_transfer(mmio_base, 0, &setup, NULL, 0) < 0) {
        printf("Failed to set device address\n");
        return -1;
    }

    sleep(10); // Wait a little after setting address

    // STEP 3: Get full device descriptor (now using new address)
    setup.bmRequestType = 0x80;
    setup.bRequest = 6;
    setup.wValue = (USB_DESCRIPTOR_TYPE_DEVICE << 8) | 0;
    setup.wIndex = 0;
    setup.wLength = 18;

    if (usb_control_transfer(mmio_base, device_addr, &setup, buffer, 18) < 0) {
        printf("Failed to get full device descriptor\n");
        return -1;
    }

    usb_device_descriptor_t* dev_desc = (usb_device_descriptor_t*)buffer;

    printf("USB Device: VID=0x%04x PID=0x%04x Class=0x%02x\n",
           dev_desc->idVendor, dev_desc->idProduct, dev_desc->bDeviceClass);

    // STEP 4: Get configuration descriptor
    setup.bmRequestType = 0x80;
    setup.bRequest = 6; // GET_DESCRIPTOR
    setup.wValue = (USB_DESCRIPTOR_TYPE_CONFIGURATION << 8) | 0;
    setup.wIndex = 0;
    setup.wLength = 64;

    if (usb_control_transfer(mmio_base, device_addr, &setup, buffer, 64) < 0) {
        printf("Failed to get configuration descriptor\n");
        return -1;
    }

    uint8_t interface_class = buffer[14]; // bInterfaceClass

    if (interface_class != 0x08) {
        printf("Not a mass storage device (class=0x%02x)\n", interface_class);
        return -1;
    }

    printf("Mass storage device detected!\n");

    usb_bulk_endpoints_t bulk_eps;
    if (usb_parse_bulk_endpoints(buffer, 64, &bulk_eps) < 0) {
        printf("Failed to find bulk endpoints\n");
        return -1;
    }
    printf("Bulk IN = 0x%02x, OUT = 0x%02x\n", bulk_eps.ep_in, bulk_eps.ep_out);


    // STEP 5: Set configuration
    uint8_t config_value = buffer[5]; // bConfigurationValue
    setup.bmRequestType = 0x00;
    setup.bRequest = 9; // SET_CONFIGURATION
    setup.wValue = config_value;
    setup.wIndex = 0;
    setup.wLength = 0;

    if (usb_control_transfer(mmio_base, device_addr, &setup, NULL, 0) < 0) {
        printf("Failed to set configuration\n");
        return -1;
    }

    return device_addr;
}


int usb_parse_bulk_endpoints(uint8_t* config_desc, size_t len, usb_bulk_endpoints_t* out) {
    size_t i = 0;
    out->ep_in = 0;
    out->ep_out = 0;

    while (i + 1 < len) {
        uint8_t bLength = config_desc[i];
        uint8_t bDescriptorType = config_desc[i + 1];

        if (bLength == 0) break;

        if (bDescriptorType == 0x05) { // Endpoint Descriptor
            uint8_t epAddr = config_desc[i + 2];
            uint8_t attr = config_desc[i + 3];

            if ((attr & 0x03) == 0x02) { // Bulk
                if (epAddr & 0x80)
                    out->ep_in = epAddr;
                else
                    out->ep_out = epAddr;
            }
        }

        i += bLength;
    }

    return (out->ep_in && out->ep_out) ? 0 : -1;
}


int usb_bulk_out_transfer(uint32_t mmio_base, uint8_t device_addr,
                          uint8_t endpoint, uint8_t* data, uint32_t length) {
    static ohci_ed_t ed __attribute__((aligned(16)));
    static ohci_td_t td __attribute__((aligned(16)));

    memset(&ed, 0, sizeof(ed));
    memset(&td, 0, sizeof(td));

    td.control = (0x0 << 19) | (0x1 << 24); // OUT PID, delay int
    td.cbp = (uint32_t)data;
    td.next_td = 0;
    td.be = td.cbp + length - 1;

    ed.control = (device_addr << 0) | ((endpoint & 0xF) << 7) | (0 << 11); // endpoint, speed = full
    ed.tail_td = (uint32_t)&td;
    ed.head_td = (uint32_t)&td;
    ed.next_ed = 0;

    mmio_write32(mmio_base + OHCI_BULK_HEAD_ED, (uint32_t)&ed);

    // Enable bulk list processing
    uint32_t ctrl = mmio_read32(mmio_base + OHCI_CONTROL);
    ctrl |= (1 << 5); // Bulk list enable
    mmio_write32(mmio_base + OHCI_CONTROL, ctrl);

    for (int i = 0; i < 100000; i++) {
        if (((td.control >> 28) & 0xF) != 0xF) break;
    }

    uint32_t cc = (td.control >> 28) & 0xF;
    return cc == 0 ? 0 : -1;
}

int usb_bulk_in_transfer(uint32_t mmio_base, uint8_t device_addr,
                         uint8_t endpoint, uint8_t* buffer, uint32_t length) {
    static ohci_ed_t ed __attribute__((aligned(16)));
    static ohci_td_t td __attribute__((aligned(16)));

    memset(&ed, 0, sizeof(ed));
    memset(&td, 0, sizeof(td));

    td.control = (0x1 << 19) | (0x1 << 24); // IN PID, delay int
    td.cbp = (uint32_t)buffer;
    td.next_td = 0;
    td.be = td.cbp + length - 1;

    ed.control = (device_addr << 0) | ((endpoint & 0xF) << 7) | (0 << 11);
    ed.tail_td = (uint32_t)&td;
    ed.head_td = (uint32_t)&td;
    ed.next_ed = 0;

    mmio_write32(mmio_base + OHCI_BULK_HEAD_ED, (uint32_t)&ed);

    // Enable bulk list processing
    uint32_t ctrl = mmio_read32(mmio_base + OHCI_CONTROL);
    ctrl |= (1 << 5); // Bulk list enable
    mmio_write32(mmio_base + OHCI_CONTROL, ctrl);

    for (int i = 0; i < 100000; i++) {
        if (td.control & (1 << 18)) break;
    }

    uint32_t cc = (td.control >> 28) & 0xF;
    return cc == 0 ? 0 : -1;
}

int usb_scsi_read_capacity(uint32_t mmio_base, uint8_t device_addr,
                           uint8_t ep_in, uint8_t ep_out,
                           uint32_t* out_block_count, uint32_t* out_block_size) {
    cbw_t cbw = {0};
    csw_t csw = {0};
    uint8_t buffer[8] = {0};

    // Fill CBW
    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 0x12345678;
    cbw.dCBWDataTransferLength = 8;
    cbw.bmCBWFlags = 0x80; // IN
    cbw.bCBWLUN = 0;
    cbw.bCBWCBLength = 10;

    cbw.CBWCB[0] = 0x25; // READ CAPACITY(10)

    // Send CBW
    if (usb_bulk_out_transfer(mmio_base, device_addr, ep_out, (uint8_t*)&cbw, sizeof(cbw)) < 0) {
        printf("CBW transfer failed\n");
        return -1;
    }

    // Receive data
    if (usb_bulk_in_transfer(mmio_base, device_addr, ep_in, buffer, sizeof(buffer)) < 0) {
        printf("Data IN transfer failed\n");
        return -1;
    }

    // Receive CSW
    if (usb_bulk_in_transfer(mmio_base, device_addr, ep_in, (uint8_t*)&csw, sizeof(csw)) < 0) {
        printf("CSW transfer failed\n");
        return -1;
    }

    // Check CSW
    if (csw.dCSWSignature != CSW_SIGNATURE || csw.bCSWStatus != 0) {
        printf("CSW error or invalid\n");
        return -1;
    }

    // Parse result
    *out_block_count = __builtin_bswap32(*(uint32_t*)&buffer[0]) + 1;
    *out_block_size = __builtin_bswap32(*(uint32_t*)&buffer[4]);

    printf("Device Capacity: %u blocks, block size = %u bytes\n", *out_block_count, *out_block_size);

    return 0;
}

int usb_scsi_read10(uint32_t mmio_base, uint8_t device_addr,
                    uint8_t ep_in, uint8_t ep_out,
                    uint32_t lba, uint16_t num_blocks,
                    uint32_t block_size, uint8_t* out_buffer) {
    cbw_t cbw = {0};
    csw_t csw = {0};

    uint32_t transfer_len = num_blocks * block_size;

    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = 0xdeadbeef;
    cbw.dCBWDataTransferLength = transfer_len;
    cbw.bmCBWFlags = 0x80; // IN
    cbw.bCBWLUN = 0;
    cbw.bCBWCBLength = 10;

    cbw.CBWCB[0] = 0x28; // SCSI READ(10)
    cbw.CBWCB[2] = (lba >> 24) & 0xFF;
    cbw.CBWCB[3] = (lba >> 16) & 0xFF;
    cbw.CBWCB[4] = (lba >> 8) & 0xFF;
    cbw.CBWCB[5] = lba & 0xFF;
    cbw.CBWCB[7] = (num_blocks >> 8) & 0xFF;
    cbw.CBWCB[8] = num_blocks & 0xFF;

    if (usb_bulk_out_transfer(mmio_base, device_addr, ep_out, (uint8_t*)&cbw, sizeof(cbw)) < 0)
        return -1;

    if (usb_bulk_in_transfer(mmio_base, device_addr, ep_in, out_buffer, transfer_len) < 0)
        return -1;

    if (usb_bulk_in_transfer(mmio_base, device_addr, ep_in, (uint8_t*)&csw, sizeof(csw)) < 0)
        return -1;

    return (csw.dCSWSignature == CSW_SIGNATURE && csw.bCSWStatus == 0) ? 0 : -1;
}
