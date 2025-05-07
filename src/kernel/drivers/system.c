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

void init_system(void) {
  // Get poweroff node
  POWEROFF_VALUE = swap_endian_32(*(u32 *)match_node("poweroff*value"));
  POWEROFF_OFFSET = *(u32 *)match_node("poweroff*offset");
  POWEROFF_REGMAP = *(u32 *)match_node("poweroff*regmap");

  // Get node with phandle = POWEROFF_REGMAP
  char *search = kmalloc(32);
  csprintf(search, "[phandle=<%x>]", swap_endian_32(POWEROFF_REGMAP));
  char *syscon = match_node(search);
  SYSCON_ADDR = get_node_addr(syscon);

  print("[system] System functions initialised, ready to call poweroff\n");
}

void poweroffmsg(char *msg) {
  print(msg);
  *(u32 *)(SYSCON_ADDR + POWEROFF_OFFSET) = POWEROFF_VALUE;
}

void poweroff(void) { poweroffmsg("Poweroff requested\n"); }
