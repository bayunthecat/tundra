#include "mem.h"
#include "queue.h"
#include "test.h"
#include <stdlib.h>

void testQueueNominal(TestRun *r) {
  Arena *a = arenaMake(1024);
  Queue *q = queueMake(a);
  int v1 = 100;
  int v2 = 200;
  queueOffer(q, &v1);
  queueOffer(q, &v2);
  int len = queueLen(q);
  if (len != 2) {
    fail(r, "want len 2, got another");
  }
  int *pollV1 = queuePoll(q);
  if (*pollV1 != v1) {
    fail(r, "values do not match");
  }
  int *pollV2 = queuePoll(q);
  if (*pollV2 != v2) {
    fail(r, "values do not match");
  }
  arenaFree(a);
}

void populateIntArray(int arr[], int len) {
  for (int i = 0; i < len; i++) {
    arr[i] = rand();
  }
}

void testQueueLarge(TestRun *r) {
  int len = 100000;
  int random[len];
  populateIntArray(random, len);
  Arena *a = arenaMake(1800000);
  Queue *q = queueMake(a);
  for (int i = 0; i < len; i++) {
    queueOffer(q, &random[i]);
  }
  int *pPolled;
  int polled;
  for (int i = 0; i < len; i++) {
    pPolled = queuePoll(q);
    polled = *pPolled;
    if (polled != random[i]) {
      fail(r, "values mismatch");
    }
  }
  arenaFree(a);
}

int main() {
  TestSuite *s = testSuiteMake("queue");
  registerFn(s, "testQueueNominal", testQueueNominal);
  registerFn(s, "testQueueLarge", testQueueLarge);
  run(s);
  testSuiteFree(s);
}
