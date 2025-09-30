#include "model.h"
#include "test.h"
#include <stdio.h>
#include <stdlib.h>

void testBoardNominal(TestRun *r) {
  unsigned int seed = 1758855645;
  int boardVals[4][4] = {
      {0, 1, 0, 0},
      {3, 14, 9, 0},
      {7, 9, 6, 9},
      {4, 6, 8, 4},
  };
  Board *brd = boardMake(seed, 4, 4);
  if (brd == NULL) {
    fail(r, "failed to create the board");
  }
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int v = boardValueAt(brd, i, j);
      if (boardVals[i][j] != v) {
        fail(r, "board values mismatch");
      }
      if (v != 0 && !boardConnectedAt(brd, i, j)) {
        fail(r, "expected connected");
      }
    }
  }
  boardFree(brd);
}

void testRotate(TestRun *r) {
  unsigned int seed = 1758855645;
  int boardVals[4][4] = {
      {0, 8, 0, 0},
      {3, 14, 9, 0},
      {7, 9, 6, 9},
      {4, 6, 8, 4},
  };
  int connVals[4][4] = {
      {0, 0, 0, 0},
      {1, 1, 1, 0},
      {1, 1, 1, 1},
      {1, 1, 1, 1},
  };
  Board *brd = boardMake(seed, 4, 4);
  if (brd == NULL) {
    fail(r, "failed to create the board");
  }
  boardRotateAt(brd, 0, 1);
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      int v = boardValueAt(brd, i, j);
      if (boardVals[i][j] != v) {
        fail(r, "board values mismatch");
      }
      if (connVals[i][j] != boardConnectedAt(brd, i, j)) {
        fail(r, "expected connected");
      }
    }
  }
  boardFree(brd);
}

int main() {
  TestSuite *s = testSuiteMake("model");
  registerFn(s, "testBoardNominal", testBoardNominal);
  registerFn(s, "testRotate", testRotate);
  run(s);
  testSuiteFree(s);
  return 0;
}
