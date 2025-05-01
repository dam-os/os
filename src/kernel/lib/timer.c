#include "timer.h"
#include "../drivers/device_tree.h"
#include "common.h"
#include "print.h"
#include "string.h"

u32 RISCV_CLINT_ADDR = NULL;
u64 *RISCV_MTIME_ADDR = NULL;
u32 TIMEBASE_FREQUENCY = NULL;

/**
 * Finds the timebase frequency from the device tree to calibrate the timer.
 */
void init_timer(void) {
  // Get RISCV_CLINT_ADDR
  char *clint = match_node("clint@");
  RISCV_CLINT_ADDR = get_node_addr(clint);

  // Get TIMEBASE_FREQUENCY
  u32 *freq = match_node("cpus>timebase-frequency");
  TIMEBASE_FREQUENCY = swap_endian_32(*freq);
  RISCV_MTIME_ADDR = (u64 *)(RISCV_CLINT_ADDR + 0xBFF8UL);

  cprintf("[timer] Timer initialised with frequency %d\n", TIMEBASE_FREQUENCY);
}

u64 mtime_get_raw_time(void) {
  volatile const uint64_t *const mtime =
      (volatile uint64_t *)(RISCV_MTIME_ADDR);
  return *mtime;
}

u64 mtime_get_time(void) {
  u64 time = mtime_get_raw_time();

  u64 ms = time / (TIMEBASE_FREQUENCY / 1000);
  return ms;
}

void sleep(u64 ms) {
  u64 start = mtime_get_time();
  while (mtime_get_time() - start < ms) {
    __asm__ __volatile__("nop");
  }
}
