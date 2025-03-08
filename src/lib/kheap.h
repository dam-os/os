#include <stddef.h>
struct block {
  size_t size;
  struct block *next;
  int free;
};

int init_heap(int page_numbers);

void print_heap_contents();
char *kmalloc(int size);

int kfree(char *ptr);
