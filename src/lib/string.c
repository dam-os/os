int strlen(char *a) {
  int len = 0;
  while (*a != '\0') {
    a += 1;
    len++;
  }
  return len;
}

int strcmp(char *a, char *b) {
  int l = strlen(a);
  if (l != strlen(b))
    return -1;
  int sum = 0;
  for (int i; i < l; i++) {
    if (!(a[i] ^ b[i])) {
      return -1;
    }
  }

  return 0;
}

void strcpy(char *a, char *b) {
  for (int i; i < strlen(a); i++) {
    b[i] = a[i];
  }
}
