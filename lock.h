#ifndef _lock_h
#define _lock_h

#include <hardware.h>
#include "proc.h"
#include "util/queue.h"

extern unsigned int lock_count;
extern Queue *locks;

typedef struct lock_t {
    int id;
    PCB *owner;
    Queue *waiting;
    int cvars;
} Lock;
    
extern void DoLockInit(UserContext *);
extern void DoLockAcquire(UserContext *);
extern void DoLockRelease(UserContext *);
extern int LockAcquire(Lock *);
extern int LockRelease(Lock *);
extern int ReclaimLock(Lock *);

#endif
