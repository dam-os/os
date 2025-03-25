#ifndef DISK_H
#define DISK_H

#include "../lib/common.h"
#include "../memory/paging.h"

#define QUEUE_SIZE (16)

// https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html#x1-320005
struct virtq_desc {
  uint64 addr;   // Address (guest-physical)
  uint32 length; // Length

// This marks a buffer as continuing via the next field.
#define VIRTQ_DESC_F_NEXT 1
// This marks a buffer as device write-only (otherwise device read-only).
#define VIRTQ_DESC_F_WRITE 2
// This means the buffer contains a list of buffer descriptors.
#define VIRTQ_DESC_F_INDIRECT 4

  uint16 flags; // Flags as indicated above
  uint16 next;  // Next field if flags & NEXT
} __attribute__((packed));
typedef struct virtq_desc virtq_desc_t;

// https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html#x1-380006
struct virtq_avail {
#define VIRTQ_AVAIL_F_NO_INTERRUPT 1
  uint16 flags;
  // Indicates where the driver would put the next descriptor entry in the ring
  // (module the queue size). This starts at 0, and increases.
  uint16 idx;
  uint16 ring[QUEUE_SIZE];
  uint16 used_event; // Only if VIRTIO_F_EVENT_IDX
} __attribute__((packed));
typedef struct virtq_avail virtq_avail_t;

// https://docs.oasis-open.org/virtio/virtio/v1.1/csprd01/virtio-v1.1-csprd01.html#x1-430008
struct virtq_used_elem {
  // Index of start of used descriptor chain.
  uint32 id;
  // Total length of the descriptor chain which was used (written to).
  uint32 len;
} __attribute__((packed));
typedef struct virtq_used_elem virtq_used_elem_t;

struct virtq_used {
#define VIRTQ_USED_F_NO_NOTIFY 1
  uint16 flags;

  uint16 idx;

  virtq_used_elem_t ring[QUEUE_SIZE];
  uint16 avail_event; // Only if VIRTIO_F_EVENT_IDX
} __attribute__((packed));
typedef struct virtq_used virtq_used_t;

struct virtq {
  // The actual descriptors (16 bytes each)
  virtq_desc_t desc[QUEUE_SIZE];

  // A ring of available descriptor heads with free-running index.
  virtq_avail_t *avail;

  // Padding to the next Queue Align boundary.
  // A ring of used descriptor heads with free-running index.
  virtq_used_t *used __attribute__((aligned(PAGE_SIZE)));

  u32 size;
  int qid;
  void *notify_addr;
} __attribute__((packed));
typedef struct virtq virtq_t;

struct virtio_pci_cap {
  uint8 cap_vndr;   /* Generic PCI field: PCI_CAP_ID_VNDR */
  uint8 cap_next;   /* Generic PCI field: next ptr. */
  uint8 cap_len;    /* Generic PCI field: capability length */
  uint8 cfg_type;   /* Identifies the structure. */
  uint8 bar;        /* Where to find it. */
  uint8 padding[3]; /* Pad to full dword. */
  uint32 offset;    /* Offset within bar. */
  uint32 length;    /* Length of the structure, in bytes. */
};
typedef struct virtio_pci_cap virtio_pci_cap_t;
// cfg_type values:
/* Common configuration */
#define VIRTIO_PCI_CAP_COMMON_CFG 1
/* Notifications */
#define VIRTIO_PCI_CAP_NOTIFY_CFG 2
/* ISR Status */
#define VIRTIO_PCI_CAP_ISR_CFG 3
/* Device specific configuration */
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
/* PCI configuration access */
#define VIRTIO_PCI_CAP_PCI_CFG 5

struct virtio_pci_common_cfg {
  /* About the whole device. */
  uint32 device_feature_select; /* read-write */
  uint32 device_feature;        /* read-only for driver */
  uint32 driver_feature_select; /* read-write */
  uint32 driver_feature;        /* read-write */
  uint16 msix_config;           /* read-write */
  uint16 num_queues;            /* read-only for driver */
  uint8 device_status;          /* read-write */
  uint8 config_generation;      /* read-only for driver */

  /* About a specific virtqueue. */
  uint16 queue_select;      /* read-write */
  uint16 queue_size;        /* read-write */
  uint16 queue_msix_vector; /* read-write */
  uint16 queue_enable;      /* read-write */
  uint16 queue_notify_off;  /* read-only for driver */
  uint64 queue_desc;        /* read-write */
  uint64 queue_driver;      /* read-write */
  uint64 queue_device;      /* read-write */
};
typedef struct virtio_pci_common_cfg virtio_pci_common_cfg_t;

struct virtio_blk_req {
  uint32 type;
  uint32 reserved;
  uint64 sector;
  uint8 data[512];
  uint8 status;
};
typedef struct virtio_blk_req virtio_blk_req_t;

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER 2
#define VIRTIO_STATUS_FAILED 128
#define VIRTIO_STATUS_FEATURES_OK 8
#define VIRTIO_STATUS_DRIVER_OK 4
#define VIRTIO_STATUS_DEVICE_NEEDS_RESET 64

// Features
/* Maximum size of any single segment is in size_max. */
#define VIRTIO_BLK_F_SIZE_MAX 1
/* Maximum number of segments in a request is in seg_max. */
#define VIRTIO_BLK_F_SEG_MAX 2
/* Disk-style geometry specified in geometry. */
#define VIRTIO_BLK_F_GEOMETRY 4
/* Device is read-only. */
#define VIRTIO_BLK_F_RO 5
/* Block size of disk is in blk_size. */
#define VIRTIO_BLK_F_BLK_SIZE 5
/* Cache flush command support. */
#define VIRTIO_BLK_F_FLUSH 9
/* Device exports information on optimal I/O alignment. */
#define VIRTIO_BLK_F_TOPOLOGY 10
/* Device can toggle its cache between writeback and writethrough modes. */
#define VIRTIO_BLK_F_CONFIG_WCE 11
/* Device supports multiqueue. */
#define VIRTIO_BLK_F_MQ 12
/* Device can support discard command, maximum discard sectors size in
 * max_discard_sectors and maximum discard segment number in max_discard_seg. */
#define VIRTIO_BLK_F_DISCARD 13
/* Device can support write zeroes command, maximum write zeroes sectors size in
 * max_write_zeroes_sectors and maximum write zeroes segment number in
 * max_write_zeroes_seg.                                  */
#define VIRTIO_BLK_F_WRITE_ZEROES 14
/* Support for indirect descriptors */
#define VIRTIO_F_INDIRECT_DESC 28
/* Support for avail_event and used_event fields */
#define VIRTIO_F_EVENT_IDX 29

// Legacy features
/* Device supports request barriers. */
#define VIRTIO_BLK_F_BARRIER 0
/* Device supports scsi packet commands. */
#define VIRTIO_BLK_F_SCSI 7
/* This feature indicates that the device accepts arbitrary descriptor layouts,
 * as described in Section 2.6.4.3 Legacy Interface: Message     Framing. */
#define VIRTIO_F_ANY_LAYOUT 27

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

void init_disk(void);

struct virtio_blk_device {
  uptr base_addr;
  virtio_pci_common_cfg_t *common_cfg;
  u32 notify_off_multiplier;
  void *notify_cfg;
  void *isr_cfg;
  void *device_cfg;
};
typedef struct virtio_blk_device virtio_blk_device_t;

struct virtio_blk {
  u8 status[QUEUE_SIZE];
  void *info[QUEUE_SIZE];
  // disk command headers.
  // one-for-one with descriptors, for convenience.
  virtio_blk_req_t ops[QUEUE_SIZE];
  virtq_t virtq;
  u32 capacity;
  u32 qsize; // queue0 size
  u16 used_idx;
  u16 avail_idx;
};
typedef struct virtio_blk virtio_blk_t;

#endif // !DISK_H
