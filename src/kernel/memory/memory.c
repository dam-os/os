#include "memory.h"

void memset(const char *dst, char c, size_t n) {
  char *d = (char *)dst;
  while (n > 0) {
    *d++ = c;
    n--;
  }
}

void memcpy(const char *src, const char *dst, size_t n) {
  char *s = (char *)src;
  char *d = (char *)dst;
  while (n > 0) {
    *d++ = *s++;
    n--;
  }
}
