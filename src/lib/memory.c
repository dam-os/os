#include "memory.h"
#include <stddef.h>

void memset(const char *dst, char c, size_t n) {
  char *d = dst;
  while (n > 0) {
    *d++ = c;
    n--;
  }
}

void memcpy(const char *src, const char *dst, size_t n) {
  char *s = src;
  char *d = dst;
  while (n > 0) {
    *d++ = *s++;
    n--;
  }
}
