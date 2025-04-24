#include "../lib/common.h"

void init_pci(void);

uint32_t pci_read_word(uint8_t bus, uint8_t device, uint8_t function,
                       uint8_t offset);
void pci_write_word(uint8_t bus, uint8_t device, uint8_t function,
                    uint8_t offset, uint32_t value);
uint32_t *pci_get_addr(uint8_t bus, uint8_t device, uint8_t function,
                       uint8_t offset);
void enumerate_pci();
