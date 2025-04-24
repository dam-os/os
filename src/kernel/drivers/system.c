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
  fdt_node_t *poweroff = find_node_by_name("poweroff");
  for (size_t i = 0; i < poweroff->property_count; i++) {
    char *name = (char *)poweroff->properties[i].name;
    u32 *value = poweroff->properties[i].value;
    if (cstrcmp(name, "value") == 0) {
      POWEROFF_VALUE = swap_endian_32(*value);
    } else if (cstrcmp(name, "offset") == 0) {
      POWEROFF_OFFSET = *value;
    } else if (cstrcmp(name, "regmap") == 0) {
      POWEROFF_REGMAP = *value;
    }
  }
  free_node(poweroff);

  // Get node with phandle = POWEROFF_REGMAP
  fdt_node_t *syscon = find_node_by_phandle(POWEROFF_REGMAP);
  SYSCON_ADDR = get_node_addr(syscon->name);
  free_node(syscon);
}

void poweroffmsg(char *msg) {
  print(msg);
  *(uint32_t *)(SYSCON_ADDR + POWEROFF_OFFSET) = POWEROFF_VALUE;
}

void poweroff(void) { poweroffmsg("Poweroff requested\n"); }
