#include "drivers/device_tree.h"
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

extern char stack_top[];

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  init_fdt(dtb_address);

  WRITE_CSR(mtvec, (uint64_t)kernel_entry);

  // ===== Init important stuff =====
  // ! Must be called before using processes !
  init_proc();
  // optional to call but still cool
  init_mem_table();
  init_heap(100);

  init_timer();
  // ===== Don't touch anything above this line unless u smort =====

  init_virtio_vga();
  // === FDT ===
  fdt_node_t *node = find_fdt("cpus");
  cprintf("Found node: %s\n", node->name);
  print_node(node, 2);

  // print("Before free:");
  //
  // print_heap_contents();

  free_node(node);
  // print("After free:");
  //
  // print_heap_contents();

  // === Timer test ===
  u64 start = mtime_get_time();
  // Wait 10 seconds
  cprintf("Sleeping for 1 second...");
  sleep(1000);
  cprintf("1 seconds passed\n");

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
