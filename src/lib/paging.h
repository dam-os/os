#define PAGE_SIZE 0x1000
int *alloc_pages(int n);
int free_pages(int *addr, int n);
