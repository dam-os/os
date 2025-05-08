#ifndef VIRT_MEMORY_H
#define VIRT_MEMORY_H

#include "../lib/common.h"

#define sv39 (u64)8 << 60

void map_virt_mem(u64 *table2, u64 vaddr, u64 paddr);

u64 translate_va_to_pa(u64 vaddr, u64 pageable);

#endif // !VIRT_MEMORY_H
