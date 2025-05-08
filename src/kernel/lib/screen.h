#ifndef SCREEN_H
#define SCREEN_H

#include "../lib/file.h"
#include "common.h"

extern file stdout_screen;

char sputchar(char c);
s8 screen_write(file *context, const char c);

#endif // !SCREEN_H
