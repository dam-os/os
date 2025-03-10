#ifndef DISK_H
#define DISK_H

/* See Table 4.1 at
 * https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html
 */
#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR 0x10001000 // @TODO: get this from pci enumerator
#define VIRTIO_REG_MAGIC 0x00
#define VIRTIO_REG_VERSION 0x04
#define VIRTIO_REG_DEVICE_ID 0x08
#define VIRTIO_REG_VENDOR_ID 0x00c
#define VIRTIO_REG_DEVICE_FEATURES 0x010
#define VIRTIO_REG_DEVICE_FEATURES_SEL 0x014
#define VIRTIO_REG_QUEUE_SEL 0x30
#define VIRTIO_REG_QUEUE_NUM_MAX 0x34
#define VIRTIO_REG_QUEUE_NUM 0x38
#define VIRTIO_REG_QUEUE_ALIGN 0x3c
#define VIRTIO_REG_QUEUE_PFN 0x40
#define VIRTIO_REG_QUEUE_READY 0x44
#define VIRTIO_REG_QUEUE_NOTIFY 0x50
#define VIRTIO_REG_DEVICE_STATUS 0x70
#define VIRTIO_REG_DEVICE_CONFIG 0x100
#define VIRTIO_STATUS_ACK 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_FEAT_OK 8
#define VIRTQ_DESC_F_NEXT 1
#define VIRTQ_DESC_F_WRITE 2
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
#define VIRTIO_BLK_T_IN 0
#define VIRTIO_BLK_T_OUT 1

// Offset from 0x1000. Actual device IDs will be 0x1041, etc.
enum DeviceTypes {
  NETWORK_CARD = 0x41,
  BLOCK_DEVICE = 0x42,
  MEMORY_BALLOONING = 0x43,
  CONSOLE = 0x44,
  HOST = 0x45,
  ENTROPY_SOURCE = 0x46,
  TRANSPORT = 0x47
};
/* Maps enums above to string names */
static const char *DeviceTypeNames[] = {
    "NETWORK_CARD", "BLOCK_DEVICE",   "MEMORY_BALLOONING", "CONSOLE",
    "HOST",         "ENTROPY_SOURCE", "TRANSPORT",
};

void verify_disk(void);

#endif // !DISK_H
