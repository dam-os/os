#include "lib/device_tree.h"
#include "lib/disk.h"
#include "lib/exception.h"
#include "lib/memory.h"
#include "lib/paging.h"
#include "lib/pci.h"
#include "lib/print.h"
#include "lib/process.h"
#include "lib/string.h"
#include "lib/system.h"
#include "lib/uart.h"

#define PRINT_SYS_INFO 0

void kmain(void) {
  uintptr dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO) {
    read_fdt(dtb_address);
  }

  // ! Must be called before using processes !
  init_proc();

  // proc_t *pci = create_process(enumerate_pci);
  proc_t *disk = create_process(init_disk);

  yield();
  print("\nAll processes finished execution!\n");

  print("\n");
  print("death\n");
  poweroff();
}
