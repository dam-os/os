
#include "user.h"
#include "../kernel/lib/common.h"
extern char user_stack_top[];

__attribute__((naked)) u32 syscall(__attribute__((unused)) u32 sysno,
                                   __attribute__((unused)) void *arg1,
                                   __attribute__((unused)) void *arg2,
                                   __attribute__((unused)) void *arg3) {
  __asm__ __volatile__("ecall \n"
                       "ret");
}

__attribute__((noreturn)) void proc_exit(void) {
  syscall(2, 0, 0, 0);
  for (;;)
    ;
}

__attribute__((section(".text.start"))) __attribute__((naked)) void
start(void) {
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
      "call proc_exit           \n" ::[stack_top] "r"((u64)user_stack_top));
}
