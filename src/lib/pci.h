uint32_t pci_read_config(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset);
void pci_write_word(uint8_t bus, uint8_t device, uint8_t function, uint8_t offset, uint32_t value);
void enumerate_pci();