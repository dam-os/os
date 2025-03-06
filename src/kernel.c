#include "lib/exception.h"
#include <stddef.h>

#include "lib/device_tree.h"
#include "lib/disk.h"
#include "lib/memory.h"
#include "lib/paging.h"
#include "lib/print.h"
#include "lib/process.h"
#include "lib/string.h"
#include "lib/system.h"
#include "lib/pci.h"
#include "lib/uart.h"

#define PRINT_SYS_INFO 0

void delay(void) {
  for (int i = 0; i < 30000000; i++)
    __asm__ __volatile__("nop");
}

struct proc *proc_a;
struct proc *proc_b;

void proc_a_entry(void) {
  cprintf("Starting process A\n");
  while (1) {
    putchar('A');
    switch_process(proc_a, proc_b);
    delay();
  }
}

void proc_b_entry(void) {
  cprintf("Starting process B\n");
  while (1) {
    putchar('B');
    switch_process(proc_b, proc_a);
    delay();
  }
}

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO)
    read_fdt(dtb_address);

  proc_a = create_process(proc_a_entry);
  proc_b = create_process(proc_b_entry);
  proc_a_entry();

  /*verify_disk();*/

  print("Hello world!\r\n");

  WRITE_CSR(mtvec, (uint64_t)kernel_entry);
  __asm__ __volatile__("unimp");

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

  enumerate_pci();
  
  PANIC("uh oh spaghettios %d", 5);
  print("we will never print this");
  print("\n");
  print("death");
  poweroff();
}
