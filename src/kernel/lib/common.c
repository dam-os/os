#include "common.h"

u32 swap_endian_32(u32 val) {
  return ((val >> 24) & 0x000000ffU) | ((val >> 8) & 0x0000ff00U) |
         ((val << 8) & 0x00ff0000U) | ((val << 24) & 0xff000000U);
}

u64 swap_endian_64(u64 val) {
  return ((val >> 56) & 0x00000000000000ffUL) |
         ((val >> 40) & 0x000000000000ff00UL) |
         ((val >> 24) & 0x0000000000ff0000UL) |
         ((val >> 8) & 0x00000000ff000000UL) |
         ((val << 8) & 0x000000ff00000000UL) |
         ((val << 24) & 0x0000ff0000000000UL) |
         ((val << 40) & 0x00ff000000000000UL) |
         ((val << 56) & 0xff00000000000000UL);
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
