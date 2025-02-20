
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/device_tree.h"
#include "lib/uart.h"
#include <stddef.h>
#include <stdint.h>

#include "./lib/device_tree.h"

#define SYSCON_ADDR 0x100000

#define PRINT_SYS_INFO 0

void poweroff(void) {
  print("Poweroff requested\r\n");
  *(uint32_t *)SYSCON_ADDR = 0x5555;
}

void kmain(void) {
  print("Hello world!\r\n");
  int cmdMax = 200;
  char cmd[cmdMax]; 
  char* p = cmd; 
  uintptr_t dtb_address;
  __asm__ volatile ("mv %0, a1" : "=r" (dtb_address));

  if (PRINT_SYS_INFO) read_fdt(dtb_address);
  char *a = "Hi!";
  char *b = "Hi!";
  int x = strcmp(a, b);
  if (x == 1) {
    putchar('W');
  } else {
    putchar('L');
  }
  while (1) {
    // Read a single character from the UART
    char c = getchar();
    if (c == 0x7F) { // Delete / Backspace
      *p = '\0';
      putchar('\n');
      if (!(p > cmd)) continue;
      p--;
      print(cmd);
    } else if (c == '\r') {
      print("\ncommand is: ");
      print(cmd);
      print("\n");

    } else {
      *p++ = c;
      putchar(c); // Echo back to the terminal
    }
  }
}
