#include "queue.h"
#include "test.h"

void testQueueNominal(TestRun *r) {
  Queue *q = queueMake();
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
  queueFree(q);
}

int main() {
  TestSuite *s = testSuiteMake("queue");
  registerFn(s, "testQueueNominal", testQueueNominal);
  run(s);
  testSuiteFree(s);
}
