#include "disk.h"
#include "common.h"
#include "pci.h"
#include "print.h"

uint32 virtio_reg_read32(unsigned offset) {
  return *((uint32 *)(VIRTIO_BLK_PADDR + offset));
}

uint64 virtio_reg_read64(unsigned offset) {
  return *((volatile uint64 *)(VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, uint32 value) {
  *((volatile uint32 *)(VIRTIO_BLK_PADDR + offset)) = value;
}

void verify_disk(void) {
  uint16 vendor_id = pci_read_16(0, 16, 0x00, 0x00);
  cprintf("Vendor ID: 0x%x\n", vendor_id);

  uint16 device_id = pci_read_16(0, 16, 0x00, 0x02);
  cprintf("Device ID: 0x%x\n", device_id);
}
