#include "io.h"
#include "../kernel.h"
#include "../lib/common.h"

#define DIGIT_COUNT 36

static const char DIGITS[DIGIT_COUNT] = "0123456789abcdefghijklmnopqrstuvwxyz";

void cputchar(char c) { stdout->write(stdout, c); }
char cgetchar() { return stdin->read(stdin); }

void print(const char *str) { __print(stdout, str); }

/** Print formatted string to stdout  */
void cprintf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  __printf(stdout, fmt, &ap);

  va_end(ap);
}

/** Print formatted string to a buffer */
void csprintf(char *buf, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  file bufr = {
      .buf = (unsigned char *)buf,
      .pos = 0,
      .write = &__buffer_write,
  };

  __printf(&bufr, fmt, &ap);

  va_end(ap);

  bufr.write(&bufr, '\0'); // Null-terminate the string
}

void print_char_hex(char c) {
  cputchar(DIGITS[(c >> 4) & 0xF]); // Print upper 4 bits
  cputchar(DIGITS[c & 0xF]);        // Print lower 4 bits
}

void __printf(file *fd, const char *fmt, va_list *ap) {
  while (*fmt != '\0') {
    switch (*fmt) {
    case '%':
      fmt++;
      __cvprintf(fd, &fmt, ap);
      break;
    default:
      fd->write(fd, *fmt);
    }
    fmt++;
  }
}

s8 __buffer_write(file *context, const char c) {
  context->buf[context->pos++] = c;
  return 0;
}

void __print(file *fd, const char *str) {
  while (*str != '\0') {
    fd->write(fd, *str);
    str++;
  }
}

void __print_u32(file *fd, u32 v, u8 base, u8 digits) {
  char buf[digits], *p = buf;
  do {
    *p++ = DIGITS[v % base];
    v /= base;
  } while (v);
  while (p != buf) {
    fd->write(fd, *--p);
  }
}

void __print_u64(file *fd, u64 v, u8 base, u8 digits) {
  char buf[digits], *p = buf;
  do {
    *p++ = DIGITS[v % base];
    v /= base;
  } while (v);
  while (p != buf) {
    fd->write(fd, *--p);
  }
}

void __cvprintf(file *fd, const char **str, va_list *ap) {
  switch (*(*str)) {
  case '%':
    fd->write(fd, '%');
    break;
  case 'c':
    fd->write(fd, va_arg(*ap, int));
    break;
  case 's':
    __print(fd, va_arg(*ap, char *));
    break;
  case 'd':
    __print_u32(fd, va_arg(*ap, int), 10, 10);
    break;
  case 'x':
    __print_u32(fd, va_arg(*ap, int), 16, 8);
    break;
  case 'p':
    __print_u64(fd, va_arg(*ap, u64), 16, 16);
    break;
  case 'b':
    __print_u32(fd, va_arg(*ap, int), 2, 32);
    break;
  case 'l': {
    switch (*++(*str)) {
    case 'd':
      __print_u64(fd, va_arg(*ap, u64), 10, 19);
      break;
    };
  } break;
  }
}
