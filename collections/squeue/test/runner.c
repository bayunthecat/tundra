#include "squeue.h"
#include "test.h"

void testSQueueNominal(TestRun *r) {
  int stoargeSize = 10;
  SQueue q = {};
  void *storage[stoargeSize];
  sQueueInit(&q, storage, stoargeSize);
  int offerOne = 1, offerTwo = 2;
  sQueueOffer(&q, &offerOne);
  sQueueOffer(&q, &offerTwo);
  int pollOne = *(int *)sQueuePoll(&q);
  int pollTwo = *(int *)sQueuePoll(&q);
  if (pollOne != offerOne) {
    testFail(r, "values mismatch");
  }
  if (pollTwo != offerTwo) {
    testFail(r, "values mismatch");
  }
}

void testSQueueWrapAround(TestRun *r) {
  int storeSize = 3;
  SQueue q = {};
  void *storage[storeSize];
  sQueueInit(&q, storage, storeSize);
  int offerOne = 1, offerTwo = 2, offerThree = 3;
  sQueueOffer(&q, &offerOne);
  sQueueOffer(&q, &offerTwo);
  sQueueOffer(&q, &offerThree);
  int pollOne = *(int *)sQueuePoll(&q);
  if (pollOne != offerOne) {
    testFail(r, "values mismatch");
  }
  int offerFour = 4;
  sQueueOffer(&q, &offerFour);
  sQueuePoll(&q);
  sQueuePoll(&q);
  int pollFour = *(int *)sQueuePoll(&q);
  if (pollFour != offerFour) {
    testFail(r, "values mismatch");
  }
}

int main() {
  TestSuite *s = testSuiteMake("squeue");
  testRegisterFn(s, "testSQueueNominal", testSQueueNominal);
  testRegisterFn(s, "testSQueueWrapAround", testSQueueWrapAround);
  testRun(s);
  testSuiteFree(s);
  return 0;
}
