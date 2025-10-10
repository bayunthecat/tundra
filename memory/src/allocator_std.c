#include "allocator.h"
#include <stddef.h>
#include <stdlib.h>

void *stdAlloc(Allocator *a, size_t alloc) { return NULL; }

void stdFree(Allocator *a, void *ptr) {}

Allocator *allocatorStd() {
  Allocator *a = malloc(sizeof(Allocator));
  a->alloc = stdAlloc;
  a->free = stdFree;
  a->ctx = NULL;
  return a;
}
