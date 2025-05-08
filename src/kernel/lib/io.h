#ifndef IO_H
#define IO_H

#include "common.h"
#include "file.h"

void print(const char *str);
void cprintf(const char *str, ...);
void print_char_hex(char c);
void cputchar(char c);
char cgetchar();

void __print(file *fd, const char *str);
void __cvprintf(file *fd, const char **str, va_list *ap);

#endif /* !IO_H */
