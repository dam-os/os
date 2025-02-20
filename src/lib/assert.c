#include "assert.h"
#include "system.h"
#include "print.h"

void assert(short condition, char *msg) {
  if (condition) return;
  printfmt("Assertion failed: %s", msg);
  poweroff();
}
