#include "mem.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct Arena {
  uint8_t *memory;
  size_t cap;
  size_t offset;
};

Arena *arenaMake(size_t cap) {
  Arena *a = malloc(sizeof(Arena));
  a->memory = malloc(cap);
  a->cap = cap;
  a->offset = 0;
  return a;
}

void arenaFree(Arena *a) {
  free(a->memory);
  free(a);
}

void *arenaAlloc(Arena *a, size_t size) {
  if (a->cap - a->offset < size) {
    fprintf(stderr, "arena out of memory");
    exit(1);
  }
  void *p = a->offset + a->memory;
  a->offset += size;
  return p;
}
