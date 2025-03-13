#include "common.h"
#include <stddef.h>
struct block {
  size_t size;
  struct block *next;
  int free;
};

int init_heap(int page_numbers);

void print_heap_contents();
uint64_t kmalloc(int size);
uint64_t krealloc(uint64_t ptr, int size);

int kfree(uint64_t ptr);
