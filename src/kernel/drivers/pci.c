#include "../lib/common.h"
#include "../lib/io.h"
#include "device_tree.h"

void* PCI_CONFIG_BASE = NULL;

void init_pci(void) {
  char *pci_node = match_node("pci@");
  if (pci_node == NULL) {
    print("[pci] PCI not found\r\n");
    return;
  }
  PCI_CONFIG_BASE = (void *)get_node_addr(pci_node);
  print("[pci] PCI config base initialised.\r\n");
}

// https://wiki.osdev.org/PCI#Enumerating_PCI_Buses
// Read a 32-bit value from PCI configuration space
u32 pci_read_word(u8 bus, u8 device, u8 function, u8 offset) {
  volatile u32 *addr =
      (volatile u32 *)(PCI_CONFIG_BASE + (bus << 16) + (device << 11) +
                       (function << 8) + offset);
  return *addr;
}

void pci_write_word(u8 bus, u8 device, u8 function, u8 offset, u32 value) {
  volatile u32 *addr =
      (volatile u32 *)(PCI_CONFIG_BASE + (bus << 16) + (device << 11) +
                       (function << 8) + offset);
  *addr = value;
}

u32 *pci_get_addr(u8 bus, u8 device, u8 function, u8 offset) {
  u32 *addr = (u32 *)(PCI_CONFIG_BASE + (bus << 16) + (device << 11) +
                      (function << 8) + offset);
  return addr;
}

// Enumerate PCI buses, devices, and functions
void enumerate_pci() {
  for (u8 bus = 0; bus < 255; bus++) { // Scan all PCI buses (0-255)
    for (u8 device = 0; device < 32;
         device++) { // Each bus has 32 possible devices
      u32 vendor_device_id = pci_read_word(bus, device, 0x00, 0x00);
      if (vendor_device_id != (u32)0xFFFFFFFF) { // 0xFFFFFFFF means "no device"
        u16 vendor_id = vendor_device_id & 0xFFFF;
        u16 device_id = (vendor_device_id >> 16) & 0xFFFF;
        cprintf("PCI Device Found: Bus %d, Device %d, Function 0x00, Vendor: "
                "%d, Device: %d\r\n",
                bus, device, vendor_id, device_id);
      }
    }
  }
}

void* get_pci_config_base(void) { return PCI_CONFIG_BASE; }
