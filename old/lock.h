/*
 * Mutex locks for Yalnix
 *
 * Author: szheng
 * created 10/10/14
 */

#ifndef _lock_h
#define _lock_h

#define LOCK 1

typedef struct lock_t Lock;

struct lock_t {
    int id;     /* Lock identifier */
    int owner;  /* PID of the process that currently owns this lock */
    Queue *lock_queue;
};

extern void doLockInit(UserContext *);
extern void doLockAcquire(UserContext *);
extern void doLockRelease(UserContext *);
extern Lock * getLock(int);
extern int destroyLock(int);

#endif /*!_lock_h*/
