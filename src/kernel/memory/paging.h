#include "../lib/common.h"
#define PAGE_SIZE 0x1000
uint64_t alloc_pages(int n);
void init_mem_table();
int free_pages(uint64_t addr, int n);
