#include "paging.h"
#include "system.h"
#include <stddef.h>
struct block {
  size_t size;
  struct block *next;
  int free;
};
struct block *blocks = NULL;
int max_size = 0;
int init_heap(int page_numbers) {
  int *pages = alloc_pages(page_numbers);

  blocks = (struct block *)pages;
  max_size = page_numbers * PAGE_SIZE;

  struct block init_block = {
      max_size - sizeof(struct block),
      NULL,
      1,
  };

  blocks = &init_block;
  return 1;
}

char *kmalloc(int size) {
  struct block *current = blocks;
  while (current) {
    // if the block is free and big enough
    if (current->free && current->size >= size) {
      // if we need to split the block
      if (current->size > size + sizeof(struct block)) {
        // puts the new block in the available space in ram
        struct block *new_block =
            (struct block *)((char *)current + sizeof(struct block) + size);
        // new block is made smaller and we fix the linked list such that it
        // points correctly
        new_block->size = current->size - size - sizeof(struct block);
        new_block->next = current->next;
        new_block->free = 1;
        // we set the size of the block we will use
        current->size -= size + sizeof(struct block);
        current->next = new_block;
      }
      current->free = 0;
      return (char *)(struct block *)((char *)current + sizeof(struct block) +
                                      size);
    }
    current = current->next;
  }
  PANIC("CANT ALLOCATE");
  return 0;
}

int kfree(char *ptr) {
  struct block *block = (struct block *)((char *)ptr - sizeof(struct block));
  block->free = 1;
  return 1;
}

void print_heap_contents() {
  struct block *current = blocks;

  while (current) {
    if (!current->free) { // If the block is allocated
      char *data = (char *)current + sizeof(struct block);
      cprintf("Block at %p with size %zu contains: %s\n", data, current->size,
              data);
    }
    cprintf("%p\n", current);
    cprintf("%p\n", current->next);
    current = current->next;
  }
}
