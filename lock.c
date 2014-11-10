#include "lock.h"
#include "std.h"
#include "proc.h"

Queue *lock_queue;
unsigned int lock_count = 0;

void DoLockInit(UserContext *context) {
    Lock *lock = malloc(sizeof(Lock));
    if (lock == NULL) {
	TracePrintf(2, "CreateLock, malloc error\n");
	context->regs[0] = ERROR;
	return;
    }

    lock->locked = queueNew();
    if (lock->locked == NULL) {
	TracePrintf(2, "CreateLock: malloc error\n");
	context->regs[0] = ERROR;
	return;
    }
    lock->cvars = queueNew();
    if (lock->cvars == NULL) {
        TracePrintf(2, "CreateLock: malloc error\n");
	context->regs[0] = ERROR;
	return;
    }

    lock_id = lock_count * 3;
    lock_count++;

    lock->id = lock_id;
    *(context->regs[0]) = lock_id;

    QueuePush(lock_queue, lock);
    context->regs[0] = SUCCESS;

}

void DoLockAcquire(UserContext *context) {
    Lock *lock = (Lock *) QueueGet(lock_queue, context->regs[0]);

    if (lock == NULL) {
        TracePrintf(2, "Acquire: Lock not found.\n");
        context->regs[0] = ERROR;
    }
    else if (lock->owner && lock->owner != current_process) {
        QueuePush(lock->locked, current_process);
        LoadNextProc(context, BLOCK);
	context->regs[0] = SUCCESS; 

    } else {
        lock->owner = current_process;
	context->regs[0] = SUCCESS;
    }
}

void DoLockRelease(UserContext *context) {
    Lock *lock = (Lock *) QueueGet(lock_queue, context->regs[0]);

    if (lock == NULL) {
        TracePrintf(2, "Acquire: Lock not found.\n");
        context->regs[0] = ERROR;
    }
    else if (lock->owner != current_process) {
	TracePrintf(2, "Release: Current process does not hold lock it is trying to release.\n");
	context->regs[0] = ERROR;
    }
    else {
	if (!QueueIsEmpty(lock->locked)) {
	    PCB *pcb = QueuePop(lock->locked);
	    QueuePush(ready_queue, pcb);
	    lock->owner = pcb;
	} else {
	    lock->owner = NULL;
	}
	context->regs[0] = SUCCESS;
    }
}

