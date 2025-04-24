#include "user.h"

extern char _binary_shell_bin_start[];
extern char _binary_shell_bin_size[];

int main(void) {
  char str[] = "what is poppin logang";
  while (1) {
    char x = syscall(3, 0, 0, 0);
    // syscall(4, (void *)x, 0, 0);
  }
}
