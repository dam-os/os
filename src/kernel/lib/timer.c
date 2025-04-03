#include "timer.h"

u64 mtime_get_raw_time(void) {
  volatile const uint64_t *const mtime =
      (volatile uint64_t *)(RISCV_MTIME_ADDR);
  return *mtime;
}

u64 mtime_get_time(void) {
  u64 time = mtime_get_raw_time();

  u64 ms = time / (QEMU_TIMEBASE_FREQUENCY / 1000);
  return ms;
}

void sleep(u64 ms) {
  u64 start = mtime_get_time();
  while (mtime_get_time() - start < ms) {
    __asm__ __volatile__("nop");
  }
}
