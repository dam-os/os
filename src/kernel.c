#include <stddef.h>

#include "lib/print.h"
#include "lib/memory.h"
#include "lib/string.h"
#include "lib/device_tree.h"
#include "lib/uart.h"
#include "lib/device_tree.h"
#include "lib/print.h"

#define PRINT_SYS_INFO 0

void kmain(void) {
  uintptr_t dtb_address;
  __asm__ volatile ("mv %0, a1" : "=r" (dtb_address));

  if (PRINT_SYS_INFO) read_fdt(dtb_address);
  
  print("Hello world!\r\n");

  /* print format */
  printfmt("We can now print numbers!\n");
  printfmt("Like %d, %d and even %d!\n", 1, 42, 6700);
  printfmt("Can also print chars? %c\n", 'Y');
  printfmt("What about strings? %s\n", "Yep yep yep");
  printfmt("Hex now too! %x, %x, %x", 10, 11, 12);

  /* File device tree */
  int cmdMax = 200;
  char cmd[cmdMax]; 
  char* p = cmd; 

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
