#ifndef SQUEUE_H
#define SQUEUE_H

typedef struct SQueue {
  int len;
  int cap;
  int head;
  int tail;
  void **data;
} SQueue;

void sQueueInit(SQueue *sq, void **storage, int cap);

int len(SQueue *sq);

void sQueueOffer(SQueue *sq, void *data);

void *sQueuePoll(SQueue *sq);

#endif // !SQUEUE_H
