#include "cvar.h"
#include "lock.h"
#include "resource.h"
#include "std.h"
#include "proc.h"
#include "cvar.h"

Queue *cvars;
unsigned int cvar_count = 0;

void DoCvarInit(UserContext *context) {

    if (BufferCheck(context->regs[0], INT_LENGTH) == ERROR || BufferWriteCheck(context->regs[0], INT_LENGTH) == ERROR) {
        TracePrintf(1, "DoCvarInit: Pointer given not valid. Returning Error\n");
        context->regs[0] = ERROR;
        return;
    }

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

    int cvar_id = cvar_count * NUM_RESOURCE_TYPES + CVAR;
    cvar_count++;
    cvar->id = cvar_id;
    *((int *) context->regs[0]) = cvar_id;

    TracePrintf(2, "CvarInit: pushing onto cvars\n");
    queuePush(cvars, cvar);
    context->regs[0] = SUCCESS;
}

void DoCvarSignal(UserContext *context) {
    Cvar *cvar = (Cvar *) GetResource(context->regs[0]);
    if (cvar == NULL) {
        TracePrintf(2, "DoCvarWait: No Cvar found.\n");
        context->regs[0] = ERROR;
        return;
    }

    PCB *pcb = queuePop(cvar->waiting);
    if (pcb) {
        queuePush(ready_queue, pcb);
        context->regs[0] = SUCCESS;
    } else {
        context->regs[0] = ERROR;
    }
}

void DoCvarBroadcast(UserContext *context) {
    Cvar *cvar = (Cvar *) GetResource(context->regs[0]);
    if (cvar == NULL) {
        TracePrintf(2, "DoCvarWait: No Cvar found.\n");
        context->regs[0] = ERROR;
        return;
    }

    if (queueIsEmpty(cvar->waiting)) {
        context->regs[0] = ERROR;
    } else {
        while (!queueIsEmpty(cvar->waiting)) {
            queuePush(ready_queue, queuePop(cvar->waiting));
        }
        context->regs[0] = SUCCESS;
    }
}

void DoCvarWait(UserContext *context) {
    int rc;
    Cvar *cvar = (Cvar *) GetResource(context->regs[0]);
    if (cvar == NULL) {
        TracePrintf(2, "DoCvarWait: No Cvar found.\n");
        context->regs[0] = ERROR;
        return;
    }

    Lock *lock = (Lock *) GetResource(context->regs[1]);

    // Release lock first
    rc = LockRelease(lock);

    if (rc == ERROR) {
        context->regs[0] = rc;
        return;
    }

    // Block until something else wakes it up
    queuePush(cvar->waiting, current_process);
    lock->cvars++;
    LoadNextProc(context, BLOCK);

    // Reacquire the lock
    rc = LockAcquire(lock);
    if (rc == BLOCK) {
        LoadNextProc(context, BLOCK);
        rc = SUCCESS;
    }
    lock->cvars--;
    context->regs[0] = rc;
}

int ReclaimCvar(Cvar *cvar) {
    // Return error if some proc is blocked on waiting on this cvar
    if (!queueIsEmpty(cvar->waiting)) {
        return ERROR;
    } else {
        queueRemove(cvars, cvar);
        free(cvar->waiting);
        free(cvar);
        return SUCCESS;
    }
}

