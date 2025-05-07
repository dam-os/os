#include "kheap.h"
#include "../drivers/system.h"
#include "../lib/assert.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "memory.h"
#include "paging.h"

#define DEBUG 0

block_t *blocks = NULL;

#define BLOCK_SIZE sizeof(block_t)

#define BLOCK_TO_PTR(b) ((void *)((char *)(b) + BLOCK_SIZE))

#define PTR_TO_BLOCK(p) ((block_t *)((char *)(p) - BLOCK_SIZE))

#define TOTAL_BLOCK_SIZE(s) ((s) + BLOCK_SIZE)

int max_size = 0;

void print_heap_contents() {
  block_t *current = blocks;
  cprintf("Heap blocks:\n");
  cprintf("  -------------------------\n");
  while (current != NULL) {
    cprintf("  Address: 0x%p\n", (void *)current);
    cprintf("  Size:    %ld bytes\n", current->size);
    cprintf("  Alloced: %ld bytes\n", current->size - BLOCK_SIZE);
    cprintf("  Status:  %s\n", current->free ? "Free" : "Allocated");
    cprintf("  Next:    0x%p\n", (void *)current->next);
    cprintf("  -------------------------\n");
    current = current->next;
  }
}
int init_heap(int page_numbers) {
  if (DEBUG) {
    cprintf("BLOCK SIZE: %d\n", BLOCK_SIZE);
  }

  void *pages = (void *)alloc_pages(page_numbers);
  blocks = (block_t *)pages;
  max_size = page_numbers * PAGE_SIZE;

  block_t init_block = {
      max_size - sizeof(block_t),
      NULL,
      1,
  };

  *blocks = init_block;

  cprintf(
      "[heap] Heap initialised with %d pages and a block size of %d bytes.\n",
      page_numbers, BLOCK_SIZE);
  return 1;
}

void *kmalloc(unsigned int size) {
  if (DEBUG) {
    print_heap_contents();
    cprintf("Trying to allocate %d bytes\n", size);
  }

  block_t *current = blocks;

  while (current) {
    // if the block is free and big enough
    if (current->free && current->size - sizeof(block_t) >= size) {
      // if we need to split the block
      // A split requires that the current block has enough space to support
      // 1. The new allocated size for left split
      // 2. The block header for left split
      // 3. At least 1 allocated byte for right split
      // 4. The block header for right split
      if (current->size > size + 1 + sizeof(block_t) * 2) {
        block_t *old = current;
        void *old_ptr = current;
        block_t *left_split = (block_t *)old_ptr;
        block_t *right_split = (block_t *)(old_ptr + TOTAL_BLOCK_SIZE(size));

        // Set new block sizes
        uptr old_end = (uptr)old_ptr + old->size;
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
  if (DEBUG) {
    print("Before free:");
    print_heap_contents();
  }

  block_t *block = (block_t *)(ptr - BLOCK_SIZE);
  kassert(block->free == FALSE);
  block->free = TRUE;

  // Merge adjacent free blocks
  block_t *current = block->next;

  while (1) {
    if (current && current->free) {
      block->size += current->size;
      block->next = current->next;

      if (DEBUG) {
        cprintf("Merged block at %p with next block at %p\n", (void *)block,
                (void *)current);
        print_heap_contents();
      }

      current = current->next;
    } else {
      break;
    }
  }

  return 1;
}

int block_can_shrink(block_t *block, unsigned int new_size) {
  return block->size > new_size + sizeof(block_t) * 2 + 1;
}

void *shrink_block(block_t *block, unsigned int new_size) {
  void *old_ptr = (void *)block;
  block_t *left = block;
  block_t *right = (block_t *)(old_ptr + TOTAL_BLOCK_SIZE(new_size));

  uptr old_end = (uptr)old_ptr + block->size;
  left->size = TOTAL_BLOCK_SIZE(new_size);
  uptr left_end = (uptr)left + left->size;
  right->size = old_end - left_end;

  right->next = left->next;
  left->next = right;

  left->free = 0;
  right->free = 1;

  return BLOCK_TO_PTR(left);
}

int block_can_grow(block_t *block, unsigned int new_size) {
  block_t *next = block->next;
  return next && next->free &&
         block->size + next->size >= TOTAL_BLOCK_SIZE(new_size);
}

void *grow_block(block_t *block, unsigned int new_size) {
  block_t *next = block->next;
  block->size += next->size;
  block->next = next->next;

  if (block_can_shrink(block, new_size)) {
    return shrink_block(block, new_size);
  }

  return BLOCK_TO_PTR(block);
}

void *krealloc(void *ptr, unsigned int new_size) {
  if (!ptr)
    return kmalloc(new_size);

  block_t *block = PTR_TO_BLOCK(ptr);
  unsigned int block_size_minus_struct = block->size - sizeof(block_t);

  if (new_size <= block_size_minus_struct) {
    if (block_can_shrink(block, new_size)) {
      return shrink_block(block, new_size);
    }
    return ptr;
  }

  if (block_can_grow(block, new_size)) {
    return grow_block(block, new_size);
  }

  void *new_ptr = kmalloc(new_size);
  if (!new_ptr)
    return NULL;

  memcpy(ptr, new_ptr, block_size_minus_struct);
  kfree(ptr);
  return new_ptr;
}
