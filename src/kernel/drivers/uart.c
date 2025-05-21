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

  UART_DATA = (volatile u8 *)(UART_BASE + 0x0); // 0th register
  UART_LSR = (volatile u8 *)(UART_BASE + 0x14); // 5th register

  // Get serial node
  char *stdout_path = match_node("chosen*stdout-path");

  if (stdout_path[0] != '/') {
    // stdout_path is not a path but a node alias, look in the aliases node
    char alias_name[48];
    // Only include up to the first colon - anything after is metadata
    for (int i = 0; stdout_path[i] != ':' && stdout_path[i] != '\0'; i++) {
      alias_name[i] = stdout_path[i];
    }
    // Prepend "aliases*"
    char alias_path[64];
    csprintf(alias_path, "aliases*%s", alias_name);
    // Look it up
    stdout_path = match_node(alias_path);
  }
  // stdout_path is now in the form /soc/serial@1000000 or similar
  // We want to isolate serial@100000 by advancing pointer past the last /
  for (u8 i = 0; stdout_path[i] != '\0'; i++) {
    if (stdout_path[i] == '/') {
      // "cut" string and effectively reset the search through it
      stdout_path = stdout_path + i + 1;
      i = 0;
    }
  }

  UART_BASE = get_node_addr(stdout_path);

  // Get registry width to determine LSR offset
  char width_search[48];
  csprintf(width_search, "%s*reg-io-width", stdout_path);
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
