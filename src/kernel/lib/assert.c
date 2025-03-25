#include "assert.h"
#include "../drivers/system.h"
#include "print.h"

void assert(short condition, char *msg) {
  if (condition)
    return;
  cprintf("Assertion failed: %s", msg);
  poweroff();
}
