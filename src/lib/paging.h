#define PAGE_SIZE 0x1000
int *alloc_pages(int n);
void init_mem_table();
int free_pages(int *addr, int n);
