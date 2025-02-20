#include <stddef.h>

#include "lib/print.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/device_tree.h"
#include "lib/uart.h"
#include "lib/print.h"
#include "lib/system.h"

#define PRINT_SYS_INFO 0

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile ("mv %0, a1" : "=r" (dtb_address));

  if (PRINT_SYS_INFO) read_fdt(dtb_address);
  
  print("Hello world!\r\n");

  /* print format */
  printfmt("hello %d\n", 1000000000);
  printfmt("binary??? %b\n", 16);
  
  PANIC("uh oh spaghettios %d", 5);
  print("we will never print this");
}
