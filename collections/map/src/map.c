#include "map.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const uint64_t DEFAULT_CAP = 16;

const float DEFAULT_LOAD_FACTOR = 0.75f;

typedef struct {
  void *key;
  void *data;
} Entry;

struct Map {
  MapOpts *opts;
  Entry **store;
  uint64_t len;
  uint64_t cap;
};

// fnv-1a 64, treat any key as a char* of length len
uint64_t byteHash(void *k, size_t size) {
  uint64_t h = 14695981039346656037ull;
  const char *s = (const char *)k;
  for (size_t i = 0; i < size; i++) {
    h ^= s[i];
    h *= 1099511628211ull;
  }
  return h;
}

// fnv-1a 64
uint64_t charPtrHash(char *k) {
  uint64_t h = 14695981039346656037ull;
  int i = 0;
  char c = 0;
  do {
    c = k[i++];
    h ^= c;
    h *= 1099511628211ull;
  } while (c != '\0');
  return h;
}

uint64_t addressHash(void *k) { return (uint64_t)k; }

MapOpts defaultOpts = {
    .hashFn = addressHash,
    .initCap = DEFAULT_CAP,
    .loadFactor = DEFAULT_LOAD_FACTOR,
};

Map *mapMake(MapOpts *mapOpts) {
  Map *m = malloc(sizeof(Map));
  MapOpts *opts = malloc(sizeof(MapOpts));
  memcpy(opts, mapOpts, sizeof(MapOpts));
  m->opts = opts;
  m->len = 0;
  m->cap = opts->initCap;
  m->store = malloc(sizeof(Entry *) * m->cap);
  return m;
}

Map *mapMakeDef() { return mapMake(&defaultOpts); }

void grow(Map *m) {
  uint64_t oldCap = m->cap;
  m->cap = oldCap * 1.5;
  Entry **newStore = malloc(sizeof(Entry *) * m->cap);
  if (!newStore) {
    // TODO bad error handling
    printf("malloc failed\n");
    exit(1);
  }
  Entry **oldStore = m->store;
  m->store = newStore;
  for (uint64_t i = 0; i < oldCap; i++) {
    Entry *e = oldStore[i];
    mapPut(m, e->key, e->data);
  }
  free(oldStore);
}

void mapPut(Map *m, void *k, void *v) {
  // TODO does overwrite
  // Does not invoke grow
  // Does not use cap
  uint64_t h = m->opts->hashFn(k);
  uint64_t bucket = h % m->cap;
  Entry *e = malloc(sizeof(Entry));
  e->data = v;
  e->key = k;
  if (m->store[bucket]) {
    //
  }
  m->store[bucket] = e;
  m->len++;
}

void *mapGet(Map *m, void *k) {
  uint64_t h = m->opts->hashFn(k);
  uint64_t bucket = h % m->cap;
  Entry *e = m->store[bucket];
  if (e != NULL) {
    return e->data;
  }
  return NULL;
}

int mapLen(Map *m) { return m->len; }

void mapFree(Map *m) {
  free(m->opts);
  for (uint64_t i = 0; i < m->len; i++) {
  }
  free(m);
}
