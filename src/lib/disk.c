#include "disk.h"
#include "common.h"
#include "pci.h"
#include "print.h"

#define BUS 0
#define DEVICE 16
#define FUNCTION 0

const char *get_device_type(uint16 device_id) {
  return DeviceTypeNames[(uint8)(device_id - 0x1000 - 0x41)];
}

uint32 read_register(uint8 reg) {
  return pci_read_32(BUS, DEVICE, FUNCTION, reg * 4);
}

uint16 read16(uint8 offset) {
  return pci_read_16(BUS, DEVICE, FUNCTION, offset);
}
uint8 read8(uint8 offset) { return pci_read_8(BUS, DEVICE, FUNCTION, offset); }

// https://wiki.osdev.org/PCI#Common_Header_Fields

void verify_disk(void) {
  print("\nReading registers of PCI VirtIO device\n\n");
  uint32 reg0 = read_register(0x0);
  uint32 reg1 = read_register(0x1);
  uint32 reg2 = read_register(0x2);
  uint32 reg3 = read_register(0x3);

  cprintf("Device ID: 0x%x\n", read16(0x0 + 2));
  cprintf("Device type: %s\n", get_device_type(read16(0x0 + 2)));
  cprintf("Vendor ID: 0x%x\n\n", read16(0x0));

  cprintf("Status: %d\n", read16(0x4 + 2));
  cprintf("Command: 0b%b\n\n", read16(0x4));

  cprintf("Class code: 0x%x\n", read8(0x8 + 3));
  cprintf("Subclass: 0x%x\n", read8(0x8 + 2));
  cprintf("Prog IF: 0x%x\n", read8(0x8 + 1));
  cprintf("Revision ID: %d\n\n", read8(0x8));

  cprintf("Subsystem ID: 0x%x\n", read16(0x2c + 2));
  cprintf("Subsystem Vendor ID: 0x%x\n", read16(0x2c));
}
