#include "uart.h"
#include "../lib/common.h"
#include "../lib/file.h"
#include "../lib/io.h"
#include "device_tree.h"

uptr UART_BASE = NULL;
volatile u8 *UART_DATA = NULL;
volatile u8 *UART_LSR = NULL;
#define UART_LSR_RX_READY 0x01
#define UART_LSR_TX_EMPTY 0x20

file stdout_uart = {
    .write = &uart_write,
};
file stdin_uart = {
    .read = &uart_read,
};

void init_uart(void) {
  // Get serial node
  char *serial = match_node("serial@[status?='okay']");
  UART_BASE = get_node_addr(serial);
  UART_DATA = (volatile u8 *)(UART_BASE + 0x00);
  UART_LSR = (volatile u8 *)(UART_BASE + 0x05);

  print("[uart] UART initialised. Printing ready.\n");
}

char getchar_uart(void) {
  while (!(*UART_LSR & UART_LSR_RX_READY))
    ; // Wait until a character is available
  return *UART_DATA;
}

char putchar_uart(char c) {
  while (!(*UART_LSR & UART_LSR_TX_EMPTY))
    ; // Wait until UART is ready to transmit
  *UART_DATA = c;
  return c;
}

s8 uart_write(__attribute__((unused)) void *context, const char c) {
  putchar_uart(c);
  return 0;
}

char uart_read(void *context) { return getchar_uart(); }
