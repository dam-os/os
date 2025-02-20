#include "memory.h"
#include "system.h"
#include <stdint.h>
#define PAGE_SIZE 0x1000
extern char __free_ram[], __free_ram_end[];

int *alloc_pages(int n) {
  if (n <= 0)
    poweroff();
  static int *next_page = (int *)__free_ram;
  int *page = next_page;
  next_page += n * PAGE_SIZE;
  if (next_page > (int *)__free_ram_end) {
    // fucking die
    poweroff();
  }
  memset((void *)page, 0, n * PAGE_SIZE);
  return page;
}
