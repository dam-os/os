#include "drivers/device_tree.h"
#include "drivers/pci.h"
#include "drivers/system.h"
#include "drivers/uart.h"
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

extern char stack_top[];
struct proc *proc_c;

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  // ===== Init important stuff =====
  init_fdt(dtb_address);
  init_uart();

  WRITE_CSR(mtvec, (uint64_t)kernel_entry);

  // === Set up processes === //
  init_proc();

  // === Init memory === //
  init_mem_table();
  init_heap(100);

  // === Get addresses from device tree === //
  init_system();
  init_timer();
  init_pci();

  // ====== Normal code ====== //

  init_virtio_vga();

  // === FDT ===
  print_fdt();

  // === Timer test ===
  u64 start = mtime_get_time();
  // Wait 10 seconds
  cprintf("Sleeping for 5 second...");
  sleep(5000);
  cprintf("5 second passed\n");

  // // ! Must be called before using processes !
  // init_proc();
  // // optional to call but still cool
  proc_c = create_process((void *)0x1000000, 0);
  //
  // Manually set registers since kernel cant do syscall
  __asm__ __volatile__("csrw mscratch, sp\n");
  __asm__ __volatile__("auipc t0, 0\n");
  __asm__ __volatile__(
      "addi t0, t0, 14\n"); // 14 should be the bytes from auipc, to after
  __asm__ __volatile__("csrw mepc, t0\n");
  yield();   // WARNING: Kernel returns here in usermode!
  print(""); // Clears uart after user process

  print("we will never print this\n");
  print("death\n");
  PANIC("uh oh spaghettios %d", 5);
}
