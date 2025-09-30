#include "model.h"
#include "queue.h"
#include <endian.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct Tile {
  int v;
  int connected;
};

typedef struct Coord {
  int i, j;
} Coord;

struct Board {
  Tile **tiles;
  int cols;
  int rows;
  Coord source;
};

static inline void shiftRight(int *v, int times) {
  *v = (*v >> times) | (*v << (4 - times) & 0b1111);
}

int **make2DIntArray(int rows, int cols) {
  int **arr = malloc(sizeof(int *) * rows);
  if (arr == NULL) {
    fprintf(stderr, "malloc failed\n");
    exit(1);
  }
  for (int i = 0; i < rows; i++) {
    size_t size = sizeof(int) * cols;
    arr[i] = malloc(size);
    if (arr[i] == NULL) {
      fprintf(stderr, "malloc failed\n");
    }
    memset(arr[i], 0, size);
  }
  return arr;
}

void free2DIntArray(int **arr, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    free(arr[i]);
  }
  free(arr);
}

int roll50P() {
  int result = rand() % 100;
  return result < 50;
}

static inline int hasLeft(Tile *t) { return (t->v & 0b1000) >> 3; }

static inline int hasRight(Tile *t) { return (t->v & 0b0010) >> 1; }

static inline int hasUp(Tile *t) { return (t->v & 0b0100) >> 2; }

static inline int hasDown(Tile *t) { return t->v & 0b0001; }

static inline Tile *tileAt(Board *brd, Coord *c) {
  return &brd->tiles[c->i][c->j];
}

Coord *makeCoord(int i, int j) {
  Coord *c = malloc(sizeof(Coord));
  c->i = i;
  c->j = j;
  return c;
};

void generateBoard(Board *brd, int seed) {
  srand(seed);
  int **visited = make2DIntArray(brd->rows, brd->cols);
  int **reserved = make2DIntArray(brd->rows, brd->cols);
  Queue *q = queueMake();
  Coord *start = makeCoord(brd->source.i, brd->source.j);
  queueOffer(q, start);
  int len;
  while ((len = queueLen(q)) != 0) {
    for (int i = 0; i < len; i++) {
      Coord *p = queuePoll(q);
      if (visited[p->i][p->j]) {
        free(p);
        continue;
      }
      visited[p->i][p->j] = 1;
      int genValue = 0;
      int connections = 0;
      // mandatory connections
      if (p->j - 1 >= 0) {
        if (hasRight(&brd->tiles[p->i][p->j - 1])) {
          connections++;
          genValue = genValue | 0b1000;
        }
      }
      if (p->i - 1 >= 0) {
        if (hasDown(&brd->tiles[p->i - 1][p->j])) {
          connections++;
          genValue = genValue | 0b0100;
        }
      }
      if (p->j + 1 < brd->cols) {
        if (hasLeft(&brd->tiles[p->i][p->j + 1])) {
          connections++;
          genValue = genValue | 0b0010;
        }
      }
      if (p->i + 1 < brd->rows) {
        if (hasUp(&brd->tiles[p->i + 1][p->j])) {
          connections++;
          genValue = genValue | 0b0001;
        }
      }
      // generate new connections
      if (p->j - 1 >= 0) {
        if (!reserved[p->i][p->j - 1] && connections < 3 && roll50P()) {
          connections++;
          genValue = genValue | 0b1000;
          reserved[p->i][p->j - 1] = 1;
          Coord *o = makeCoord(p->i, p->j - 1);
          queueOffer(q, o);
        }
      }
      if (p->i - 1 >= 0) {
        if (!reserved[p->i - 1][p->j] && connections < 3 && roll50P()) {
          connections++;
          genValue = genValue | 0b0100;
          reserved[p->i - 1][p->j] = 1;
          Coord *o = makeCoord(p->i - 1, p->j);
          queueOffer(q, o);
        }
      }
      if (p->j + 1 < brd->cols) {
        if (!reserved[p->i][p->j + 1] && connections < 3 && roll50P()) {
          connections++;
          genValue = genValue | 0b0010;
          reserved[p->i][p->j + 1] = 1;
          Coord *o = makeCoord(p->i, p->j + 1);
          queueOffer(q, o);
        }
      }
      if (p->i + 1 < brd->rows) {
        if (!reserved[p->i + 1][p->j] && connections < 3 && roll50P()) {
          connections++;
          genValue = genValue | 0b0001;
          Coord *o = makeCoord(p->i + 1, p->j);
          reserved[p->i + 1][p->j] = 1;
          queueOffer(q, o);
        }
      }
      brd->tiles[p->i][p->j].v = genValue;
      free(p);
    }
  }
  free2DIntArray(visited, brd->rows, brd->rows);
  free2DIntArray(reserved, brd->cols, brd->cols);
  queueFree(q);
}

void resetConnected(Board *brd) {
  for (int i = 0; i < brd->rows; i++) {
    for (int j = 0; j < brd->cols; j++) {
      brd->tiles[i][j].connected = 0;
    }
  }
}

void markConnected(Board *brd, int i, int j) {
  Tile *t = &brd->tiles[i][j];
  t->connected = 1;
  if (hasLeft(t)) {
    if (j - 1 >= 0 && hasRight(&brd->tiles[i][j - 1]) &&
        !brd->tiles[i][j - 1].connected) {
      markConnected(brd, i, j - 1);
    }
  }
  if (hasUp(t)) {
    if (i - 1 >= 0 && hasDown(&brd->tiles[i - 1][j]) &&
        !brd->tiles[i - 1][j].connected) {
      markConnected(brd, i - 1, j);
    }
  }
  if (hasRight(t)) {
    if (j + 1 < brd->cols && hasLeft(&brd->tiles[i][j + 1]) &&
        !brd->tiles[i][j + 1].connected) {
      markConnected(brd, i, j + 1);
    }
  }
  if (hasDown(t)) {
    if (i + 1 < brd->rows && hasUp(&brd->tiles[i + 1][j]) &&
        !brd->tiles[i + 1][j].connected) {
      markConnected(brd, i + 1, j);
    }
  }
}

void markConnectedFromSource(Board *brd) {
  resetConnected(brd);
  markConnected(brd, brd->source.i, brd->source.j);
}

void boardRotateAt(Board *brd, int row, int col) {
  shiftRight(&brd->tiles[row][col].v, 1);
  markConnectedFromSource(brd);
}

int boardValueAt(Board *brd, int row, int col) {
  return brd->tiles[row][col].v;
}

int boardConnectedAt(Board *brd, int row, int col) {
  return brd->tiles[row][col].connected;
}

void printBoard(Board *brd) {
  for (int i = 0; i < brd->rows; i++) {
    for (int j = 0; j < brd->cols; j++) {
      printf("%d ", brd->tiles[i][j].v);
    }
    printf("\n");
  }
}

Board *boardMake(int seed, int rows, int cols) {
  Board *pBrd = malloc(sizeof(Board));
  pBrd->cols = cols;
  pBrd->rows = rows;
  pBrd->source.i = rows / 2;
  pBrd->source.j = cols / 2;
  if (pBrd == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  pBrd->tiles = malloc(sizeof(Tile *) * rows);
  if (pBrd->tiles == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  for (int i = 0; i < cols; i++) {
    pBrd->tiles[i] = malloc(sizeof(Tile) * rows);
    if (pBrd->tiles[i] == NULL) {
      printf("malloc failed\n");
      exit(1);
    }
    for (int j = 0; j < cols; j++) {
      pBrd->tiles[i][j].v = 0;
    }
  }
  generateBoard(pBrd, seed);
  markConnectedFromSource(pBrd);
  return pBrd;
}

void boardFree(Board *brd) {
  for (int i = 0; i < brd->rows; i++) {
    free(brd->tiles[i]);
  }
  free(brd->tiles);
  free(brd);
}
