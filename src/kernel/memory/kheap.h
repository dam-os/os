#include "../lib/common.h"

struct block {
  size_t size;
  struct block *next;
  u8 free;
} __attribute__((aligned(8)));
typedef struct block block_t;

int init_heap(int page_numbers);

void print_heap_contents();
void *kmalloc(unsigned int size);
void *krealloc(void *ptr, unsigned int size);

int kfree(void *ptr);
