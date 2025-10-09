#ifndef MAP_H
#define MAP_H

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

typedef struct Map Map;

typedef uint64_t (*hash)(void *);

typedef struct {
  float loadFactor;
  int initCap;
  hash hashFn;
} MapOpts;

uint64_t charPtrHash(char *key);

void mapPut(Map *m, void *k, void *v);

void *mapGet(Map *m, void *k);

int mapLen(Map *m);

Map *mapMake(MapOpts *opts);

Map *mapMakeDef();

void mapFree(Map *m);

#endif
