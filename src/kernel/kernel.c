#include "drivers/device_tree.h"
#include "drivers/pci.h"
#include "drivers/system.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "lib/exception.h"
#include "lib/print.h"
#include "lib/process.h"
#include "lib/screen.h"
#include "lib/string.h"
#include "lib/timer.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "memory/paging.h"
#include "memory/virt_memory.h"

extern char stack_top[];
struct proc *proc_c;

void kmain(void) {
  uptr dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  // ===== Init important stuff =====
  init_fdt(dtb_address);
  init_uart();
  init_timer();
  stopwatch("FDT, UART and Timer initialisation");

  WRITE_CSR(mtvec, (u64)kernel_entry);
  stopwatch("Wrote DSR");

  poweroff();

  // === Set up processes === //
  init_proc();
  stopwatch("Process initialisation");

  // === Init memory === //
  init_mem_table();
  stopwatch("Memory table initialisation");
  init_heap(100);
  stopwatch("Heap initialisation");

  // === Get addresses from device tree === //
  init_system();
  stopwatch("System initialisation");
  init_pci();
  stopwatch("PCI initialisation");

  // ====== Normal code ====== //

  init_virtio_vga();
  stopwatch("VGA initialisation");

  // === FDT ===
  print_fdt();

  // === Timer test ===
  init_print(1);
  // Wait 10 seconds
  cprintf("Sleeping for 5 second...");
  sleep(5000);
  cprintf("5 second passed\n");
  stopwatch("5 second sleep");
  // // ! Must be called before using processes !
  init_proc();
  // optional to call but still cool
  init_mem_table();
  init_heap(100);
  proc_c = create_process((void *)0x1000000, 0);

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
  poweroff();
}
