#ifndef _lock_h
#define _lock_h

#include "util/queue.h"
#include "util/list.h"

extern unsigned int lock_count;
extern Queue *lock_queue;


typedef struct lock_t {
    int id;
    Queue *locked;
    int cvars;
    PCB *owner;
} Lock;
    
extern void DoLockInit(UserContext *);
extern void DoLockAcquire(UserContext *);
extern void DoLockRelease(UserContext *);

#endif
