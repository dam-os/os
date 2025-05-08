#ifndef UART_H
#define UART_H

#include "../lib/common.h"
#include "../lib/file.h"

extern file stdout_uart;
extern file stdin_uart;

void init_uart(void);
char getchar_uart(void);
char putchar_uart(char c);
s8 uart_write(file *context, const char c);
char uart_read(file *context);

#endif /* !UART_H */
