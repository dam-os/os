#include "user.h"
#define SYS_WRITE 1
#define SYS_READ 2
#define SYS_GETCHAR 3
#define SYS_PUTCHAR 4
#define SYS_SLEEP 5
#define SYS_POWEROFF 6
#define SYS_YIELD 8
#define SYS_EXIT 9

extern char _binary_shell_bin_start[];

int main(void) {
  while (1) {
    char x = syscall(SYS_GETCHAR, 0, 0, 0);
    syscall(SYS_PUTCHAR, (void *)(u64)x, (void *)(u64)x, 0);
  }
}
