
#include "lib/memory.h"
#include "lib/string.h"
#include <stddef.h>
#include <stdint.h>
#define SYSCON_ADDR 0x100000

#define UART_BASE 0x10000000
#define UART_DATA (*(volatile uint8_t *)(UART_BASE + 0x00))
#define UART_LSR (*(volatile uint8_t *)(UART_BASE + 0x05))
#define UART_LSR_RX_READY 0x01
#define UART_LSR_TX_EMPTY 0x20

char getchar(void) {
  while (!(UART_LSR & UART_LSR_RX_READY))
    ; // Wait until a character is available
  return UART_DATA;
}

char putchar(char c) {
  while (!(UART_LSR & UART_LSR_TX_EMPTY))
    ; // Wait until UART is ready to transmit
  UART_DATA = c;
  return c;
}

void print(const char *str) {
  while (*str != '\0') {
    putchar(*str);
    str++;
  }
}

void poweroff(void) {
  print("Poweroff requested\r\n");
  *(uint32_t *)SYSCON_ADDR = 0x5555;
}

void kmain(void) {
  print("Hello world!\r\n");
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
    putchar(c); // Echo back to the terminal
    if (c == '\r') {
      poweroff();
    }
  }
}
