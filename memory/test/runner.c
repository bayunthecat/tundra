#include "allocator.h"
#include "test.h"
#include <stddef.h>
#include <stdlib.h>

typedef struct {
  int i;
  char c;
} TestStruct;

void testArenaAllocatorNominal(TestRun *r) {
  Allocator *arena = allocatorArena(1024);
  int *pI = arena->alloc(arena, sizeof(int));
  char *pC = arena->alloc(arena, sizeof(char));
  TestStruct *pTs = arena->alloc(arena, sizeof(TestStruct));
  *pI = 10;
  *pC = 'a';
  pTs->i = 123;
  pTs->c = 's';
  if (*pI != 10) {
    testFail(r, "value mismatch");
  }
  if (*pC != 'a') {
    testFail(r, "value mismatch");
  }
  if (pTs->i != 123) {
    testFail(r, "value mismatch");
  }
  if (pTs->c != 's') {
    testFail(r, "value mismatch");
  }
  free(arena);
}

int main() {
  TestSuite *s = testSuiteMake("arena allocator");
  testRegisterFn(s, "testArenaAllocatorNominal", testArenaAllocatorNominal);
  testRun(s);
  testSuiteFree(s);
  return 0;
}
