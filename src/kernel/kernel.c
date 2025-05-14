#include "drivers/device_tree.h"
#include "drivers/pci.h"
#include "drivers/system.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "lib/common.h"
#include "lib/exception.h"
#include "lib/file.h"
#include "lib/io.h"
#include "lib/process.h"
#include "lib/screen.h"
#include "lib/timer.h"
#include "memory/kheap.h"
#include "memory/memory.h"
#include "memory/paging.h"

extern char stack_top[];
extern char __dtb_start[];
struct proc *proc_c;
file *stdout;
file *stdin;

#define MSTATUS_MPP_MASK (3UL << 11)

void kmain(void) {
  // NOTE: Loads the device tree address from arguments. Currently unused cause
  // we hardcoded the devicetree in the kernel
  // __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  // Call the syscall that we have patched to put us in M-mode
  __asm__ volatile("li a0, 65\n"
                   "li a6, 0\n"
                   "li a7, 0x1\n"
                   "ecall\n"
                   "la sp, stack_top\n");
  u64 mstatus;
  asm volatile("csrr %0, mstatus" : "=r"(mstatus));
  mstatus &= ~MSTATUS_MPP_MASK; // Clear bits 12:11 (set MPP to 00)

  asm volatile("csrw mstatus, %0" ::"r"(mstatus));
  u64 new_deleg = 0x0;
  asm volatile("csrw medeleg, %0" ::"r"(new_deleg));

  WRITE_CSR(mtvec, (u64)kernel_entry);

  // === io ===
  stdout = &stdout_uart;
  stdin = &stdin_uart;
  // ===== Init important stuff =====
  init_fdt((uptr)__dtb_start);
  init_uart();
  init_timer();
  cprintf("DTB START %p\n", __dtb_start);
  stopwatch("FDT, UART and Timer initialisation");

  stopwatch("Wrote DSR");

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
  // print_fdt();

  // Change stdout to print to screen instead of uart
  stdout = &stdout_screen;

  // sprintf test
  // char *buf = kmalloc(100);
  // csprintf(buf, "sprintf got me feeling like five equals %d\n", 5);
  // cprintf("printf got a message from sprintf: %s\n", buf);

  // Timer test
  // Wait 10 seconds
  cprintf("Sleeping for 5 second...");
  // sleep(5000);
  cprintf("5 second passed\r\n");
  stopwatch("5 second sleep");

  // optional to call but still cool
  proc_c = create_process((void *)0x1000000, 0);

  // Manually set registers since kernel cant do syscall
  __asm__ __volatile__("csrw mscratch, sp\n");
  __asm__ __volatile__("auipc t0, 0\n");
  __asm__ __volatile__(
      "addi t0, t0, 14\n"); // 14 should be the bytes from auipc, to after
  __asm__ __volatile__("csrw mepc, t0\n");
  cprintf("Yielding to user process\r\n");

  yield();   // WARNING: Kernel returns here in usermode!
  print(""); // Clears uart after user process

  print("we will never print this\r\n");
  print("death\n");
  PANIC("OS is done %d", 1);
  poweroff();
}
