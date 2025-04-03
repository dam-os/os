#include "drivers/device_tree.h"
#include "drivers/disk.h"
#include "drivers/pci.h"
#include "drivers/system.h"
#include "drivers/vga.h"
#include "lib/exception.h"
#include "lib/print.h"
#include "lib/process.h"
#include "lib/string.h"
#include "lib/timer.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "memory/paging.h"
#include "memory/virt_memory.h"

#define PRINT_SYS_INFO 0

struct proc *proc_a;
struct proc *proc_b;
struct proc *proc_c;
extern char stack_top[];

void delay(void) {
  for (int i = 0; i < 300000000; i++)
    __asm__ __volatile__("nop"); // do nothing
}

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO)
    read_fdt(dtb_address);

  WRITE_CSR(mtvec, (uint64_t)kernel_entry);

  cprintf("Stack top at: %p\n", stack_top);
  // ! Must be called before using processes !
  init_proc();
  // optional to call but still cool
  init_mem_table();
  init_heap(100);

  init_virtio_vga();
  proc_c = create_process((void *)0x1000000, 0);
  // === Timer test ===
  u64 start = mtime_get_time();
  // Wait 10 seconds
  cprintf("Sleeping for 10 seconds...");
  sleep(10000);
  cprintf("10 seconds passed\n");

  // // ! Must be called before using processes !
  // init_proc();
  // // optional to call but still cool
  // init_mem_table();
  // init_heap(100);
  // proc_c = create_process((void *)0x1000000, 0);
  //
  // // Manually set registers since kernel cant do syscall
  // __asm__ __volatile__("csrw mscratch, sp\n");
  // __asm__ __volatile__("auipc t0, 0\n");
  // __asm__ __volatile__(
  //     "addi t0, t0, 14\n"); // 14 should be the bytes from auipc, to after
  //     yield
  // __asm__ __volatile__("csrw mepc, t0\n");
  // yield(); // WARNING: Kernel returns here in usermode!

  // PANIC("uh oh spaghettios %d", 5);
  print("we will never print this\n");
  print("death\n");
  poweroff();
}
