#ifndef IO_H
#define IO_H

#include "common.h"
#include "file.h"

void print(const char *str);
void cprintf(const char *str, ...);
void csprintf(char *buf, const char *fmt, ...);
void print_char_hex(char c);
void cputchar(char c);
char cgetchar();

void __printf(file *fd, const char *fmt, va_list *ap);
void __print(file *fd, const char *str);
void __cvprintf(file *fd, const char **str, va_list *ap);
s8 __buffer_write(file *context, const char c);

#endif /* !IO_H */
