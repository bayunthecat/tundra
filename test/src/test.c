#include "test.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  test fn;
  char *fnName;
  char *status;
  char *reason;
} TestCase;

struct TestSuite {
  char *name;
  TestCase *cases;
  int testFnCount;
  int capacity;
  int failCount;
};

struct TestRun {
  TestSuite *s;
  int idx;
};

TestSuite *testSuiteMake(char *name) {
  TestSuite *s = malloc(sizeof(TestSuite));
  if (!s) {
    fprintf(stderr, "Failed to allocate TestSuite");
    exit(1);
  }
  s->testFnCount = 0;
  s->failCount = 0;
  s->name = name;
  s->capacity = 20;

  s->cases = malloc(sizeof(TestCase) * s->capacity);
  if (!s->cases) {
    fprintf(stderr, "Failed to allocate test suite cases");
  }
  return s;
}

void run(TestSuite *s) {
  printf("===== %s =====\n\n", s->name);
  TestRun run;
  run.s = s;
  for (int i = 0; i < s->testFnCount; i++) {
    TestCase *c = &s->cases[i];
    char *n = s->cases[i].fnName;
    run.idx = i;
    s->cases[i].fn(&run);
    printf("%s: %s %s\n", c->status, c->fnName, c->reason);
  }
  int pass = s->testFnCount - s->failCount;
  printf("\n%d out of %d tests PASS\n", pass, s->testFnCount);
}

void registerFn(TestSuite *s, char *name, test fn) {
  int idx = s->testFnCount;
  if (idx >= s->capacity) {
    printf("Exceeds suite capacity of %d\n", s->capacity);
    exit(1);
  }
  TestCase *c = &s->cases[idx];
  c->status = "PASS";
  c->fnName = name;
  c->reason = "";
  c->fn = fn;
  s->testFnCount++;
}

void fail(TestRun *r, char *reason) {
  TestCase *c = &r->s->cases[r->idx];
  c->status = "FAIL";
  c->reason = reason;
  r->s->failCount++;
}

void testSuiteFree(TestSuite *s) {
  free(s->cases);
  free(s);
}
