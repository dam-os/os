#include "pci.h"
#include "common.h"
#include "print.h"

#define PCI_CONFIG_BASE 0x30000000

#define ADDR(bus, device, function, offset)                                    \
  (PCI_CONFIG_BASE + (bus << 16) + (device << 11) + (function << 8) + offset)

// https://wiki.osdev.org/PCI#Enumerating_PCI_Buses
// Read a n-bit values from PCI configuration space
uint8 pci_read_8(uint8 bus, uint8 device, uint8 function, uint8 offset) {
  return *(volatile uint8 *)ADDR(bus, device, function, offset);
}
uint16 pci_read_16(uint8 bus, uint8 device, uint8 function, uint8 offset) {
  return *(volatile uint16 *)ADDR(bus, device, function, offset);
}
uint32 pci_read_32(uint8 bus, uint8 device, uint8 function, uint8 offset) {
  return *(volatile uint32 *)ADDR(bus, device, function, offset);
}
uint64 pci_read_64(uint8 bus, uint8 device, uint8 function, uint8 offset) {
  return *(volatile uint64 *)ADDR(bus, device, function, offset);
}

void pci_write_32(uint8 bus, uint8 device, uint8 function, uint8 offset,
                  uint32 value) {
  *(volatile uint32 *)ADDR(bus, device, function, offset) = value;
}

// Enumerate PCI buses, devices, and functions
void enumerate_pci() {
  for (uint8 bus = 0; bus < 255; bus++) { // Scan all PCI buses (0-255)
    for (uint8 device = 0; device < 32;
         device++) { // Each bus has 32 possible devices
      uint32 vendor_device_id = pci_read_32(bus, device, 0x00, 0x00);
      if (vendor_device_id != 0xFFFFFFFF) { // 0xFFFFFFFF means "no device"
        uint16 vendor_id = vendor_device_id & 0xFFFF;
        uint16 device_id = (vendor_device_id >> 16) & 0xFFFF;
        cprintf("PCI Device Found: Bus %d, Device %d, Function 0x00, Vendor: "
                "0x%x, Device: 0x%x\n",
                bus, device, vendor_id, device_id);
      }
    }
  }
}
