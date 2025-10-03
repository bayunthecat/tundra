#include "squeue.h"
#include <stdio.h>
#include <stdlib.h>

void sQeueuInit(SQueue *sq, void **storage, int capacity) {
  sq->tail = 0;
  sq->len = 0;
  sq->cap = capacity;
  sq->data = storage;
  sq->head = 0;
}

int len(SQueue *sq) { return sq->len; };

void sQueueOffer(SQueue *sq, void *data) {
  if (sq->len == sq->cap) {
    fprintf(stderr, "queue is full");
    exit(1);
  }
  sq->data[sq->tail] = data;
  sq->tail = (sq->tail + 1) % sq->cap;
  sq->len++;
}

void *sQueuePoll(SQueue *sq) {
  void *d = sq->data[sq->head];
  sq->head = (sq->head + 1) % sq->cap;
  sq->len--;
  return d;
}
