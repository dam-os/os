#include "drivers/device_tree.h"
#include "drivers/disk.h"
#include "drivers/pci.h"
#include "drivers/system.h"
#include "lib/exception.h"
#include "lib/print.h"
#include "lib/process.h"
#include "lib/string.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "memory/paging.h"
#include "memory/virt_memory.h"

#define PRINT_SYS_INFO 0

struct proc *proc_a;
struct proc *proc_b;
extern char stack_top[];

void proc_a_entry(void) {
  cprintf("Starting process A\n");
  for (int i = 0; i < 5; i++) {
    cprintf("A running for the %d. time...\n", i + 1);
    syscall(0, 0, 0, 8);
  }
  cprintf("Process A is done!\n");
}

void proc_b_entry(void) {
  cprintf("Starting process B\n");
  for (int i = 0; i < 8; i++) {
    cprintf("B running for the %d. time...\n", i + 1);
    syscall(0, 0, 0, 8);
  }
  cprintf("Process B is done!\n");
}

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO)
    read_fdt(dtb_address);

  verify_disk();

  WRITE_CSR(mtvec, (uint64_t)kernel_entry);

  cprintf("Stack top at: %p\n", stack_top);
  // ! Must be called before using processes !
  init_proc();
  // optional to call but still cool
  init_mem_table();
  init_heap(100);
  proc_a = create_process(proc_a_entry);
  proc_b = create_process(proc_b_entry);

  // Manually set registers since kenel cant do syscall
  __asm__ __volatile__("csrw mscratch, sp\n");
  __asm__ __volatile__("auipc t0, 0\n");
  __asm__ __volatile__(
      "addi t0, t0, 14\n"); // 14 should be the bytes from auipc, to after yield
  __asm__ __volatile__("csrw mepc, t0\n");
  yield(); // WARNING: Kernel returns here in usermode!

  print("\nAll processes finished execution!\n");

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
