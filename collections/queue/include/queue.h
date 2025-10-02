#ifndef QUEUE_H
#define QUEUE_H

#include "mem.h"

typedef struct Queue Queue;

Queue *queueMake(Arena *a);

void queueOffer(Queue *q, void *v);

void *queuePoll(Queue *q);

int queueLen(Queue *q);

#endif // !QUEUE_H
