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
extern void queuePush(Queue *, void *);
extern void * queuePop(Queue *);
extern int queueIsEmpty(Queue *);
extern void queueRemove(Queue *, void *);
extern int queueContains(Queue *, void *);

#endif /*!_queue_h*/
