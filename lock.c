#include "lock.h"
#include "resource.h"
#include "std.h"
#include "proc.h"

Queue *locks;
unsigned int lock_count = 0;

void DoLockInit(UserContext *context) {
    Lock *lock = malloc(sizeof(Lock));
    if (lock == NULL) {
        TracePrintf(2, "CreateLock, malloc error\n");
        context->regs[0] = ERROR;
        return;
    }

    lock->waiting = queueNew();
    if (lock->waiting == NULL) {
        TracePrintf(2, "CreateLock: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }
    lock->cvars = 0;

    int lock_id = lock_count * NUM_RESOURCE_TYPES + LOCK;
    lock_count++;
    lock->id = lock_id;
    *((int *) context->regs[0]) = lock_id;

    queuePush(locks, lock);
    context->regs[0] = SUCCESS;
}

void DoLockAcquire(UserContext *context) {
    Lock *lock = (Lock *) GetResource(context->regs[0]);

    context->regs[0] = LockAcquire(lock);
    if (context->regs[0] == BLOCK) {
        LoadNextProc(context, BLOCK);
        context->regs[0] = SUCCESS;
    }
}

void DoLockRelease(UserContext *context) {
    Lock *lock = (Lock *) GetResource(context->regs[0]);
    context->regs[0] = LockRelease(lock);
}

int LockAcquire(Lock *lock) {
    if (lock == NULL) {
        TracePrintf(2, "Acquire: Lock not found.\n");
        return ERROR;
    } else if (lock->owner && lock->owner != current_process) {
        queuePush(lock->waiting, current_process);
        return BLOCK; 
    } else {
        lock->owner = current_process;
        return SUCCESS;
    }
}

int LockRelease(Lock *lock) {
    if (lock == NULL) {
        TracePrintf(2, "LockRelease: Lock not found.\n");
        return ERROR;
    } else if (lock->owner != current_process) {
        TracePrintf(2, "LockRelease: Current process does not hold lock it is trying to release.\n");
        return ERROR;
    } else {
        lock->owner = queuePop(lock->waiting);
        if (lock->owner) {
            queuePush(ready_queue, lock->owner);
        } 
        return SUCCESS;
    }
}
