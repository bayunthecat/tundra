#include "mem.h"
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
  Tile *tiles;
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

static inline Tile *tileAt(Board *brd, int row, int col) {
  return &brd->tiles[brd->cols * row + col];
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
  Arena *qArena = arenaMake(16 * brd->rows * brd->cols + 100);
  Queue *q = queueMake(qArena);
  Coord *start = makeCoord(brd->source.i, brd->source.j);
  queueOffer(q, start);
  int size = 10;
  int arr[size];
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
        Tile *atLeft = tileAt(brd, p->i, p->j - 1);
        if (hasRight(atLeft)) {
          connections++;
          genValue = genValue | 0b1000;
        }
      }
      if (p->i - 1 >= 0) {
        Tile *atUp = tileAt(brd, p->i - 1, p->j);
        if (hasDown(atUp)) {
          connections++;
          genValue = genValue | 0b0100;
        }
      }
      if (p->j + 1 < brd->cols) {
        Tile *atRight = tileAt(brd, p->i, p->j + 1);
        if (hasLeft(atRight)) {
          connections++;
          genValue = genValue | 0b0010;
        }
      }
      if (p->i + 1 < brd->rows) {
        Tile *atDown = tileAt(brd, p->i + 1, p->j);
        if (hasUp(atDown)) {
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
      tileAt(brd, p->i, p->j)->v = genValue;
      free(p);
    }
  }
  free2DIntArray(visited, brd->rows, brd->rows);
  free2DIntArray(reserved, brd->cols, brd->cols);
  arenaFree(qArena);
}

void resetConnected(Board *brd) {
  for (int i = 0; i < brd->rows * brd->cols; i++) {
    brd->tiles[i].connected = 0;
  }
}

void markConnected(Board *brd, int i, int j) {
  Tile *t = tileAt(brd, i, j);
  t->connected = 1;
  if (hasLeft(t) && j - 1 >= 0) {
    Tile *leftOf = tileAt(brd, i, j - 1);
    if (hasRight(leftOf) && !leftOf->connected) {
      markConnected(brd, i, j - 1);
    }
  }
  if (hasUp(t) && i - 1 >= 0) {
    Tile *upOf = tileAt(brd, i - 1, j);
    if (hasDown(upOf) && !upOf->connected) {
      markConnected(brd, i - 1, j);
    }
  }
  if (hasRight(t) && j + 1 < brd->cols) {
    Tile *rightOf = tileAt(brd, i, j + 1);
    if (hasLeft(rightOf) && !rightOf->connected) {
      markConnected(brd, i, j + 1);
    }
  }
  if (hasDown(t) && i + 1 < brd->rows) {
    Tile *downOf = tileAt(brd, i + 1, j);
    if (hasUp(downOf) && !downOf->connected) {
      markConnected(brd, i + 1, j);
    }
  }
}

void markConnectedFromSource(Board *brd) {
  resetConnected(brd);
  markConnected(brd, brd->source.i, brd->source.j);
}

void boardRotateAt(Board *brd, int row, int col) {
  shiftRight(&tileAt(brd, row, col)->v, 1);
  markConnectedFromSource(brd);
}

int boardValueAt(Board *brd, int row, int col) {
  return tileAt(brd, row, col)->v;
}

int boardConnectedAt(Board *brd, int row, int col) {
  return tileAt(brd, row, col)->connected;
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
  pBrd->tiles = malloc(sizeof(Tile *) * rows * cols);
  if (pBrd->tiles == NULL) {
    printf("malloc failed\n");
    exit(1);
  }
  for (int i = 0; i < rows * cols; i++) {
    pBrd->tiles[i].v = 0;
  }
  generateBoard(pBrd, seed);
  markConnectedFromSource(pBrd);
  return pBrd;
}

void boardFree(Board *brd) {
  free(brd->tiles);
  free(brd);
}
