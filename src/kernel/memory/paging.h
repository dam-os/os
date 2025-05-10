#ifndef PAGING_H
#define PAGING_H

#include "../lib/common.h"

#define PAGE_SIZE 0x1000

u64 alloc_pages(u32 n);
void init_mem_table();
u32 free_pages(u64 addr, u32 n);

#endif // !PAGING_H
