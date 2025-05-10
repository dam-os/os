#include "disk.h"
#include "../lib/common.h"
#include "../lib/io.h"
#include "system.h"

u32 virtio_reg_read32(unsigned offset) {
  return *(((u32 *)VIRTIO_BLK_PADDR + offset));
}

u64 virtio_reg_read64(unsigned offset) {
  return *(((volatile u64 *)VIRTIO_BLK_PADDR + offset));
}

void virtio_reg_write32(unsigned offset, u32 value) {
  *(((volatile u32 *)VIRTIO_BLK_PADDR + offset)) = value;
}

void verify_disk(void) {
  cprintf("Checking disk specs at 0x%x\r\n", VIRTIO_BLK_PADDR);

  u32 magic = virtio_reg_read32(VIRTIO_REG_MAGIC);
  cprintf("virtio magic: 0x%x\r\n", magic);
  if (magic != 0x74726976)
    PANIC("virtio: Invalid magic (expected 0x74726976)");

  u32 version = virtio_reg_read32(VIRTIO_REG_VERSION);
  cprintf("virtio version: 0x%x\r\n", version);
  if (version == 0x1)
    print("WARNING: virtio: Legacy device detected (expected version 0x2, got "
          "0x1)\r\n");
  else if (version != 0x2)
    PANIC("virtio: Invalid version (expected 0x2)\r\n");

  u32 device_id = virtio_reg_read32(VIRTIO_REG_DEVICE_ID);
  cprintf("virtio device id: 0x%x\r\n", device_id);
  cprintf("virtio device type: %s\r\n", DeviceTypeNames[device_id]);

  u32 vendor_id = virtio_reg_read32(VIRTIO_REG_VENDOR_ID);
  cprintf("virtio vendor id: 0x%x\r\n", vendor_id);

  /*u32 device_features = virtio_reg_read32(VIRTIO_REG_DEVICE_FEATURES);*/
  /*cprintf("virtio device features: 0b%b\n",*/
  /*        device_features); // @TODO: parse bits*/
  /**/
  /*u32 device_features_selection =*/
  /*    virtio_reg_read32(VIRTIO_REG_DEVICE_FEATURES_SEL);*/
  /*cprintf("virtio selected features: 0b%b\n", device_features_selection);*/
}
