#include "timer.h"
#include "../drivers/device_tree.h"
#include "common.h"
#include "io.h"
#include "string.h"

void *RISCV_CLINT_ADDR = NULL;
u64 *RISCV_MTIME_ADDR = NULL;
u32 TIMEBASE_FREQUENCY = NULL;

u64 stopwatch_last_timestamp = 0;

/**
 * Finds the timebase frequency from the device tree to calibrate the timer.
 */
void init_timer(void) {
  // Get RISCV_CLINT_ADDR
  char *clint = match_node("[compatible='sifive,clint0']");
  RISCV_CLINT_ADDR = (void *)get_node_addr(clint);

  // Get TIMEBASE_FREQUENCY
  u32 *freq = match_node("cpus*timebase-frequency");
  TIMEBASE_FREQUENCY = swap_endian_32(*freq);
  RISCV_MTIME_ADDR = (u64 *)(RISCV_CLINT_ADDR + 0xBFF8);

  stopwatch_last_timestamp = mtime_get_microseconds();
  cprintf("[timer] Timer initialised with frequency %d\r\n",
          TIMEBASE_FREQUENCY);
}

u64 mtime_get_raw_time(void) {
  volatile const u64 *const mtime = (volatile u64 *)(RISCV_MTIME_ADDR);
  return *mtime;
}

u64 mtime_get_scaled_time(u32 scale) {
  u64 time = mtime_get_raw_time();
  // Convert to microseconds
  u64 us = time / (TIMEBASE_FREQUENCY / scale);
  return us;
}

u64 mtime_get_microseconds(void) { return mtime_get_scaled_time(1000000); }

u64 mtime_get_milliseconds(void) { return mtime_get_scaled_time(1000); }

void sleep(u64 ms) {
  u64 start = mtime_get_milliseconds();
  while (mtime_get_milliseconds() - start < ms) {
    __asm__ __volatile__("nop");
  }
}

void stopwatch(const char *message) {
  u64 difference = mtime_get_microseconds() - stopwatch_last_timestamp;
  if (difference > 10000) {
    cprintf("[stopwatch] %s: %d ms\r\n", message, difference / 1000);
  } else {
    cprintf("[stopwatch] %s: %d μs\r\n", message, difference);
  }
  stopwatch_last_timestamp = mtime_get_microseconds();
}
