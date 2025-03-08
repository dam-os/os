#include "lib/device_tree.h"
#include "lib/disk.h"
#include "lib/exception.h"
#include "lib/kheap.h"
#include "lib/memory.h"
#include "lib/paging.h"
#include "lib/pci.h"
#include "lib/print.h"
#include "lib/process.h"
#include "lib/string.h"
#include "lib/system.h"
#include "lib/uart.h"
#define PRINT_SYS_INFO 0

struct proc *proc_a;
struct proc *proc_b;

void proc_a_entry(void) {
  cprintf("Starting process A\n");
  for (int i = 0; i < 5; i++) {
    cprintf("A running for the %d. time...\n", i + 1);
    yield();
  }
  cprintf("Process A is done!\n");
}

void proc_b_entry(void) {
  cprintf("Starting process B\n");
  for (int i = 0; i < 8; i++) {
    cprintf("B running for the %d. time...\n", i + 1);
    yield();
  }
  cprintf("Process B is done!\n");
}

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO)
    read_fdt(dtb_address);

  // ! Must be called before using processes !
  init_proc();
  // optional to call but still cool
  init_mem_table();
  init_heap(100);

  uint64_t ptr1 = kmalloc(20);
  cstrcpy((char *)ptr1, "11234567890234567890");
  cprintf("Allocated and filled 100 bytes: %s\n", ptr1);

  uint64_t ptr2 = kmalloc(10);
  cstrcpy((char *)ptr2, "1234567890");
  cprintf("Allocated and filled 10 bytes: %s\n", ptr2);

  char *ptr3 = (char *)kmalloc(50);
  cstrcpy(ptr3, "Block 3 with 50 bytes.");
  cprintf("Allocated and filled 50 bytes: %s\n", ptr3);
  print_heap_contents();
  proc_a = create_process(proc_a_entry);
  proc_b = create_process(proc_b_entry);

  yield();
  print("\nAll processes finished execution!\n");

  WRITE_CSR(mtvec, (uint64_t)kernel_entry);
  __asm__ __volatile__("unimp");

  uint64_t page = alloc_pages(5);
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
  print("death\n");
  poweroff();
}
