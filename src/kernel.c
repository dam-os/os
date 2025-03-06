#include <stddef.h>

#include "lib/device_tree.h"
#include "lib/memory.h"
#include "lib/paging.h"
#include "lib/print.h"
#include "lib/string.h"
#include "lib/system.h"
#include "lib/uart.h"
#include "lib/disk.h"

#define PRINT_SYS_INFO 0

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile("mv %0, a1" : "=r"(dtb_address));

  if (PRINT_SYS_INFO)
    read_fdt(dtb_address);
  
  verify_disk();

  print("\n");
  poweroff();
}
