#include "queue.h"
#include <stdlib.h>

typedef struct Node {
  void *data;
  struct Node *next;
} Node;

struct Queue {
  Node *head;
  Node *tail;
  int len;
};

int queueLen(Queue *q) { return q->len; }

Queue *queueMake() {
  Queue *q = malloc(sizeof(Queue));
  q->tail = NULL;
  q->head = NULL;
  q->len = 0;
  return q;
}

void queueOffer(Queue *q, void *val) {
  Node *new = malloc(sizeof(Node));
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
  free(head);
  return d;
}

void queueFree(Queue *q) {
  Node *head = q->head;
  while (head != NULL) {
    free(head->data);
    Node *tmp = head->next;
    free(head);
    head = tmp;
  }
  free(q);
}
