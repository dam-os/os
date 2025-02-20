#include <stdarg.h>
#include <stddef.h>

#include "assert.h"
#include "print.h"
#include "uart.h"

#define DIGIT_COUNT 36

static const char DIGITS[DIGIT_COUNT] = "0123456789abcdefghijklmnopqrstuvwxyz";

void print(const char *str) {
  while (*str != '\0') {
    putchar(*str);
    str++;
  }
}

/**
 * Prints integers using putchar
 *
 * Takes an integer 123 and reverses it to 321. Then prints each digit from
 * right to left.
 */
void __print_int_base(int v, char base) {
  int r = 0, z = 0, m;

  if (v < 0) {
    v *= -1;
    putchar('-');
  }

  /* Reverse v */
  do {
    r *= base;
    m = v % base;
    z += !m;
    r += m;
    v /= base;
  } while (v);

  /* Print r in reverse */
  while (r) {
    putchar(DIGITS[r % base]);
    r /= base;
  }
  while (z) {
    putchar('0');
    z--;
  }
}

void __print_string(char *str) { print(str); }

void __formatprint(const char *str, va_list *ap) {
  switch (*str) {
  case '%':
    putchar('%');
    break;
  case 'd':
    __print_int_base(va_arg(*ap, int), 10);
    break;
  case 'c':
    putchar(va_arg(*ap, int));
    break;
  case 'x':
    __print_int_base(va_arg(*ap, int), 16);
    break;
  case 's':
    __print_string(va_arg(*ap, char *));
    break;
  case 'b':
    __print_int_base(va_arg(*ap, int), 2);
    break;
  }
}

void printfmt(const char *str, ...) {
  va_list ap;
  va_start(ap, str);

  while (*str != '\0') {
    switch (*str) {
    case '%':
      str++;
      __formatprint(str, &ap);
      break;
    default:
      putchar(*str);
    }
    str++;
  }

  va_end(ap);
}
