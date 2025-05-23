#ifndef SYSTEM_H
#define SYSTEM_H

#include "../lib/io.h"

#define PANIC(fmt, ...)                                                        \
  do {                                                                         \
    cprintf("PANIC: %s:%d " fmt "\r\n", __FILE__, __LINE__, ##__VA_ARGS__);    \
    while (1) {                                                                \
    }                                                                          \
  } while (0)

void init_system(void);
void poweroff(void);
void poweroffmsg(char *msg);
void poweroff_qemu(void);
#endif /* !SYSTEM_H */
