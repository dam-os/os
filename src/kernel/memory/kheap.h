#include "../lib/common.h"

struct block {
  size_t size;
  struct block *next;
  u8 free;
} __attribute__((aligned(8)));
typedef struct block block_t;

u32 init_heap(u32 page_numbers);

void print_heap_contents();
void *kmalloc(size_t size);
void *krealloc(void *ptr, size_t size);

u32 kfree(void *ptr);
