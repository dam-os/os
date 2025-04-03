#include "../lib/common.h"
#include <stddef.h>
struct block {
  size_t size;
  struct block *next;
  int free;
};

int init_heap(int page_numbers);

void print_heap_contents();
void *kmalloc(int size);
void *krealloc(void *ptr, int size);

int kfree(void *ptr);
