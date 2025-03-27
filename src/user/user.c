
#include "../kernel/lib/common.h"
#include "user.h"
extern char user_stack_top[];

#pragma GCC push_options
#pragma GCC optimize ("O0")
int syscall(int sysno, void* arg1, void* arg2, void* arg3) {
    int res;
    
    __asm__ __volatile__("ecall"
        : "=r"(res)
        : "r"(sysno), "r"(arg1), "r"(arg2), "r"(arg3)
        : "memory");
    return res;
}
#pragma GCC pop_options

__attribute__((noreturn)) void proc_exit(void) {
    syscall(2, 0, 0, 0);
    for (;;);
}

__attribute__((section(".text.start")))
__attribute__((naked))
void start(void) {
    __asm__ __volatile__(
        "nop \n"
        "nop \n"
        "nop \n"
        "mv sp, %[stack_top] \n"
        "nop \n"
        "nop \n"
        "call main           \n"
        "nop \n"
        "nop \n"
        "call proc_exit           \n"
        :: [stack_top] "r" ((uint64_t)user_stack_top)
    );
}