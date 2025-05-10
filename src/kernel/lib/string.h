#ifndef STRING_H
#define STRING_H

#include "common.h"

u32 cstrlen(char *src);

u32 cstrcmp(char *src, char *dest);

void cstrcpy(char *src, char *dest);
u32 startswith(char *search, char *target);
u32 strincl(char *in, char *container);

#endif
