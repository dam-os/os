#include "../lib/common.h"

#define sv39 (uint64_t)8 << 60

void map_virt_mem(uint64_t *table2, uint64_t vaddr, uint64_t paddr);

uint64_t translate_va_to_pa(uint64_t vaddr, uint64_t page_table);
