#include "common.h"

int cstrlen(char *src) {
  u32 len = 0;
  while (*src != '\0') {
    src += 1;
    len++;
  }
  return len;
}

u32 cstrcmp(char *src, char *dest) {
  u32 l = cstrlen(src);
  if (l != cstrlen(dest))
    return -1;
  for (u32 i = 0; i < l; i++) {
    if (src[i] ^ dest[i]) {
      return -1;
    }
  }

  return 0;
}

u32 startswith(char *search, char *target) {
  u32 l = cstrlen(search);
  // can't have a search longer than the target now
  if (l > cstrlen(target))
    return -2;

  for (u32 i = 0; i < l; i++) {
    if (search[i] ^ target[i]) {
      return -1;
    }
  }

  return 0;
}

u32 strincl(char *in, char *container) {
  u32 len = cstrlen(in);
  u32 clen = cstrlen(container);
  u32 test = 0;
  u32 i = 0;
  while (i < clen && test != len) {
    if (container[i] == in[test]) {
      test++;
    } else {
      test = 0;

      if (i > 0 && container[i - 1] == in[test]) {
        test++;
        continue;
      }
    }
    i++;
  }
  return test != len;
}

void cstrcpy(char *src, char *dest) {
  for (u32 i = 0; i < cstrlen(src); i++) {
    dest[i] = src[i];
  }
}
