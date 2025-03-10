#ifndef PCI_H
#define PCI_H

#include "common.h"

#define CONFIG_ADDRESS = 0xCF8
#define CONFIG_DATA = 0xCFC

uint8 pci_read_8(uint8 bus, uint8 device, uint8 function, uint8 offset);
uint16 pci_read_16(uint8 bus, uint8 device, uint8 function, uint8 offset);
uint32 pci_read_32(uint8 bus, uint8 device, uint8 function, uint8 offset);
uint64 pci_read_64(uint8 bus, uint8 device, uint8 function, uint8 offset);
void pci_write_32(uint8 bus, uint8 device, uint8 function, uint8 offset,
                  uint32 value);
void enumerate_pci();
#endif // !PCI_H
