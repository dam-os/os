#ifndef PAGING_H
#define PAGING_H

#include "../lib/common.h"

#define PAGE_SIZE 0x1000

u64 alloc_pages(int n);
void init_mem_table();
int free_pages(u64 addr, int n);

#endif // !PAGING_H
