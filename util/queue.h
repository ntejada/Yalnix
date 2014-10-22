#ifndef _queue_h
#define _queue_h

#include "list.h"

typedef struct _Queue Queue;

struct _Queue {
    List *head;
    List *tail;
    int  length;
};

Queue * queueNew();
void queuePush(Queue *queue, void *data);
void * queuePop(Queue *queue);
int queueIsEmpty(Queue *queue);

#endif /*!_queue_h*/
