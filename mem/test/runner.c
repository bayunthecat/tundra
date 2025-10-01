#include "mem.h"
#include "test.h"
#include <stddef.h>

typedef struct {
  int i;
  char c;
} TestStruct;

void testMemoryNominal(TestRun *r) {
  Arena *a = arenaMake(100);
  int *pI = arenaAlloc(a, sizeof(int));
  char *pC = arenaAlloc(a, sizeof(char));
  TestStruct *pTs = arenaAlloc(a, sizeof(TestStruct));
  *pI = 10;
  *pC = 'a';
  pTs->i = 123;
  pTs->c = 's';
  if (*pI != 10) {
    fail(r, "value mismatch");
  }
  if (*pC != 'a') {
    fail(r, "value mismatch");
  }
  if (pTs->i != 123) {
    fail(r, "value mismatch");
  }
  if (pTs->c != 's') {
    fail(r, "value mismatch");
  }
  arenaFree(a);
}

int main() {
  TestSuite *s = testSuiteMake("memory");
  registerFn(s, "testMemoryNominal", testMemoryNominal);
  run(s);
  testSuiteFree(s);
  return 0;
}
