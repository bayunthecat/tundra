#include "queue.h"
#include "mem.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
  void *data;
  struct Node *next;
} Node;

struct Queue {
  Arena *arena;
  Node *head;
  Node *tail;
  int len;
};

int queueLen(Queue *q) { return q->len; }

Queue *queueMake(Arena *a) {
  Queue *q = arenaAlloc(a, sizeof(Queue));
  q->tail = NULL;
  q->head = NULL;
  q->len = 0;
  q->arena = a;
  return q;
}

void queueOffer(Queue *q, void *val) {
  Node *new = arenaAlloc(q->arena, sizeof(Node));
  new->data = val;
  new->next = NULL;
  Node *tail = q->tail;
  q->len++;
  if (tail == NULL) {
    q->head = new;
    q->tail = new;
    return;
  }
  q->tail->next = new;
  q->tail = new;
}

void *queuePoll(Queue *q) {
  if (q->head == NULL) {
    return NULL;
  }
  q->len--;
  Node *head = q->head;
  void *d = head->data;
  q->head = q->head->next;
  if (q->head == NULL) {
    q->tail = NULL;
  }
  return d;
}
