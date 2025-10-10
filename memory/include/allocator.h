#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include <stddef.h>

typedef struct Allocator {
  void *(*alloc)(struct Allocator *, size_t size);
  void (*free)(struct Allocator *, void *ptr);
  void *ctx;
} Allocator;

Allocator *allocatorArena(size_t size);

Allocator *allocatorStd();

Allocator *allocatorStack(void *buffer, size_t size);

void *alloc(Allocator *a, size_t size);

void dealloc(Allocator *a, void *ptr);

#endif // !ALLOCATOR_H
