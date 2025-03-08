- Could attach the virtio-blk-device using mmio
- Used the specs/1k-os to find the memory address of the device
  - Could now read metadata from device by looking at specifications
- For some reason device version was always 0x1, even though newest version was
  0x2
- Found
  [stackexchange post](https://unix.stackexchange.com/questions/686010/what-is-the-difference-between-virtio-net-pci-virtio-net-pci-non-transitional-a)
  that explains that modern virtio devices cannot be used with mmio, but need to
  be added with pci
- Mounted as PCI device instead, used a mix of virtio specs and osdev PCI/virtio
  pages to read from device
