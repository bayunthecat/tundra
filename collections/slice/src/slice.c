#include "slice.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Slice {
  int len;
  int cap;
  void **store;
};

void grow(Slice *slice) {
  slice->cap = slice->cap * 1.5;
  void **newStore = realloc(slice->store, sizeof(void *) * slice->cap);
  if (!newStore) {
    printf("Failed to reallocate slice store");
    exit(1);
  }
  slice->store = newStore;
}

void sliceAppend(Slice *slice, void *v) {
  int nIdx = slice->len;
  if (nIdx >= slice->cap) {
    grow(slice);
  }
  slice->store[nIdx] = v;
  slice->len++;
}

void *sliceGet(Slice *s, unsigned int index) {
  if (index >= s->len) {
    return NULL;
  }
  return s->store[index];
}

Slice *sliceMake() {
  Slice *s = malloc(sizeof(Slice));
  if (!s) {
    fprintf(stderr, "Failed to allocate slice");
    exit(1);
  }
  s->len = 0;
  s->cap = 10;
  s->store = malloc(sizeof(void *) * s->cap);
  if (s->store == NULL) {
    exit(1);
  }
  memset(s->store, 0, sizeof(void *) * s->cap);
  return s;
}

unsigned int sliceLen(Slice *slice) { return slice->len; }

void sliceFree(Slice *slice) {
  free(slice->store);
  free(slice);
}
