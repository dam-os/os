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

#define CLINT_BASE       0x2000000;
                            



void shutdown(void) {
  // Power off PMU devices, and call software encourage to apply it
  u64 pmu = 0x17030000;
  u32 * pmu_ptr = (u32 *)(pmu + 0x10);
  u32 data = *pmu_ptr;
  *pmu_ptr = data | 0b111111;
  
  
  u32 * pmu_software_encourage_ptr = (u32 *)(pmu + 0x44);
  *pmu_software_encourage_ptr = 0xFF;
  *pmu_software_encourage_ptr = 0x0A;
  *pmu_software_encourage_ptr = 0xA0;
  cprintf("Should not happen!\r\n");
}

void kmain(void) {
  uptr dtb_address = 0x84000000;
  //__asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  // ===== Init important stuff =====
  init_uart();
  cprintf("Hello Framework Board!\r\n");
  
u64 timer_base = 0x13050000;
u32 *load_ptr   = (u32 *)(timer_base + 0x8);
u32 *ctrl_ptr   = (u32 *)(timer_base + 0x4);
u32 *reload_ptr = (u32 *)(timer_base + 0x14);
u32 *enable_ptr = (u32 *)(timer_base + 0x10);
u32 *value_ptr  = (u32 *)(timer_base + 0x18);
u32 *interrupt_ptr  = (u32 *)(timer_base + 0x24);

*interrupt_ptr = 0x1;
cprintf("No Interrupts!\r\n");
*load_ptr = 0xFF;
cprintf("Load FF!\r\n");
*ctrl_ptr = 0x0;         // Continuous mode
cprintf("Continuous mode!\r\n");
*reload_ptr = 0x1;       // Trigger reload
cprintf("Reloaded!\r\n");
*enable_ptr = 0x1;       // Start timer
cprintf("Enabled timer!\r\n");

for (u32 k = 0; k < 100; k++) {
  u32 data = *value_ptr;
    cprintf("TIMER VALUE IS = %d\r\n", data);
} 



  cprintf("A1 is %p\r\n", dtb_address);
  init_fdt(dtb_address);
  init_timer();
  stopwatch("FDT, UART and Timer initialisation");

  //WRITE_CSR(mtvec, (u64)kernel_entry);
  stopwatch("Wrote DSR");
  
  // === Set up processes === //
  stopwatch("Process initialisation");
  
  // === Init memory === //
  init_mem_table();
  stopwatch("Memory table initialisation");
  init_heap(10);
  stopwatch("Heap initialisation");
  
  // === Get addresses from device tree === //
  //init_system();
  stopwatch("System initialisation");
  //init_pci();
  stopwatch("PCI initialisation");
  
  // ====== Normal code ====== //
  
  //init_virtio_vga();
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
  
  //WRITE_CSR(mtvec, (u64)kernel_entry);
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
