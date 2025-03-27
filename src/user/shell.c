#include "user.h"

extern char _binary_shell_bin_start[];
extern char _binary_shell_bin_size[];


int main(void) {
    char msg[] = "Hello from the shell!";
    syscall(1, msg, 0, 0);
}