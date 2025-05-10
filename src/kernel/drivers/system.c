#include "system.h"
#include "../lib/common.h"
#include "../lib/io.h"
#include "../lib/string.h"
#include "../memory/kheap.h"
#include "device_tree.h"

uptr SYSCON_ADDR = NULL;
u32 POWEROFF_VALUE = NULL;
u32 POWEROFF_OFFSET = NULL;
u32 POWEROFF_REGMAP = NULL;

void (*poweroff_func)(void) = poweroff_qemu;

void poweroff_starfive(void) {
  // Power off PMU devices, and call software encourage to apply it
  u64 pmu = 0x17030000;
  u32 *pmu_ptr = (u32 *)(pmu + 0x10);
  u32 data = *pmu_ptr;
  *pmu_ptr = data | 0b111111;

  u32 *pmu_software_encourage_ptr = (u32 *)(pmu + 0x44);
  *pmu_software_encourage_ptr = 0xFF;
  *pmu_software_encourage_ptr = 0x0A;
  *pmu_software_encourage_ptr = 0xA0;
  cprintf("Should not happen!\r\n");
}

void init_system(void) {
  print("[system] START\r\n");
  
  char* model =  match_node("*model");
  cprintf("[system] model %s\r\n", model);
  // Get poweroff node
  if (startswith("StarFive", model) == 0) {
    print("starfive board");
    poweroff_func = poweroff_starfive;
    return;
  }

  POWEROFF_VALUE = swap_endian_32(*(u32 *)match_node("poweroff*value"));
  POWEROFF_OFFSET = *(u32 *)match_node("poweroff*offset");
  POWEROFF_REGMAP = *(u32 *)match_node("poweroff*regmap");
  

  // Get node with phandle = POWEROFF_REGMAP
  char *search = kmalloc(32);
  csprintf(search, "[phandle=<%x>]", swap_endian_32(POWEROFF_REGMAP));
  char *syscon = match_node(search);
  kfree(search);
  SYSCON_ADDR = get_node_addr(syscon);

  print("[system] System functions initialised, ready to call poweroff\r\n");
}
void poweroff_qemu(void) {
  *(u32 *)(SYSCON_ADDR + POWEROFF_OFFSET) = POWEROFF_VALUE;
}

void poweroffmsg(char *msg) {
  print(msg);
  poweroff_func();
}

void poweroff(void) { poweroffmsg("Poweroff requested\r\n"); }
