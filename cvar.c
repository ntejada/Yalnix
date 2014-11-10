#include "cvar.h"
#include "std.h"
#include "proc.c"

Queue *cvar_queue;
unsigned int cvar_count = 0;

void DoCvarInit(UserContext *context) {
    Cvar *cvar = malloc(sizeof(Cvar));
    if (cvar == NULL) {	
	TracePrintf(2, "CvarInit: malloc error.\n");
	context->regs[0] = ERROR;
	return;
    }
    cvar->waiting = queueNew();
    if (cvar->waiting == NULL) {
        TracePrintf(2, "CvarInit: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }

    cvar_id = (cvar_count * 3) + 1;
    cvar->id = cvar_id;
    *(context->regs[0]) = cvar_id;
    cvar_count++;

    QueuePush(cvar_queue, cvar);
    context->regs[0] = SUCCESS;
}

void DoCvarSignal(UserContext *context) {
    Cvar *cvar = (Cvar *) QueueGet(cvar_queue, context->regs[0]);

    if (cvar == NULL) {
	TracePrintf(2, "DoCvarSignal: No Cvar found.\n");
	context->regs[0] = ERROR;
    }
    
    else if (!QueueIsEmpty(cvar->waiting)) {
	int rc;
	CV_Helper *cv_h = QueuePop(cvar->waiting);
	rc = CvarOntoLockQueue(cv_h);
	free(cv_h);
	if (rc == ERROR)
	    context->regs[0] = ERROR;
	else
	    context->regs[0] = SUCCESS;

    } else {
	context->regs[0] = SUCCESS; // Return silently if nothing to pop since nothing went wrong.
    }
}

// Returns if lock isn't found. Need to discuss whether this is best plan
void DoCvarBroadcast(UserContext *context) {
    Cvar *cvar = (Cvar *) QueueGet(cvar_queue, context->regs[0]);

    if (cvar == NULL) {
        TracePrintf(2, "DoCvarBroadcast: No Cvar found.\n");
        context->regs[0] = ERROR;
    }
    else {
	int rc;
	CV_Helper *cv_h;
	while (!QueueIsEmpty(cvar->waiting)) {
	    cv_h = QueuePop(cvar->waiting);
	    rc = CvarOntoLockQueue(cv_h);
	    free(cv_h);
	    if (rc == ERROR) {
		context->regs[0] = ERROR;
		return;
	    }
	}
	context->regs[0] = SUCCESS;
    }
}

void DoCvarWait(UserContext *context) {
    Cvar *cvar = (Cvar *) QueueGet(cvar_queue, context->regs[0]);

    if (cvar == NULL) {
        TracePrintf(2, "DoCvarWait: No Cvar found.\n");
        context->regs[0] = ERROR;
	return;
    }
    Lock *lock = (Lock *) QueueGet(lock_queue, context->regs[1]);

    if (lock == NULL) {
        TracePrintf(2, "DoCvarWait: Lock not found.\n");
        context->regs[0] = ERROR;
	return;
    }

    if (lock->owner != current_process) {
	TracePrintf(2, "DoCvarWait: process does not hold the lock.\n");
	context->regs[0] = ERROR;
	return;
    }
    
    CV_Helper *cv_h = malloc(sizeof(CV_Helper));
    if (cv_h == NULL) {
	TracePrintf(2, "DoCvarWait: malloc error.\n");
	context->regs[0] = ERROR;
	return;
    }

    cv_h->lock_id = lock->id;
    cv_h->pcb = current_process;

    QueuePush(cvar->waiting, cv_h);
    lock->cvars++;

    LoadNextProc(context, BLOCK);
    context->regs[0] = SUCCESS;

}



int CvarOntoLockQueue(CV_Helper *cv_h) {
    Lock *lock = (Lock *) QueueGet(lock_queue, cv_h->lock_id);

    if (lock == NULL) {
        TracePrintf(2, "PutCvarOnLockQueue: Lock not found.\n");
        return ERROR;
    }

    lock->cvars--;
    QueuePush(lock->locked, cv_h->pcb);
    return SUCCESS;
}
