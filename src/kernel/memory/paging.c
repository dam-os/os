#include "../drivers/system.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "memory.h"
#define PAGE_SIZE 0x1000
extern char __free_ram[], __free_ram_end[];

char *memory_table;

int mem_table_size = 0;
uint64_t basec;

uint64_t lookup(int n, int *byte_idx, int *bit_idx) {
  int zero_count = 0;
  int bit_index = -1;
  int byte_index = -1;
  for (int byte = 0; byte < mem_table_size; byte++) {
    for (int bit = 0; bit < 8; bit++) {
      if (!(memory_table[byte] & (1 << bit))) {
        if (zero_count == 0) {
          byte_index = byte;
          bit_index = bit;
        }
        zero_count++;
        if (zero_count == n) {
          *byte_idx = byte_index;
          *bit_idx = bit_index;
          return byte * 8 * PAGE_SIZE + bit * PAGE_SIZE;
        }
      } else {
        zero_count = 0;
      }
    }
  }
  return -1;
}

void init_mem_table() {
  uint64_t free_ram_size = ((uint64_t)&__free_ram_end - (uint64_t)&__free_ram);
  mem_table_size = (free_ram_size / PAGE_SIZE) / 8;
  memset(__free_ram, '\0', mem_table_size);
  memory_table = __free_ram;
  int mem_table_pages = (mem_table_size + PAGE_SIZE - 1) / PAGE_SIZE;

  uint64_t base = (uint64_t)__free_ram + (mem_table_size) +
                  (PAGE_SIZE - (mem_table_size % PAGE_SIZE));

  for (int i = 0; i < mem_table_pages; i++) {
    int bit_pos = (free_ram_size / PAGE_SIZE) - 1 - i;
    memory_table[bit_pos / 8] |= (1 << (bit_pos % 8));
  }
  basec = base;

  cprintf("Free RAM starts at: %p\n", __free_ram);
  cprintf("base memory for paging starts at: %p\n", __free_ram);
}

void printBits(unsigned char byte) {
  for (int i = 0; i <= 7; i++) {
    cprintf("%d", (byte >> i) & 1);
  }
}

void print_mem_table() {
  for (size_t i = 0; i < 20; i++) {
    printBits((unsigned char)memory_table[i]);
    cprintf(" "); // Space between each byte
  }
  cprintf("\n"); // Space between each byte
}

uint64_t alloc_pages(int n) {
  if (mem_table_size == 0)
    // this sets base variable
    init_mem_table();
  if (n <= 0)
    poweroff();
  int byte_index = -1;
  int bit_index = -1;
  uint64_t page = basec + lookup(n, &byte_index, &bit_index);

  for (int i = 0; i < n; i++) {
    memory_table[byte_index] |= (1 << bit_index);
    bit_index++;
    if (bit_index != 8)
      continue;
    bit_index = 0;
    byte_index++;
  }

  memset((void *)page, 0, n * PAGE_SIZE);
  return page;
}

int free_pages(uint64_t addr, int n) {
  uint64_t page_index = (addr)-basec;
  int byte_index = page_index / 8 / PAGE_SIZE;
  int bit_index = page_index % 8;

  for (int i = 0; i < n; i++) {
    memory_table[byte_index] &= ~(1 << bit_index);
    bit_index++;
    if (bit_index == 8) {
      bit_index = 0;
      byte_index++;
    }
  }
  print_mem_table();
  return 0;
}
