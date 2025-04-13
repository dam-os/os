#ifndef TIMER_H
#define TIMER_H

#include "common.h"

#define RISCV_CLINT_ADDR 0x2000000UL
#define RISCV_MTIME_ADDR (RISCV_CLINT_ADDR + 0xBFF8UL)

void init_timer(void);

u64 mtime_get_raw_time(void);
u64 mtime_get_time(void);
void sleep(u64 ms);

#endif // !TIMER_H
