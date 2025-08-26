#ifndef PEON_H
#define PEON_H

#include "slice.h"

typedef struct PeonTarget {
  char *path;
  Slice *srcs;
  Slice *includes;
  Slice *flags;
  Slice *tests;
} PeonTarget;

#endif // !PEON_H
