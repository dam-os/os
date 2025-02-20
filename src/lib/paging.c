#include "memory.h"
#include "system.h"
#include <stdint.h>
#define PAGE_SIZE 0x1000
extern char __free_ram[], __free_ram_end[];

char *memory_table;

int mem_table_size = 0;

int lookup(int n) {
  int zero_count = 0;
  int bit_index = 0;
  for (int byte = 0; byte < mem_table_size; byte++) {
    for (int bit = 0; byte < 8; byte++) {
      if (!(memory_table[byte] & (1 << bit))) {
        if (zero_count == 0) {
          bit_index = bit;
        }
        zero_count++;
        if (zero_count == n) {
          bit_index = bit;
          return byte * 8 * PAGE_SIZE + bit * PAGE_SIZE;
        }
      } else {
        zero_count = 0;
      }
    }
  }
  return -1;
}

int *set_memory_table() {
  int free_ram_size =
      ((int)(uintptr_t)&__free_ram_end - (uintptr_t)&__free_ram);
  mem_table_size = (free_ram_size / PAGE_SIZE) / 8;
  memset(__free_ram, '\0', mem_table_size);
  memory_table = __free_ram;
  int mem_table_pages = (mem_table_size + PAGE_SIZE - 1) / PAGE_SIZE;

  int *base = (int *)__free_ram + (mem_table_size) +
              (PAGE_SIZE - (mem_table_size % PAGE_SIZE));

  for (int i = 0; i < mem_table_pages; i++) {
    int bit_pos = (free_ram_size / PAGE_SIZE) - 1 - i;
    memory_table[bit_pos / 8] |= (1 << (bit_pos % 8));
  }
  return base;
}
// 4096 + 10000
// 14096
// 4096-10000%

int *alloc_page(int n) {
  static int *base;
  if (mem_table_size == 0)
    base = set_memory_table();
  if (n <= 0)
    poweroff();
  int *page = base + lookup(n);
  memset((void *)page, 0, n * PAGE_SIZE);
  return page;
}
