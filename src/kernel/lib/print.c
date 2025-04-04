#include "print.h"
#include "../drivers/uart.h"
#include "../lib/common.h"
// @TODO: Don't depend on stdarg.h!
#include <stdarg.h>

#define DIGIT_COUNT 36

static const char DIGITS[DIGIT_COUNT] = "0123456789abcdefghijklmnopqrstuvwxyz";

void print(const char *str) {
  while (*str != '\0') {
    putchar(*str);
    str++;
  }
}

void cvprintf_str(char *str) { print(str); }

void cvprintf_int(int v, int base, int digits) {
  char buf[digits], *p = buf;
  do {
    *p++ = DIGITS[v % base];
    v /= base;
  } while (v);
  while (p != buf) {
    putchar(*--p);
  }
}
void cvprintf_uint64_t(uint64_t v, int base, int digits) {
  char buf[digits], *p = buf;
  do {
    *p++ = DIGITS[v % base];
    v /= base;
  } while (v);
  while (p != buf) {
    putchar(*--p);
  }
}

void cvprintf(const char **str, va_list *ap) {
  switch (*(*str)) {
  case '%':
    putchar('%');
    break;
  case 'c':
    putchar(va_arg(*ap, int));
    break;
  case 's':
    cvprintf_str(va_arg(*ap, char *));
    break;
  case 'd':
    cvprintf_int(va_arg(*ap, int), 10, 10);
    break;
  case 'x':
    cvprintf_int(va_arg(*ap, int), 16, 8);
    break;
  case 'p':
    cvprintf_uint64_t(va_arg(*ap, uint64_t), 16, 16);
    break;
  case 'b':
    cvprintf_int(va_arg(*ap, int), 2, 32);
    break;
  case 'l': {
    switch (*++(*str)) {
    case 'd':
      cvprintf_uint64_t(va_arg(*ap, uint64_t), 10, 19);
      break;
    };
  } break;
  }
}

void cprintf(const char *str, ...) {
  va_list ap;
  va_start(ap, str);

  while (*str != '\0') {
    switch (*str) {
    case '%':
      str++;
      cvprintf(&str, &ap);
      break;
    default:
      putchar(*str);
    }
    str++;
  }

  va_end(ap);
}

void print_char_hex(char c) {
  putchar(DIGITS[(c >> 4) & 0xF]); // Print upper 4 bits
  putchar(DIGITS[c & 0xF]);        // Print lower 4 bits
}
