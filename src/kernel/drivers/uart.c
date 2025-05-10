#include "uart.h"
#include "../lib/common.h"
#include "../lib/file.h"
#include "../lib/io.h"
#include "device_tree.h"

uptr UART_BASE = NULL;
u32 UART_REG_WIDTH = 1;
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

u32 reg(u8 register_number) { return register_number * UART_REG_WIDTH; }

void init_uart(void) {
  // Get serial node
  char *serial = match_node("serial@[status?='okay']");
  UART_BASE = get_node_addr(serial);

  // Get registry width to determine LSR offset
  char width_search[48];
  csprintf(width_search, "%s*reg-io-width", serial);
  u32 *width_ptr = match_node(width_search);
  if (width_ptr != NULL) {
    UART_REG_WIDTH = swap_endian_32(*width_ptr);
  }

  UART_DATA = (volatile u8 *)(UART_BASE + reg(0)); // 0th register
  UART_LSR = (volatile u8 *)(UART_BASE + reg(5));  // 5th register

  print("[uart] UART initialised. Printing ready.\r\n");
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

s8 uart_write(__attribute__((unused)) file *context, const char c) {
  putchar_uart(c);
  return 0;
}

char uart_read(__attribute__((unused)) file *context) { return getchar_uart(); }
