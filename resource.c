#include <hardware.h>
#include "resource.h"
#include "std.h"
#include "util/queue.h"

void *GetResource(Queue *queue, int res_id) {
    if (queueIsEmpty(queue)) {
        return NULL;
    }

    List *list;
    for (list = queue->head; 
         list && ((Resource *) list->data)->id != res_id; 
         list = list->next);

    return list->data;
}

void DoReclaim(UserContext *context) {
    int res_id = context->regs[0];
    switch (res_id % NUM_RESOURCE_TYPES) {
        case LOCK:
            Lock *lock = (Lock *) GetResource(lock_queue, res_id);
            // do stuff
            break;
        case CVAR:
            Cvar *cvar = (Cvar *) GetResource(cvar_queue, res_id);
            // do stuff
            break;
        case PIPE:
            Pipe *pipe = (Pipe *) GetResource(pipe_queue, res_id);
            // do stuff
            break;
    }
}
