#include <stddef.h>

void memset(char *p, char c, size_t n) {
  while (n > 0) {
    *p++ = c;
    n--;
  }
}

void memcpy(char *src, char *dst, size_t n) {
  while (n > 0) {
    *dst++ = *src++;
    n--;
  }
}
