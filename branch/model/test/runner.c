#include "model.h"
#include "test.h"
#include <stdlib.h>

void testBoardNominal(TestRun *r) {
  Board *brd = makeBoard(4, 4);
  if (brd == NULL) {
    fail(r, "failed to create the board");
  }
  freeBoard(brd);
}

int main() {
  TestSuite *s = testSuiteMake("model");
  registerFn(s, "testBoardNominal", testBoardNominal);
  run(s);
  testSuiteFree(s);
  return 0;
}
