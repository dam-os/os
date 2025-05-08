#ifndef USER_H
#define USER_H

#include "../kernel/lib/common.h"

u32 syscall(u32 sysno, void *arg0, void *arg1, void *arg2);
#endif
