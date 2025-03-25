#include "pci.h"
#include "common.h"
#include "print.h"

u8 pci_cfg_r8(u64 offset) { return PCI_REG8(offset); }
u16 pci_cfg_r16(u64 offset) { return PCI_REG16(offset); }
u32 pci_cfg_r32(u64 offset) { return PCI_REG32(offset); }
u64 pci_cfg_r64(u64 offset) { return PCI_REG64(offset); }

void pci_cfg_w8(u32 offset, u8 val) { PCI_REG8(offset) = val; }
void pci_cfg_w16(u32 offset, u16 val) { PCI_REG16(offset) = val; }
void pci_cfg_w32(u32 offset, u32 val) { PCI_REG32(offset) = val; }

u64 pci_device_probe(u16 vendor_id, u16 device_id) {
  u32 pci_id = (((u32)device_id) << 16) | vendor_id;
  u64 ret = 0;

  for (int bus = 0; bus < 255; bus++) {
    for (int dev = 0; dev < 32; dev++) {
      int func = 0;
      int offset = 0;
      u64 off = (bus << 20) | (dev << 15) | (func << 12) | (offset);
      volatile u32 *base = (volatile u32 *)PCI_ADDR(off);
      u32 id = base[0]; // device_id + vendor_id

      if (id != pci_id)
        continue;

      ret = off;

      // command and status register.
      // bit 0 : I/O access enable
      // bit 1 : memory access enable
      // bit 2 : enable mastering
      base[1] = 0b111;
      __sync_synchronize();

      for (int i = 0; i < 6; i++) {
        u32 old = base[4 + i];
        cprintf("bar%d origin value: 0x%x\t", i, old);
        if (old & 0x1) {
          cprintf("IO space\n");
          continue; // all my homies hate IO space
        } else {    // but we do use mem space
          cprintf("Mem space\t");
        }
        if (old & 0x4) {
          cprintf("64 bit with BAR%d\n", i + 1); // 64bit system mapping
          base[4 + i] = 0xffffffff;
          base[4 + i + 1] = 0xffffffff;
          __sync_synchronize();

          u64 sz = ((u64)base[4 + i + 1] << 32) | base[4 + i];
          sz = ~(sz & 0xfffffffffffffff0) + 1;
          cprintf("bar%d need size: 0x%x\n", i, sz);
          u64 mem_addr = pci_alloc_mmio(sz);
          // Write allocation size
          base[4 + i] = (u32)(mem_addr);
          base[4 + i + 1] = (u32)(mem_addr >> 32);
          cprintf("bar%d mem_addr: 0x%x\n", i, mem_addr);
          i++; // Skip next BAR because it takes up 2 of them
        } else {
          cprintf("32 bit\n");
          // writing all 1's to the BAR causes it to be
          // replaced with its size.
          base[4 + i] = 0xffffffff;
          __sync_synchronize();

          u32 sz = base[4 + i];
          sz = ~(sz & 0xfffffff0) + 1;
          cprintf("bar%d need size: 0x%x\n", i, sz);
          base[4 + i] = (u32)pci_alloc_mmio((u64)sz);
          cprintf("bar%d mem_addr: 0x%x\n", i, (u64)base[4 + i]);
        }
      }
    }
  }
  cprintf("vendor_id: 0x%x\n", vendor_id);
  cprintf("device_id: 0x%x\n", device_id);
  cprintf("bar_addr : 0x%x\n", ret); // ECAM 中的 offset
  return ret;
}

#define PAGE_ALIGN(sv) ((((u64)sz) + 0x0fff) & ~0x0fff)
u64 pci_alloc_mmio(u64 sz) {
  static u64 s_offset = 0; // Static so it persists across calls
  u64 addr =
      PCIE0_MMIO + s_offset; // Get an unused address in the PCI MMIO space
  s_offset +=
      PAGE_ALIGN(sz); // Increment the offset by the size of the allocation
  cprintf("addr: 0x%x, 0x%x\n", addr, s_offset);
  return addr;
}

void pci_config_read(void *target, u64 len, u64 offset) {
  volatile u8 *dst = (volatile u8 *)target;
  while (len) {
    *dst = PCI_REG8(offset);
    --len;
    ++dst;
    ++offset;
  }
}
