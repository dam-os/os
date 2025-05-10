#include "drivers/device_tree.h"
#include "drivers/pci.h"
#include "drivers/system.h"
#include "drivers/uart.h"
#include "drivers/vga.h"
#include "lib/common.h"
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

void timer_test(void) {

  u64 timer_base = 0x13050000;
  u32 *load_ptr = (u32 *)(timer_base + 0x8);
  u32 *ctrl_ptr = (u32 *)(timer_base + 0x4);
  u32 *reload_ptr = (u32 *)(timer_base + 0x14);
  u32 *enable_ptr = (u32 *)(timer_base + 0x10);
  u32 *value_ptr = (u32 *)(timer_base + 0x18);
  u32 *interrupt_ptr = (u32 *)(timer_base + 0x24);

  *interrupt_ptr = 0x1;
  cprintf("No Interrupts!\r\n");
  *load_ptr = 0xFF;
  cprintf("Load FF!\r\n");
  *ctrl_ptr = 0x0; // Continuous mode
  cprintf("Continuous mode!\r\n");
  *reload_ptr = 0x1; // Trigger reload
  cprintf("Reloaded!\r\n");
  *enable_ptr = 0x1; // Start timer
  cprintf("Enabled timer!\r\n");

  for (u32 k = 0; k < 100; k++) {
    u32 data = *value_ptr;
    cprintf("TIMER VALUE IS = %d\r\n", data);
  }
}

void shutdown(void) {
  // Power off PMU devices, and call software encourage to apply it
  u64 pmu = 0x17030000;
  u32 *pmu_ptr = (u32 *)(pmu + 0x10);
  u32 data = *pmu_ptr;
  *pmu_ptr = data | 0b111111;

  u32 *pmu_software_encourage_ptr = (u32 *)(pmu + 0x44);
  *pmu_software_encourage_ptr = 0xFF;
  *pmu_software_encourage_ptr = 0x0A;
  *pmu_software_encourage_ptr = 0xA0;
  cprintf("Should not happen!\r\n");
}

unsigned long test_escape(void) {
  unsigned long mepc_val;
  __asm__ volatile("csrr %0, mepc" : "=r"(mepc_val));
}

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

  WRITE_CSR(mtvec, (u64)kernel_entry);

  uptr dtb_address = 0x85000000UL; // Hardcoded device tree address - see
                                   // Makefile for where it is defined
  init_fdt(dtb_address);
  init_uart();

  // ===== Init important stuff =====

  unsigned long mepc_val = test_escape();
  cprintf("Is in mmode %p!\r\n", mepc_val);

  init_timer();
  stopwatch("FDT, UART and Timer initialisation");

  stopwatch("Wrote DSR");

  // === Set up processes === //
  stopwatch("Process initialisation");

  // === Init memory === //
  init_mem_table();
  stopwatch("Memory table initialisation");
  init_heap(10);
  stopwatch("Heap initialisation");

  // === Get addresses from device tree === //
  // init_system();
  stopwatch("System initialisation");
  // init_pci();
  stopwatch("PCI initialisation");

  // ====== Normal code ====== //

  // init_virtio_vga();
  int i = 0;
  while (i < 10) {
    cprintf("Sleeping for 1 second...");
    sleep(1000);
    cprintf("1 second passed\r\n");
    i++;
  }
  stopwatch("VGA initialisation");

  while (1) {
    char x = kgetchar();
    kputchar(x);
  }

  // WRITE_CSR(mtvec, (u64)kernel_entry);
  init_proc();
  // === FDT ===
  // print_fdt();

  // === Timer test ===
  init_print(0);
  // Wait 10 seconds

  stopwatch("5 second sleep");
  // // ! Must be called before using processes !
  // optional to call but still cool
  proc_c = create_process((void *)0x1000000, 0);

  // Manually set registers since kernel cant do syscall
  __asm__ __volatile__("csrw mscratch, sp\n");
  __asm__ __volatile__("auipc t0, 0\n");
  __asm__ __volatile__(
      "addi t0, t0, 14\n"); // 14 should be the bytes from auipc, to after
  __asm__ __volatile__("csrw mepc, t0\n");
  yield();   // WARNING: Kernel returns here in usermode!
  print(""); // Clears uart after user process

  print("we will never print this\r\n");
  print("death\r\n");
  PANIC("uh oh spaghettios %d", 5);
  poweroff();
}
