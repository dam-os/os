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

void __print_int(int v) {
  while (1) {
    putchar(v % 10 +'0');
    if (v < 10) break;
    v /= 10;
  }
}

void __formatprint(const char *str, va_list ap) {
  switch (*str) {
    case '%':
      putchar('%');
      break;
    case 'd':
      __print_int(va_arg(ap, int));
      break;
  }
}

void printf(const char *str, ...) {
  va_list ap;
  va_start(ap, str);

  while (*str != '\0') {
    switch (*str) {
      case '%':
        __formatprint(str, ap);
        break;
      default:
        putchar(*str);
    }
    str++;
  } 
}
