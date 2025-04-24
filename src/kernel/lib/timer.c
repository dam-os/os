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
  fdt_node_t *clint = find_node_by_name("clint@");
  RISCV_CLINT_ADDR = get_node_addr(clint->name);
  free_node(clint);

  // Get TIMEBASE_FREQUENCY
  fdt_node_t *cpus = find_node_by_name("cpus");
  for (size_t i = 0; i < cpus->property_count; i++) {
    if (cstrcmp((char *)cpus->properties[i].name, "timebase-frequency") == 0) {
      TIMEBASE_FREQUENCY = swap_endian_32(*(u32 *)cpus->properties[i].value);
      cprintf("[timer] Set timebase frequency: %d\n", TIMEBASE_FREQUENCY);
      break;
    }
  }

  free_node(cpus);

  RISCV_MTIME_ADDR = (u64 *)(RISCV_CLINT_ADDR + 0xBFF8UL);
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
