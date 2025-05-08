#ifndef UART_H
#define UART_H

#include "../lib/common.h"
#include "../lib/file.h"

extern file stdout_uart;
extern file stdin_uart;

void init_uart(void);
char getchar_uart(void);
char putchar_uart(char c);
s8 uart_write(void *context, const char c);
char uart_read(void *context);

#endif /* !UART_H */
