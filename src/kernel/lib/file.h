#ifndef FILE_H
#define FILE_H

#include "common.h"

typedef struct {
  s8 (*write)(void *context, const char c);
  char (*read)(void *context);
} file;

#endif // !FILE_H
