#ifndef DISK_H
#define DISK_H

/* See Table 4.1 at
 * https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html
 */
#define SECTOR_SIZE 512
#define VIRTQ_ENTRY_NUM 16
#define VIRTIO_DEVICE_BLK 2
#define VIRTIO_BLK_PADDR                                                       \
  0x10001000 /* Gotten from the flat device tree thingy. @TODO: Is the first   \
                disk always mapped to the smallest address? */
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

enum DeviceTypes {
  RESERVED_INVALID = 0,
  NETWORK_CARD = 1,
  BLOCK_DEVICE = 2,
  CONSOLE = 3,
  ENTROPY_SOURCE = 4,
  MEMORY_BALLOONING_TRADITIONAL = 5,
  IO_MEMORY = 6,
  RPMSG = 7,
  SCSI_HOST = 8,
  NINE_P_TRANSPORT = 9,
  MAC80211_WLAN = 10,
  RPROC_SERIAL = 11,
  VIRTIO_CAIF = 12,
  MEMORY_BALLOON = 13,
  GPU_DEVICE = 16,
  TIMER_CLOCK_DEVICE = 17,
  INPUT_DEVICE = 18,
  SOCKET_DEVICE = 19,
  CRYPTO_DEVICE = 20,
  SIGNAL_DISTRIBUTION_MODULE = 21,
  PSTORE_DEVICE = 22,
  IOMMU_DEVICE = 23,
  MEMORY_DEVICE = 24
};
/* Maps enums above to string names */
static const char *const DeviceTypeNames[] = {
    "RESERVED_INVALID", "NETWORK_CARD",
    "BLOCK_DEVICE",     "CONSOLE",
    "ENTROPY_SOURCE",   "MEMORY_BALLOONING_TRADITIONAL",
    "IO_MEMORY",        "RPMSG",
    "SCSI_HOST",        "NINE_P_TRANSPORT",
    "MAC80211_WLAN",    "RPROC_SERIAL",
    "VIRTIO_CAIF",      "MEMORY_BALLOON",
    "GPU_DEVICE",       "TIMER_CLOCK_DEVICE",
    "INPUT_DEVICE",     "SOCKET_DEVICE",
    "CRYPTO_DEVICE",    "SIGNAL_DISTRIBUTION_MODULE",
    "PSTORE_DEVICE",    "IOMMU_DEVICE",
    "MEMORY_DEVICE"};

void verify_disk(void);

#endif // !DISK_H
