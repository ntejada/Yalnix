#ifndef _queue_h
#define _queue_h

#include "list.h"

typedef struct _Queue Queue;

struct _Queue {
    List *head;
    List *tail;
    int  length;
};

extern Queue * queueNew();
extern void queuePush(Queue *queue, void *data);
extern void * queuePop(Queue *queue);
extern int queueIsEmpty(Queue *queue);
extern void * queueRemove(Queue *queue, void *data);

#endif /*!_queue_h*/
