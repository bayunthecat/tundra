#include "allocator.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Arena {
  uint8_t *memory;
  size_t cap;
  size_t offset;
} Arena;

void arenaFree(Allocator *alloc, void *ptr) {
  // no operation for arena allocator
}

void *arenaAlloc(Allocator *alloc, size_t size) {
  Arena *a = alloc->ctx;
  if (a->cap - a->offset < size) {
    // TODO come up with proper error handling
    fprintf(stderr, "arena out of memory");
    exit(1);
  }
  void *p = a->offset + a->memory;
  a->offset += size;
  return p;
}

Allocator *allocatorArena(size_t size) {
  void *arenaAllocMem = malloc(sizeof(Allocator) + sizeof(Arena) + size);
  Arena *arena = arenaAllocMem + sizeof(Allocator);
  arena->memory = arenaAllocMem + sizeof(Allocator) + sizeof(Arena);
  arena->cap = size;
  arena->offset = 0;
  Allocator *allocator = arenaAllocMem;
  allocator->ctx = arena;
  allocator->alloc = arenaAlloc;
  allocator->free = arenaFree;
  return allocator;
}
