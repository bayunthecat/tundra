#include "map.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint64_t myHash(void *key) { return charPtrHash(key); }

void testMapNominal() {
  MapOpts opts = {
      .initCap = 20,
      .hashFn = myHash,
      .loadFactor = 0.75,
  };
  Map *m = mapMake(&opts);
  mapPut(m, "hello", "helloVal");
  mapPut(m, "word", "wordVal");
  char *helloVal = mapGet(m, "hello");
  if (strcmp(helloVal, "helloVal") != 0) {
    printf("want: %s, got: %s\n", "helloVal", helloVal);
    exit(1);
  }
  mapFree(m);
}

void testMapGrow() {
  MapOpts opts = {
      .initCap = 20,
      .hashFn = myHash,
      .loadFactor = 0.75,
  };
  Map *m = mapMake(&opts);
  for (uint64_t i = 0; i < 100; i++) {
    char *key = malloc(sizeof(char) * 8);
    sprintf(key, "hello%ld", i);
    char *val = malloc(sizeof(char) * 6);
    sprintf(val, "val%ld", i);
    mapPut(m, key, val);
  }
  int len = mapLen(m);
  if (len != 100) {
    printf("want: %d, got %d", 100, len);
    exit(1);
  }
  mapFree(m);
}

int main() {
  testMapNominal();
  testMapGrow();
  printf("Tests: OK\n");
  return 0;
}
