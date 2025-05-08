#include "common.h"

u32 swap_endian_32(u32 val) {
  return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) |
         ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000);
}

s8 hex_char_to_int(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  } else if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  } else {
    return -1;
  }
}
