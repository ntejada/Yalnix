#include <hardware.h>
#include "resource.h"
#include "lock.h"
#include "cvar.h"
#include "pipe.h"
#include "std.h"
#include "util/queue.h"

void InitResources() {
    locks = queueNew();
    cvars = queueNew();
    pipes = queueNew();
    sems = queueNew();
}

void *GetResource(int res_id) {
    Queue *queue;
    switch (res_id % NUM_RESOURCE_TYPES) {
        case LOCK:
            queue = locks;
            break;
        case CVAR:
            queue = cvars;
            break;
        case SEM:
            queue = sems;
            break;
        case PIPE:
            queue = pipes;
            break;
    }

    if (queueIsEmpty(queue)) {
        return NULL;
    }

    List *list;
    for (list = queue->head; 
         list && ((Resource *) list->data)->id != res_id; 
         list = list->next) {
        TracePrintf(5, "GetResource: list->data->id = %d\n", ((Resource *) list->data)->id);
    }

    return list->data;
}

void DoReclaim(UserContext *context) {
    int res_id = context->regs[0];
    void *resource = GetResource(res_id);
    if (!resource) {
        context->regs[0] = ERROR;
        return;
    }

    switch (res_id % NUM_RESOURCE_TYPES) {
        case LOCK:
            context->regs[0] = ReclaimLock((Lock *) resource);
            break;
        case CVAR:
            context->regs[0] = ReclaimCvar((Cvar *) resource);
            break;
        case SEM:
            context->regs[0] = ReclaimSem((Sem *) resource);
            break;
        case PIPE:
            context->regs[0] = ReclaimPipe((Pipe *) resource);
            break;
    }
}
