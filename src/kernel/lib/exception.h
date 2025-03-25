#ifndef EXCEPTION_H
#define EXCEPTION_H

#include "common.h"

#define READ_CSR(reg)                                                          \
  ({                                                                           \
    unsigned long __tmp;                                                       \
    __asm__ __volatile__("csrr %0, " #reg : "=r"(__tmp));                      \
    __tmp;                                                                     \
  })

#define WRITE_CSR(reg, value)                                                  \
  do {                                                                         \
    uint64 __tmp = (value);                                                    \
    __asm__ __volatile__("csrw " #reg ", %0" ::"r"(__tmp));                    \
  } while (0)

void kernel_entry(void);

int syscall(int sysno, int arg0, int arg1, int arg2);
#endif // !EXCEPTION_H
