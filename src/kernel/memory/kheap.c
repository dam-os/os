#include "../drivers/system.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "memory.h"
#include "paging.h"

#define DEBUG 0

struct block {
  size_t size;
  struct block *next;
  int free;
} __attribute__((aligned(8)));

struct block *blocks = NULL;

#define BLOCK_SIZE sizeof(struct block)

#define BLOCK_TO_PTR(b) ((void *)((char *)(b) + BLOCK_SIZE))

#define PTR_TO_BLOCK(p) ((struct block *)((char *)(p) - BLOCK_SIZE))

#define TOTAL_BLOCK_SIZE(s) ((s) + BLOCK_SIZE)

int max_size = 0;

void print_heap_contents() {
  struct block *current = blocks;
  cprintf("Heap blocks:\n");
  cprintf("  -------------------------\n");
  while (current != NULL) {
    cprintf("  Block at %p:\n", (void *)current);
    cprintf("  Size: %ld bytes\n", current->size - sizeof(struct block));
    cprintf("  Status: %s\n", current->free ? "Free" : "Allocated");
    cprintf("  Next Block: %p\n", (void *)current->next);
    cprintf("  -------------------------\n");
    current = current->next;
  }
}
int init_heap(int page_numbers) {
  void *pages = (void *)alloc_pages(page_numbers);
  blocks = (struct block *)pages;
  max_size = page_numbers * PAGE_SIZE;

  struct block init_block = {
      max_size - sizeof(struct block),
      NULL,
      1,
  };

  *blocks = init_block;
  return 1;
}

void *kmalloc(int size) {
  if (DEBUG) {
    print_heap_contents();
    cprintf("Trying to allocate %d bytes\n", size);
  }

  struct block *current = blocks;

  while (current) {
    // if the block is free and big enough
    if (current->free && current->size - sizeof(struct block) >= size) {
      // if we need to split the block
      // A split requires that the current block has enough space to support
      // 1. The new allocated size for left split
      // 2. The block header for left split
      // 3. At least 1 allocated byte for right split
      // 4. The block header for right split
      if (current->size > size + 1 + sizeof(struct block) * 2) {
        struct block *old = current;
        struct block *left_split = old;
        struct block *right_split = old + TOTAL_BLOCK_SIZE(size);

        // Set new block sizes
        uptr old_end = (uptr)old + old->size;
        left_split->size = TOTAL_BLOCK_SIZE(size);

        uptr left_end = (uptr)left_split + left_split->size;
        right_split->size = old_end - left_end;

        // Update linked list references
        right_split->next = old->next;
        left_split->next = right_split;

        // Allocate left, free right
        left_split->free = 0;
        right_split->free = 1;
      }
      // if the block is perfect size we just use that one
      current->free = 0;

      if (DEBUG) {
        cprintf("Allocated %d bytes at %p\n", size,
                (void *)BLOCK_TO_PTR(current));
        print_heap_contents();
      }

      return BLOCK_TO_PTR(current);
    }
    current = current->next;
  }
  PANIC("CANT ALLOCATE");
  return 0;
}

int kfree(void *ptr) {
  struct block *block = (struct block *)(ptr - sizeof(struct block));
  block->free = 1;
  // @TODO: Wanna implement this again eventually
  // merge free blocks
  // struct block *current = block->next;
  // while (current) {
  //   if (current && current && current->next) {
  //     current->size += sizeof(struct block) + current->size;
  //     current = current->next;
  //   }
  //   current = current->next;
  // }
  return 1;
}

void *krealloc(void *ptr, int size) {
  if (DEBUG)
    cprintf("Trying to realloc ptr %p to size %d\n", ptr, size);

  struct block *block = (struct block *)(ptr - sizeof(struct block));

  if (size < block->size - sizeof(struct block)) {
    int remaining_size = block->size - size - sizeof(struct block);
    if (DEBUG)
      cprintf("REMAINING SIZE %d\n", remaining_size);

    if (remaining_size > sizeof(struct block)) {
      struct block *new_block =
          (struct block *)((char *)block + sizeof(struct block) + size);
      new_block->size = remaining_size;
      new_block->free = 1;
      new_block->next = block->next;

      block->size = size + sizeof(struct block);
      block->next = new_block;
    }

    return ptr;
  }

  int possible_size = block->size;
  struct block *current = block->next;
  struct block *prev = block;
  // check if we can expand current block
  while (possible_size < size && current) {
    if (current->free) {
      possible_size += current->size;
      prev = current;
      current = current->next;
      continue;
    }
    break;
  }
  if (possible_size >= size) {
    struct block *new_block =
        (struct block *)((char *)block + sizeof(struct block) + size);
    block->next = new_block;

    new_block->next = current;
    new_block->free = 1;
    new_block->size =
        prev->size - (block->size - (size + sizeof(struct block)));
    block->size = size + sizeof(struct block);
    block->next = new_block;
    return ((void *)block + sizeof(struct block));
    // expand current block
  } else {
    // alloc new block and free the old
    void *new_ptr = kmalloc(size);
    memcpy(ptr, new_ptr, block->size);
    kfree(ptr);
    return new_ptr;
  }
}
