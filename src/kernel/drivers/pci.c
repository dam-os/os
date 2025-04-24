#include "../lib/common.h"
#include "../lib/print.h"
#include "device_tree.h"

uptr PCI_CONFIG_BASE = NULL;

void init_pci(void) {
  fdt_node_t *node = find_node_by_name("pci@");
  PCI_CONFIG_BASE = get_node_addr(node->name);

  free_node(node);
}

// https://wiki.osdev.org/PCI#Enumerating_PCI_Buses
// Read a 32-bit value from PCI configuration space
uint32_t pci_read_word(uint8_t bus, uint8_t device, uint8_t function,
                       uint8_t offset) {
  volatile uint32_t *addr =
      (volatile uint32_t *)(PCI_CONFIG_BASE + (bus << 16) + (device << 11) +
                            (function << 8) + offset);
  return *addr;
}

void pci_write_word(uint8_t bus, uint8_t device, uint8_t function,
                    uint8_t offset, uint32_t value) {
  volatile uint32_t *addr =
      (volatile uint32_t *)(PCI_CONFIG_BASE + (bus << 16) + (device << 11) +
                            (function << 8) + offset);
  *addr = value;
}

uint32_t *pci_get_addr(uint8_t bus, uint8_t device, uint8_t function,
                       uint8_t offset) {
  uint32_t *addr = (uint32_t *)(PCI_CONFIG_BASE + (bus << 16) + (device << 11) +
                                (function << 8) + offset);
  return addr;
}

// Enumerate PCI buses, devices, and functions
void enumerate_pci() {
  for (uint8_t bus = 0; bus < 255; bus++) { // Scan all PCI buses (0-255)
    for (uint8_t device = 0; device < 32;
         device++) { // Each bus has 32 possible devices
      uint32_t vendor_device_id = pci_read_word(bus, device, 0x00, 0x00);
      if (vendor_device_id !=
          (uint32_t)0xFFFFFFFF) { // 0xFFFFFFFF means "no device"
        uint16_t vendor_id = vendor_device_id & 0xFFFF;
        uint16_t device_id = (vendor_device_id >> 16) & 0xFFFF;
        cprintf("PCI Device Found: Bus %d, Device %d, Function 0x00, Vendor: "
                "%d, Device: %d\n",
                bus, device, vendor_id, device_id);
      }
    }
  }
}
