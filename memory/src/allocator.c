#include "allocator.h"
#include <stddef.h>

void *alloc(Allocator *a, size_t size) { return a->alloc(a, size); }

void dealloc(Allocator *a, void *ptr) { a->free(a, ptr); }
