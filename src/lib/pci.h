#ifndef PCI_H
#define PCI_H

#include "common.h"

#define PCI_ADDR_BAR0 0x10
#define PCI_ADDR_CAP 0x34

#define PCI_CAP_ID_VNDR 0x09

#define PCIE0_ECAM 0x30000000UL
#define PCIE0_MMIO 0x40000000UL

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA 0xCFC

#define PCI_ADDR(addr) (PCIE0_ECAM + (u64)(addr))
#define PCI_REG8(reg) (*(volatile u8 *)PCI_ADDR(reg))
#define PCI_REG16(reg) (*(volatile u16 *)PCI_ADDR(reg))
#define PCI_REG32(reg) (*(volatile u32 *)PCI_ADDR(reg))
#define PCI_REG64(reg) (*(volatile u64 *)PCI_ADDR(reg))

u8 pci_cfg_r8(u64 offset);
u16 pci_cfg_r16(u64 offset);
u32 pci_cfg_r32(u64 offset);
u64 pci_cfg_r64(u64 offset);

void pci_cfg_w8(u32 offset, u8 val);
void pci_cfg_w16(u32 offset, u16 val);
void pci_cfg_w32(u32 offset, u32 val);

u64 pci_device_probe(u16 vendor_id, u16 device_id);
u64 pci_alloc_mmio(u64 sz);
void pci_config_read(void *target, u64 len, u64 offset);

// uint8 pci_read_8(uint8 bus, uint8 device, uint8 function, uint8 offset);
// uint16 pci_read_16(uint8 bus, uint8 device, uint8 function, uint8 offset);
// uint32 pci_read_32(uint8 bus, uint8 device, uint8 function, uint8 offset);
// uint64 pci_read_64(uint8 bus, uint8 device, uint8 function, uint8 offset);
// void pci_write_8(uint8 bus, uint8 device, uint8 functin, uint8 offset,
//                  uint8 value);
// void pci_write_32(uint8 bus, uint8 device, uint8 function, uint8 offset,
//                   uint32 value);
// uint32 *pci_get_addr_32(uint8 bus, uint8 device, uint8 function, uint8
// offset); void enumerate_pci();
#endif // !PCI_H
