#ifndef TIMER_H
#define TIMER_H

#include "common.h"

void init_timer(void);

u64 mtime_get_raw_time(void);
u64 mtime_get_nanoseconds(void);
u64 mtime_get_microseconds(void);
u64 mtime_get_milliseconds(void);
void sleep(u64 ms);
void stopwatch(const char *message);

#endif // !TIMER_H
