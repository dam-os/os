#include "../lib/common.h"
#include "../lib/print.h"
#include "device_tree.h"

uptr UART_BASE = NULL;
volatile u8 *UART_DATA = NULL;
volatile u8 *UART_LSR = NULL;
#define UART_LSR_RX_READY 0x01
#define UART_LSR_TX_EMPTY 0x20

void init_uart(void) {
  // Get serial node
  char *serial = match_node("serial@");
  UART_BASE = get_node_addr(serial);
  UART_DATA = (volatile u8 *)(UART_BASE + 0x00);
  UART_LSR = (volatile u8 *)(UART_BASE + 0x05);

  print("[uart] UART initialised. Printing ready.\n");
}

char kgetchar(void) {
  while (!(*UART_LSR & UART_LSR_RX_READY))
    ; // Wait until a character is available
  return *UART_DATA;
}

char kputchar(char c) {
  while (!(*UART_LSR & UART_LSR_TX_EMPTY))
    ; // Wait until UART is ready to transmit
  *UART_DATA = c;
  return c;
}
