#ifndef PAGING_H
#define PAGING_H

#include "../lib/common.h"

#define PAGE_SIZE 0x1000

uint64 alloc_pages(int n);
void init_mem_table();
int free_pages(uint64 addr, int n);

#endif // !PAGING_H
