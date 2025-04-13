#include "timer.h"
#include "../drivers/device_tree.h"
#include "common.h"
#include "print.h"
#include "string.h"

u32 TIMEBASE_FREQUENCY = NULL;

/**
 * Finds the timebase frequency from the device tree to calibrate the timer.
 */
void init_timer(void) {
  fdt_node_t *node = find_fdt("cpus");
  for (size_t i = 0; i < node->property_count; i++) {
    if (cstrcmp((char *)node->properties[i].name, "timebase-frequency") == 0) {
      TIMEBASE_FREQUENCY = swap_endian_32(*(u32 *)node->properties[i].value);
      cprintf("[timer] Set timebase frequency: %d\n", TIMEBASE_FREQUENCY);
      break;
    }
  }

  free_node(node);
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
