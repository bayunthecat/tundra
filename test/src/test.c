#include "test.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  uint8_t *memory;
  size_t offset;
  size_t capacity;
} StringPool;

char *buff(size_t size, StringPool *pool) {
  if (pool->offset + size < pool->capacity) {
    void *b = pool->memory + pool->offset;
    memset(b, 0, size);
    pool->offset += size;
    return b;
  }
  fprintf(stderr, "test string pool is out of memory");
  exit(1);
}

typedef struct {
  test fn;
  char *fnName;
  char *status;
  char *reason;
} TestCase;

struct TestSuite {
  char *name;
  TestCase *cases;
  int caseCount;
  int capacity;
  int failCount;
  StringPool *pool;
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
  s->caseCount = 0;
  s->failCount = 0;
  s->name = name;
  s->capacity = 20;

  s->cases = malloc(sizeof(TestCase) * s->capacity);
  if (!s->cases) {
    fprintf(stderr, "Failed to allocate test suite cases");
  }
  // TODO need improvement
  s->pool = malloc(sizeof(StringPool));
  s->pool->memory = malloc(1024);
  return s;
}

void testRun(TestSuite *s) {
  printf("===== %s =====\n", s->name);
  TestRun run;
  run.s = s;
  for (int i = 0; i < s->caseCount; i++) {
    TestCase *c = &s->cases[i];
    char *n = s->cases[i].fnName;
    run.idx = i;
    s->cases[i].fn(&run);
    printf("%s: %s %s\n", c->status, c->fnName, c->reason);
  }
  int pass = s->caseCount - s->failCount;
  printf("\n%d out of %d tests PASS\n", pass, s->caseCount);
}

char *makeString(TestRun *r, char *want, char *got) {
  int size = snprintf(NULL, 0, "want: %s, got: %s\n", want, got) + 1;
  char *b = buff(size, r->s->pool);
  snprintf(b, size, "want: %s, got: %s", want, got);
  return b;
}

void verify(TestRun *r, void *want, size_t wantS, void *got, size_t gotS) {
  if (wantS != gotS) {
    testFail(r, "size mismatch");
    return;
  }
  char *cWant = want;
  char *cGot = got;
  int index;
  while (index < wantS) {
    if (cWant[index] != cGot[index]) {
      testFail(r, makeString(r, cWant, cGot));
    }
    index++;
  }
}

void testRegisterFn(TestSuite *s, char *name, test fn) {
  int idx = s->caseCount;
  if (idx >= s->capacity) {
    int newCap = s->capacity * 1.5;
    TestCase *reCase = realloc(s->cases, newCap);
    if (!reCase) {
      fprintf(stderr, "Failed to reallocated test suite cases");
      exit(1);
    }
  }
  TestCase *c = &s->cases[idx];
  c->status = "PASS";
  c->fnName = name;
  c->reason = strdup("");
  c->fn = fn;
  s->caseCount++;
}

void testFail(TestRun *r, char *reason) {
  TestCase *c = &r->s->cases[r->idx];
  c->status = "FAIL";
  char *reasonDup = strdup(reason);
  if (!reasonDup) {
    fprintf(stderr, "string duplication failed\n");
    exit(1);
  }
  c->reason = reasonDup;
  r->s->failCount++;
}

void testSuiteFree(TestSuite *s) {
  for (int i = 0; i < s->caseCount; i++) {
    free(s->cases[i].reason);
  }
  free(s->cases);
  free(s);
}
