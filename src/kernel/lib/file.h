#ifndef FILE_H
#define FILE_H

#include "common.h"

struct __file {
  unsigned char *buf;
  size_t pos;

  s8 (*write)(struct __file *context, const char c);
  char (*read)(struct __file *context);
};
typedef struct __file file;

#endif // !FILE_H
