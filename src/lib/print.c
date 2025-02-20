#include <stddef.h>
#include <stdarg.h>

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
  assert(base >= 2U && base <= DIGIT_COUNT, "Number base not within bounds (2 <= base <= 36)\n");
  /* Reversed int is initialized to 1 so we don't lose trailing zeros. A 500
   * would be reversed to 005 aka. 5, but now it will become 1005. We just don't
   * print the last 1. */
  int r = 1;

  /* Reverse v */
  while (v) {
    r *= base;
    r += v % base;
    v /= base;
  }

  /* Print r in reverse */
  while (r-1) {
    putchar(DIGITS[r % base]);
    r /= base;
  }
}

void __print_string(char *str) {
  print(str);
}

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
      __print_string(va_arg(*ap, char*));
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
