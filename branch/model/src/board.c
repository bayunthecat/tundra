#include "model.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

struct Tile {
  int val;
};

struct Board {
  Tile **tiles;
  int width;
  int height;
};

void generateBoard(Board *brd) { Queue *q = queueMake(); }

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
