#include "common.h"

u32 swap_endian_32(u32 val) {
  return ((val >> 24) & 0xff) | ((val >> 8) & 0xff00) |
         ((val << 8) & 0xff0000) | ((val << 24) & 0xff000000);
}
