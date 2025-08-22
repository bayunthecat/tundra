#include "test.h"
#include <stdio.h>
#include <stdlib.h>

struct TestSuite {
  int idx;
  char *name;
  test *testFns;
  char **testFnNames;
  char **statuses;
  int testFnCount;
  int capacity;
  int failCount;
};

TestSuite *testSuiteMake(char *name) {
  TestSuite *s = malloc(sizeof(TestSuite));
  s->testFnCount = 0;
  s->failCount = 0;
  s->name = name;
  s->capacity = 20;
  s->statuses = malloc(sizeof(char *) * s->capacity);
  s->testFnNames = malloc(sizeof(char *) * s->capacity);
  s->testFns = malloc(sizeof(test) * s->capacity);
  return s;
}

void run(TestSuite *s) {
  for (int i = 0; i < s->testFnCount; i++) {
    char *n = s->testFnNames[i];
    s->idx = i;
    s->testFns[i](s);
  }
  if (s->failCount > 0) {
    for (int i = 0; i < s->testFnCount; i++) {
      printf("%s: %s\n", s->testFnNames[i], s->statuses[i]);
    }
  } else {
    printf("%s: OK", s->name);
  }
}

void registerFn(TestSuite *s, char *name, test fn) {
  int idx = s->testFnCount;
  if (idx >= s->capacity) {
    printf("Exceeds suite capacity of %d\n", s->capacity);
    exit(1);
  }
  s->statuses[idx] = "OK";
  s->testFnNames[idx] = name;
  s->testFns[idx] = fn;
  s->testFnCount++;
}

void fail(TestSuite *s, char *reason) {
  s->statuses[s->idx] = reason;
  s->failCount++;
}

void testSuiteFree(TestSuite *s) {
  free(s->testFns);
  free(s->testFnNames);
  free(s->statuses);
  free(s);
}
