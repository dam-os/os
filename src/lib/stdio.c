#include "stdio.h"
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

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

/**
 * Prints integers using putchar
 *
 * Takes an integer 123 and reverses it to 321. Then prints each digit from
 * right to left.
 */
void __print_int(int v) {
  /* Reversed int is initialized to 1 to we don't lose trailing zeros. A 500
   * would be reversed to 005 aka. 5, but now it will become 1005. We just don't
   * print the last 1. */
  int r = 1;
  /* Reverse v */
  while (v) {
    r *= 10;
    r += v % 10;
    v /= 10;
  }

  /* Print r in reverse */
  while (r-1) {
    putchar(r % 10 +'0');
    r /= 10;
  }
}

void __formatprint(const char *str, va_list *ap) {
  switch (*str) {
    case '%':
      putchar('%');
      break;
    case 'd':
      __print_int(va_arg(*ap, int));
      break;
  }
}

void printf(const char *str, ...) {
  va_list ap;
  va_start(ap, str);

  while (*str != '\0') {
    switch (*str) {
      case '%':
        str++;
        __formatprint(str, &ap);
        break;
      default:
        putchar(*str);
    }
    str++;
  } 

  va_end(ap);
}
