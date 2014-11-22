#include "resource.h"
#include "sem.h"
#include "std.h"
#include "proc.h"

Queue *sems;
unsigned int sem_count = 0;

void DoSemInit(UserContext *context) {
    TracePrintf(2, "DoSemInit\n");
    Sem *sem = malloc(sizeof(Sem));
    if (sem == NULL) {	
        TracePrintf(2, "DoSemInit: malloc error.\n");
        context->regs[0] = ERROR;
        return;
    }

    sem->owners = queueNew();
    if (sem->owners == NULL) {
        TracePrintf(2, "DoSemInit: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }

    sem->waiting = queueNew();
    if (sem->waiting == NULL) {
        TracePrintf(2, "DoSemInit: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }

    int sem_id = sem_count * NUM_RESOURCE_TYPES + SEM;
    sem_count++;
    sem->id = sem_id;
    *((int *) context->regs[0]) = sem_id;
    sem->count = sem->max = context->regs[1];

    TracePrintf(2, "DoSemInit: pushing onto sems\n");
    queuePush(sems, sem);
    context->regs[0] = SUCCESS;
}

void DoSemUp(UserContext *context) {
    TracePrintf(2, "DoSemUp\n");
    Sem *sem = (Sem *) GetResource(context->regs[0]);
    if (sem == NULL) {
        TracePrintf(2, "DoSemUp: No Sem found.\n");
        context->regs[0] = ERROR;
        return;
    }

    if (!queueContains(sem->owners, current_process)) {
        TracePrintf(5, "DoSemUp: Current process tried to release semaphore that was never requested.\n");
        context->regs[0] = ERROR;
        return;
    }

    // If count is negative, wake up a process waiting on semaphore
    if (sem->count < 0) {
        queuePush(ready_queue, queuePop(sem->waiting));
    }
    sem->count++;
    queueRemove(sem->owners, current_process);

    if (sem->count > sem->max) {
        // Should NEVER get here.
        TracePrintf(5, "DoSemUp: Should never have gotten here, count greater than what semaphore was initiated with\n");
        context->regs[0] = ERROR;
    }

    context->regs[0] = SUCCESS;
}

void DoSemDown(UserContext *context) {
    TracePrintf(2, "DoSemDown\n");
    Sem *sem = (Sem *) GetResource(context->regs[0]);
    if (sem == NULL) {
        TracePrintf(2, "DoSemDown: No Sem found.\n");
        context->regs[0] = ERROR;
        return;
    }

    if (queueContains(sem->owners, current_process)) {
        TracePrintf(5, "DoSemDown: Current process tried to request semaphore that it already is holding on to.\n");
        context->regs[0] = ERROR;
        return;
    }

    sem->count--;
    if (sem->count < 0) {
        queuePush(sem->waiting, current_process);
        LoadNextProc(context, BLOCK);
    }
    queuePush(sem->owners, current_process);

    context->regs[0] = SUCCESS;
}

int ReclaimSem(Sem *sem) {
    // Return error if some proc is blocked on waiting on this sem or there are owners
    if (sem->count != sem->max) {
        return ERROR;
    } else {
        queueRemove(sems, sem);
        free(sem->owners);
        free(sem->waiting);
        free(sem);
        return SUCCESS;
    }
}

