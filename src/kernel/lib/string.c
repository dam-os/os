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

void cstrcpy(char *src, char *dest) {
  for (int i = 0; i < cstrlen(src); i++) {
    dest[i] = src[i];
  }
}
