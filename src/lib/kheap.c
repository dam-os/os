#include "common.h"
#include "paging.h"
#include "print.h"
#include "system.h"
#include <stddef.h>

struct block {
  size_t size;
  struct block *next;
  int free;
} __attribute__((aligned(8)));

struct block *blocks = NULL;

int max_size = 0;

void print_heap_contents() {
  struct block *current = blocks;
  cprintf("Heap blocks:\n");
  while (current != NULL) {
    cprintf("Block at %p:\n", (void *)current);
    cprintf("  Size: %d bytes\n", current->size - sizeof(struct block));
    cprintf("  Status: %s\n", current->free ? "Free" : "Allocated");
    cprintf("  Next Block: %p\n", (void *)current->next);
    cprintf("-------------------------\n");
    current = current->next;
  }
}
int init_heap(int page_numbers) {
  uint64_t pages = alloc_pages(page_numbers);
  cprintf("PAGES START AT: %p\n", pages);
  blocks = (struct block *)pages;
  max_size = page_numbers * PAGE_SIZE;

  struct block init_block = {
      max_size - sizeof(struct block),
      NULL,
      1,
  };

  *blocks = init_block;
  cprintf("Heap blocks start at: %p\n", blocks);
  return 1;
}

void *kmalloc(int size) {
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
        current->size = size + sizeof(struct block);
        current->next = new_block;
      }
      // if the block is perfect size we just use that one
      current->free = 0;
      cprintf("return ptr to here: %p\n", (current + sizeof(struct block)));
      cprintf("blck actually here: %p, %p\n", current, sizeof(struct block));

      return (current + sizeof(struct block));
    }
    current = current->next;
  }
  PANIC("CANT ALLOCATE");
  return 0;
}

int kfree(void *ptr) {
  struct block *block = (struct block *)(ptr - sizeof(struct block));
  block->free = 1;
  // merge free blocks
  cprintf("freeing block at %p\n", block);
  struct block *current = blocks;
  while (current) {
    if (current->free && current->next && current->next->free) {
      current->size += sizeof(struct block) + current->next->size;
      current->next = current->next->next;
    }
    current = current->next;
  }
  return 1;
}
