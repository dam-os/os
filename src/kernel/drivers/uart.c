#include "../lib/common.h"

#define UART_BASE 0x10000000
#define UART_DATA (*(volatile u8 *)(UART_BASE + 0x00))
#define UART_LSR (*(volatile u8 *)(UART_BASE + 0x05))
#define UART_LSR_RX_READY 0x01
#define UART_LSR_TX_EMPTY 0x20

char kgetchar(void) {
  while (!(UART_LSR & UART_LSR_RX_READY))
    ; // Wait until a character is available
  return UART_DATA;
}

char kputchar(char c) {
  while (!(UART_LSR & UART_LSR_TX_EMPTY))
    ; // Wait until UART is ready to transmit
  UART_DATA = c;
  return c;
}
