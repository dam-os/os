#include "system.h"
#include "common.h"
#include "print.h"

#define SYSCON_ADDR 0x100000

void poweroffmsg(char *msg) {
  print(msg);
  *(uint32 *)SYSCON_ADDR = 0x5555;
}

void poweroff(void) { poweroffmsg("Poweroff requested\n"); }
