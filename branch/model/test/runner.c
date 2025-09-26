#include "model.h"
#include "test.h"
#include <stdlib.h>

void testBoardNominal(TestRun *r) {
  unsigned int seed = 1758855645;
  int boardVals[4][4] = {
      {0, 1, 0, 0},
      {3, 14, 9, 0},
      {7, 9, 6, 9},
      {4, 6, 8, 4},
  };
  Board *brd = makeBoard(seed, 4, 4);
  if (brd == NULL) {
    fail(r, "failed to create the board");
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      if (boardVals[i][j] != boardValueAt(brd, i, j)) {
        fail(r, "board values mismatch");
      }
    }
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
