#include "user.h"

extern char _binary_shell_bin_start[];
extern char _binary_shell_bin_size[];


int main(void) {
    char msg[] = "Hello from the shell!\n";
    syscall(1, msg, (void *)0x21, (void *)0x22);
    syscall(1, msg, (void *)0x21, (void *)0x22);
    syscall(1, msg, 0, 0);
    syscall(1, msg, 0, 0);
}