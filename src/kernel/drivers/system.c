#include "system.h"
#include "../lib/common.h"
#include "../lib/print.h"
#include "../lib/string.h"
#include "device_tree.h"

uptr SYSCON_ADDR = NULL;
u32 POWEROFF_VALUE = NULL;
u32 POWEROFF_OFFSET = NULL;
u32 POWEROFF_REGMAP = NULL;

void init_system(void) {
  // Get poweroff node
  POWEROFF_VALUE = swap_endian_32(*(u32 *)match_node("poweroff*value"));
  POWEROFF_OFFSET = *(u32 *)match_node("poweroff*offset");
  POWEROFF_REGMAP = *(u32 *)match_node("poweroff*regmap");

  // Get node with phandle = POWEROFF_REGMAP
  fdt_node_t *syscon = find_node_by_phandle(POWEROFF_REGMAP);
  SYSCON_ADDR = get_node_addr(syscon->name);
  free_node(syscon);

  print("[system] System functions initialised, ready to call poweroff\n");
}

void poweroffmsg(char *msg) {
  print(msg);
  *(uint32_t *)(SYSCON_ADDR + POWEROFF_OFFSET) = POWEROFF_VALUE;
}

void poweroff(void) { poweroffmsg("Poweroff requested\n"); }
