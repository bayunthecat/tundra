#include "model.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

struct Tile {
  TileType type;
  int degree;
};

struct Board {
  Tile **tiles;
  int width;
  int height;
};

typedef struct Coord {
  int i, j;
} Coord;

int rotateLeft(int v) { return ((v << 1) & 0b1111) | (v >> 0b0011); }

inline int applyDegree(int val) {
  int shift = val / 90;
  return 0;
}

inline int val(Tile *t) {
  switch (t->type) {
  case T:
    return 0b1110;
  case I:
    return 0b0101;
  case L:
    return 0b0110;
  case E:
    return 0;
  }
}

void generateBoard(Board *brd) {
  Queue *q = queueMake();
  Coord start = {4, 4};
  queueOffer(q, &start);
  int len;
  Coord dir[] = {
      {0, -1},
      {-1, 0},
      {0, 1},
      {1, 0},
  };
  while ((len = queueLen(q)) != 0) {
    for (int i = 0; i < len; i++) {
      Coord *polled = queuePoll(q);
      if (polled->j < 0 || polled->j >= brd->height) {
        continue;
      }
      if (polled->i < 0 || polled->i >= brd->width) {
        continue;
      }
      // check visited, set visited
      for (int j = 0; j < 4; j++) {
        Coord toOffer = {polled->i + dir[j].i, polled->j + dir[j].j};
        queueOffer(q, &toOffer);
      }
    }
  }
}

Board *makeBoard(int width, int height) {
  Board *pBrd = malloc(sizeof(Board));
  pBrd->width = width;
  pBrd->height = height;
  if (pBrd == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  pBrd->tiles = malloc(sizeof(Tile *) * width);
  if (pBrd->tiles == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  for (int i = 0; i < width; i++) {
    pBrd->tiles[i] = malloc(sizeof(Tile) * height);
    if (pBrd->tiles[i] == NULL) {
      printf("malloc failed\n");
      exit(1);
    }
  }
  return pBrd;
}

void freeBoard(Board *brd) {
  for (int i = 0; i < brd->width; i++) {
    free(brd->tiles[i]);
  }
  free(brd->tiles);
  free(brd);
}
