#include "test.h"
#include <stdio.h>
#include <stdlib.h>

struct TestSuite {
  char *name;
  // TODO combine that into a separate struct?
  test *testFns;
  char **testFnNames;
  char **statuses;
  char **reasons;
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
  s->statuses = malloc(sizeof(char *) * s->capacity);
  if (!s->statuses) {
    fprintf(stderr, "Failed to allocate test suite statuses\n");
    exit(1);
  }
  s->testFnNames = malloc(sizeof(char *) * s->capacity);
  if (!s->statuses) {
    fprintf(stderr, "Failed to allocate test suite function names\n");
    exit(1);
  }
  s->testFns = malloc(sizeof(test) * s->capacity);
  if (!s->statuses) {
    fprintf(stderr, "Failed to allocate test suite functions\n");
    exit(1);
  }
  s->reasons = malloc(sizeof(char *) * s->capacity);
  if (!s->statuses) {
    fprintf(stderr, "Failed to allocate test suite reasons\n");
    exit(1);
  }
  return s;
}

void run(TestSuite *s) {
  printf("=== %s ===\n", s->name);
  TestRun run;
  run.s = s;
  for (int i = 0; i < s->testFnCount; i++) {
    char *n = s->testFnNames[i];
    run.idx = i;
    s->testFns[i](&run);
    printf("%s: %s\n", s->statuses[i], s->testFnNames[i]);
  }
}

void registerFn(TestSuite *s, char *name, test fn) {
  int idx = s->testFnCount;
  if (idx >= s->capacity) {
    printf("Exceeds suite capacity of %d\n", s->capacity);
    exit(1);
  }
  s->statuses[idx] = "PASS";
  s->testFnNames[idx] = name;
  s->testFns[idx] = fn;
  s->testFnCount++;
}

void fail(TestRun *r, char *reason) {
  r->s->statuses[r->idx] = "FAIL";
  r->s->reasons[r->idx] = reason;
  r->s->failCount++;
}

void testSuiteFree(TestSuite *s) {
  free(s->testFns);
  free(s->testFnNames);
  free(s->statuses);
  free(s->reasons);
  free(s);
}
