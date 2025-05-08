#include "user.h"

extern char _binary_shell_bin_start[];

int main(void) {
  while (1) {
    syscall(3, 0, 0, 0);
  }
}
