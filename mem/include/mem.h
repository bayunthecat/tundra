#ifndef MEM_H

#include <stddef.h>
#include <stdlib.h>

typedef struct Arena Arena;

Arena *arenaMake(size_t capacity);

void *arenaAlloc(Arena *a, size_t size);

void arenaFree(Arena *a);

#endif // !MEMORY_H
