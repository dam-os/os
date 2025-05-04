#ifndef TIMER_H
#define TIMER_H

#include "common.h"

void init_timer(void);

u64 mtime_get_raw_time(void);
u64 mtime_get_time(void);
void sleep(u64 ms);

#endif // !TIMER_H
