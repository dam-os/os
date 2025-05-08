#include "../lib/common.h"

void init_pci(void);

u32 pci_read_word(u8 bus, u8 device, u8 function, u8 offset);
void pci_write_word(u8 bus, u8 device, u8 function, u8 offset, u32 value);
u32 *pci_get_addr(u8 bus, u8 device, u8 function, u8 offset);
void enumerate_pci();
uptr get_pci_config_base(void);
