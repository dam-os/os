#include "disk.h"
#include "common.h"
#include "pci.h"
#include "print.h"

#define BUS 0
#define DEVICE 16
#define FUNCTION 0

static virtio_blk_t virtio_block = {0};

// const char *get_device_type(uint16 device_id) {
//   return DeviceTypeNames[(uint8)(device_id - 0x1000 - 0x41)];
// }
//
// uint32 read_register(uint8 reg) {
//   return pci_read_32(BUS, DEVICE, FUNCTION, reg * 4);
// }
//
// uint32 read32(uint8 offset) {
//   return pci_read_32(BUS, DEVICE, FUNCTION, offset);
// }
// uint16 read16(uint8 offset) {
//   return pci_read_16(BUS, DEVICE, FUNCTION, offset);
// }
// uint8 read8(uint8 offset) { return pci_read_8(BUS, DEVICE, FUNCTION, offset);
// }
//
// void write8(uint8 offset, uint8 value) {
//   pci_write_8(BUS, DEVICE, FUNCTION, offset, value);
// }
//

static void *get_cfg_addr(u64 pci_base, struct virtio_pci_cap *cap) {
  // 64 bit uses both BAR4 and BAR5
  u64 reg = pci_base + PCI_ADDR_BAR0 + 4 * cap->bar;
  return (void *)((pci_cfg_r64(reg) & 0xFFFFFFFFFFFFFFF0) + cap->offset);
}

void virtio_pci_read_caps(virtio_blk_device_t *dev) {
  virtio_pci_cap_t cap;
  u64 pos = 0;

  pos = pci_cfg_r8(dev->base_addr + PCI_ADDR_CAP);
  cprintf("cap: 0x%x\n", dev->base_addr + PCI_ADDR_CAP);

  while (pos) {
    pos += dev->base_addr;
    pci_config_read(&cap, sizeof(cap), pos);

    if (cap.cap_vndr != PCI_CAP_ID_VNDR) {
      cprintf("@0x%x: skip, cap not VNDR id: 0x%x\n", pos, cap.cap_vndr);
      pos = cap.cap_next;
      continue;
    }

    cprintf("@0x%x cfg type: %d, bar: %d, offset: 0x%x, len: %d\n", pos,
            cap.cfg_type, cap.bar, cap.offset, cap.length);

    switch (cap.cfg_type) {
    case VIRTIO_PCI_CAP_COMMON_CFG:
      dev->common_cfg = get_cfg_addr(dev->base_addr, &cap);
      cprintf("common_cfg addr: 0x%x\n", (u64)dev->common_cfg);
      break;
    case VIRTIO_PCI_CAP_NOTIFY_CFG:
      pci_config_read(&dev->notify_off_multiplier, 4, pos + sizeof(cap));
      dev->notify_cfg = get_cfg_addr(dev->base_addr, &cap);
      cprintf("notify_cfg addr: 0x%x\n", (u64)dev->notify_cfg);
      break;
    case VIRTIO_PCI_CAP_ISR_CFG:
      dev->isr_cfg = get_cfg_addr(dev->base_addr, &cap);
      cprintf("isr_cfg addr: %x\n", (u64)dev->isr_cfg);

      break;
    case VIRTIO_PCI_CAP_DEVICE_CFG:
      dev->device_cfg = get_cfg_addr(dev->base_addr, &cap);
      cprintf("device_cfg addr: %x\n", (u64)dev->device_cfg);

      break;
    case VIRTIO_PCI_CAP_PCI_CFG:
      // We're not doing this one
      break;
    }

    // Next cap
    pos = cap.cap_next;
  }

  if (dev->common_cfg == NULL || dev->notify_cfg == NULL ||
      dev->isr_cfg == NULL || dev->device_cfg == NULL) {
    cprintf("virtio pci device missing crucial configs, device invalid\n");
    return;
  }

  cprintf("found modern virtio pci device.\n");
  cprintf("common cfg mapped at: %p\n", dev->common_cfg);
  cprintf("isr cfg mapped at %p\n", dev->isr_cfg);
  cprintf("device cfg mapped at: %p\n", dev->device_cfg);
  cprintf("notify base: %p, notify off multiplier: %d\n", dev->notify_cfg,
          dev->notify_off_multiplier);
}

u8 virtio_pci_get_status(virtio_blk_device_t *dev) {
  virtio_pci_common_cfg_t *cfg = dev->common_cfg;
  return PCI_REG8(&cfg->device_status);
}

void virtio_pci_set_status(virtio_blk_device_t *dev, u8 status) {
  virtio_pci_common_cfg_t *cfg = dev->common_cfg;
  PCI_REG8(&cfg->device_status) = status;
}

u64 virtio_pci_get_device_features(virtio_blk_device_t *dev) {
  virtio_pci_common_cfg_t *cfg = dev->common_cfg;

  PCI_REG32(&cfg->device_feature_select) = 0;
  u64 f1 = PCI_REG32(&cfg->device_feature);

  PCI_REG32(&cfg->device_feature_select) = 1;
  u64 f2 = PCI_REG32(&cfg->device_feature);

  return (f2 << 32) | f1;
}

void virtio_pci_set_driver_features(virtio_blk_device_t *dev, u64 features) {
  virtio_pci_common_cfg_t *cfg = dev->common_cfg;
  PCI_REG32(&cfg->driver_feature_select) = 0;
  __sync_synchronize();
  PCI_REG32(&cfg->driver_feature) = features & 0xFFFFFFFF;

  PCI_REG32(&cfg->driver_feature_select) = 1;
  __sync_synchronize();
  PCI_REG32(&cfg->driver_feature) = features >> 32;
}

u16 virtio_pci_get_queue_enable(virtio_blk_device_t *dev, int qid) {
  struct virtio_pci_common_cfg *cfg = dev->common_cfg;

  PCI_REG16(&cfg->queue_select) = qid;
  __sync_synchronize();

  return PCI_REG16(&cfg->queue_enable);
}

u16 virtio_pci_get_queue_size(virtio_blk_device_t *dev, int qid) {
  struct virtio_pci_common_cfg *cfg = dev->common_cfg;

  PCI_REG16(&cfg->queue_select) = qid;
  __sync_synchronize();

  return PCI_REG16(&cfg->queue_size);
}

void init_disk(void) {
  /**
   * 3.1.1 Driver Requirements: Device Initialization
   */
  virtio_blk_device_t dev;
  dev.base_addr = pci_device_probe(0x1af4, 0x1042);

  if (!dev.base_addr) {
    print("No disk found\n");
    return;
  }

  // Read capabilities
  virtio_pci_read_caps(&dev);

  u8 status = 0;
  // 1. Reset the device
  virtio_pci_set_status(&dev, status);

  // 2. Set ACKNOWLEDGE status bit
  status |= VIRTIO_STATUS_ACKNOWLEDGE;
  virtio_pci_set_status(&dev, status);

  // 3. Set DRIVER status bit
  status |= VIRTIO_STATUS_DRIVER;
  virtio_pci_set_status(&dev, status);

  // 4. Read features
  u64 features = virtio_pci_get_device_features(&dev);
  cprintf("device features: 0b%b\n", features);
  // Remove unsupported features
  features &= ~(1 << VIRTIO_BLK_F_RO);
  features &= ~(1 << VIRTIO_BLK_F_SCSI);
  features &= ~(1 << VIRTIO_BLK_F_FLUSH);
  features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
  features &= ~(1 << VIRTIO_BLK_F_MQ);
  features &= ~(1 << VIRTIO_F_ANY_LAYOUT);
  features &= ~(1 << VIRTIO_F_EVENT_IDX);
  features &= ~(1 << VIRTIO_F_INDIRECT_DESC);
  cprintf("driver features: 0x%x\n", features);
  virtio_pci_set_driver_features(&dev, features);

  // 5. Set FEATURES_OK status bit
  status |= VIRTIO_STATUS_FEATURES_OK;
  virtio_pci_set_status(&dev, status);

  // 6. Verify device states has FEATURES_OK
  status = virtio_pci_get_status(&dev);
  if (!(status & VIRTIO_STATUS_FEATURES_OK)) {
    cprintf("virtio disk FEATURES_OK unset");
    return;
  }

  // 7. initialise queue 0.
  int qnum = 0;
  int qsize = QUEUE_SIZE;
  // ensure queue 0 is not in use.
  if (virtio_pci_get_queue_enable(&dev, qnum)) {
    cprintf("virtio disk should not be ready");
    return;
  }

  // check maximum queue size.
  u32 max = virtio_pci_get_queue_size(&dev, qnum);
  cprintf("queue_0 max size: %d\n", max);
  if (max == 0) {
    cprintf("virtio disk has no queue 0");
    return;
  }
  if (max < qsize) {
    cprintf("virtio disk max queue too short");
    return;
  }
  virtio_block.qsize = max;

  // 初始化 vring 相关
  int r = virtio_vring_init(&gs_virtio_blk.vr, gs_blk_buf, sizeof(gs_blk_buf),
                            qsize, qnum);
  if (r) {
    printf("virtio_vring_init failed: %d\n", r);
    return r;
  }

  // 根据 vring 等初始化的内容进行配置
  // (1) set queue size.
  virtio_pci_set_queue_size(&gs_virtio_blk_hw, qnum, qsize);
  // (2) disable msix / enable msix
  // virtio_pci_set_config_msix(&gs_virtio_blk_hw, 0);
  // virtio_pci_set_queue_msix(&gs_virtio_blk_hw, qnum, 1);
  // virtio_pci_disable_config_msix(&gs_virtio_blk_hw);              // config
  // 需要一个中断 virtio_pci_disable_queue_msix(&gs_virtio_blk_hw, qnum); //
  // 每个 virtqueue 需要一个中断 (3) write physical addresses.
  virtio_pci_set_queue_addr(&gs_virtio_blk_hw, qnum, &gs_virtio_blk.vr);
  // (4) queue is ready.
  virtio_pci_set_queue_enable(&gs_virtio_blk_hw, qnum);
}
