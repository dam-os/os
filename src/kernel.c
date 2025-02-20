#include <stddef.h>
#include <stdint.h>

#include "lib/memory.h"
#include "lib/string.h"
#include "lib/device_tree.h"
#include "lib/uart.h"
#include "lib/device_tree.h"
#include "lib/print.h"

#define PRINT_SYS_INFO 0

void kmain(void) {
  print("Hello world!\r\n");

  /* print format */
  printf("We can now print numbers!\n");
  printf("Like %d, %d and even %d!", 1, 42, 6700);
  printf("Can also print chars? %c", 'Y');

  /* File device tree */
  int cmdMax = 200;
  char cmd[cmdMax]; 
  char* p = cmd; 
  uintptr_t dtb_address;
  __asm__ volatile ("mv %0, a1" : "=r" (dtb_address));

  if (PRINT_SYS_INFO) read_fdt(dtb_address);

  /* String compare */
  char *a = "Hi!";
  char *b = "Hi!";
  int x = strcmp(a, b);
  if (x == 1) {
    putchar('W');
  } else {
    putchar('L');
  }

  /* CLI */
  printf("Hello world!!! %d", 5);
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
