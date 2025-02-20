#include <stddef.h>

#include "lib/device_tree.h"
#include "lib/memory.h"
#include "lib/paging.h"
#include "lib/print.h"
#include "lib/string.h"
#include "lib/system.h"
#include "lib/uart.h"

#define PRINT_SYS_INFO 0

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO)
    read_fdt(dtb_address);

  print("Hello world!\r\n");

  int *page = alloc_pages(5);
  alloc_pages(3);
  free_pages(page, 5);
  alloc_pages(3);

  /* print format */
  cprintf("hello %d\n", 1234567);
  cprintf("hello %d\n", 12345678901);
  cprintf("binary??? %b\n", 586);
  cprintf("hex... %x, %x\n", 16, 500);
  cprintf("bin... %b, %b\n", 16, 500);

  int buf[64];
  int *ptr = buf;

  cprintf("ponter: %p\n%ld\n", ptr, ptr);
  cprintf("%d\n", 98);
  cprintf("%d\n", 99);
  cprintf("%d\n", 100);
  cprintf("%d\n", 101);

  PANIC("uh oh spaghettios %d", 5);
  print("we will never print this");
}
