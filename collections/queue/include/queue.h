#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue Queue;

Queue *queueMake();

void queueOffer(Queue *q, void *v);

void *queuePoll(Queue *q);

int queueLen(Queue *q);

void queueFree(Queue *q);

#endif // !QUEUE_H
