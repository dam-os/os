#include "disk.h"
#include "print.h"
#include "common.h"
#include "system.h"

uint32_t virtio_reg_read32(unsigned offset) {
    return *((uint32_t *) (VIRTIO_BLK_PADDR + offset));
}

uint64_t virtio_reg_read64(unsigned offset) {
    return *((volatile uint64_t *) (VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, uint32_t value) {
    *((volatile uint32_t *) (VIRTIO_BLK_PADDR + offset)) = value;
}

void verify_disk(void) {
  cprintf("Checking disk specs at 0x%x\n", VIRTIO_BLK_PADDR);

  uint32_t magic = virtio_reg_read32(VIRTIO_REG_MAGIC);
  cprintf("virtio magic: 0x%x\n", magic);
  if (magic != 0x74726976) PANIC("virtio: Invalid magic (expected 0x74726976)");

  uint32_t version = virtio_reg_read32(VIRTIO_REG_VERSION);
  cprintf("virtio version: 0x%x\n", version);
  if (version == 0x1) print("WARNING: virtio: Legacy device detected (expected version 0x2, got 0x1)\n");
  else if (version != 0x2) PANIC("virtio: Invalid version (expected 0x2)\n");

  uint32_t device_id = virtio_reg_read32(VIRTIO_REG_DEVICE_ID);
  cprintf("virtio device id: 0x%x\n", device_id);
  cprintf("virtio device type: %s\n", DeviceTypeNames[device_id]);
}
