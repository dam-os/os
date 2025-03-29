#include "system.h"
#include "../lib/common.h"
#include "../lib/print.h"

#define SYSCON_ADDR 0x100000

void poweroffmsg(char *msg) {
  print(msg);
  *(uint32_t *)SYSCON_ADDR = 0x5555;
}

void poweroff(void) { poweroffmsg("Poweroff requested\n"); }
