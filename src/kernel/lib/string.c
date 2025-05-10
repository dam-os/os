#include "print.h"
int cstrlen(char *src) {
  int len = 0;
  while (*src != '\0') {
    src += 1;
    len++;
  }
  return len;
}

int cstrcmp(char *src, char *dest) {
  int l = cstrlen(src);
  if (l != cstrlen(dest))
    return -1;
  for (int i = 0; i < l; i++) {
    if (src[i] ^ dest[i]) {
      return -1;
    }
  }

  return 0;
}

int startswith(char *search, char *target) {
  int l = cstrlen(search);
  // can't have a search longer than the target now
  if (l > cstrlen(target))
    return -2;

  for (int i = 0; i < l; i++) {
    if (search[i] ^ target[i]) {
      return -1;
    }
  }

  return 0;
}

int strincl(char *in, char *container) {
  int len = cstrlen(in);
  int clen = cstrlen(container);
  int test = 0;
  int i = 0;
  while (i < clen && test != len) {
    if (container[i] == in[test]) {
      test++;
    } else {
      test = 0;

      if (container[i - 1] == in[test]) {
        test++;
        continue;
      }
    }
    i++;
  }
  return test == len;
}

void cstrcpy(char *src, char *dest) {
  for (int i = 0; i < cstrlen(src); i++) {
    dest[i] = src[i];
  }
}
