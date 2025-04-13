#include "assert.h"
#include "../drivers/system.h"

void kassert(short condition) {
  if (!condition)
    PANIC("Assertion failed");
}
