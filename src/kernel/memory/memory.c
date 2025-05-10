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

int memcmp(const void *s1, const void *s2, size_t n) {
  const unsigned char *p1 = (const unsigned char *)s1;
  const unsigned char *p2 = (const unsigned char *)s2;

  while (n--) {
    if (*p1 != *p2) {
      return n;
    }
    p1++;
    p2++;
  }
  return 0;
}
