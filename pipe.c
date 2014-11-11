#include "resource.h"
#include "pipe.h"
#include "proc.h"
#include "std.h"

Queue *pipes;
unsigned int pipe_count = 0;

void DoPipeInit(UserContext *context) {
    Pipe *pipe= malloc(sizeof(Pipe));
    if (pipe == NULL) {	
        TracePrintf(2, "PipeInit: malloc error.\n");
        context->regs[0] = ERROR;
        return;
    }

    pipe->waiting = queueNew();
    if (pipe->waiting == NULL) {
        TracePrintf(2, "PipeInit: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }

    int pipe_id = pipe_count * NUM_RESOURCE_TYPES + PIPE;
    pipe_count++;
    pipe->id = pipe_id;
    *((int *) context->regs[0]) = pipe_id;

    TracePrintf(2, "PipeInit: pushing onto pipes\n");
    queuePush(pipes, pipe);
    context->regs[0] = SUCCESS;
}

// Should be pretty similar to ReadBuffer from tty.c
void DoPipeRead(UserContext *context) {
    TracePrintf(2, "DoPipeRead\n");
    
    Pipe *pipe = (Pipe *) GetResource(context->regs[0]);
    if (pipe == NULL) {
        TracePrintf(2, "DoPipeRead: No Pipe found.\n");
        context->regs[0] = ERROR;
        return;
    }

    void *buf = context->regs[1];
    int len = context->regs[2];
    // TODO check integrity of buf pointer?

    while (!pipe->len) {
        queuePush(pipe->waiting, current_process);
        LoadNextProc(context, BLOCK);
    }

    PipeBuffer *pipe_buf = pipe->bufs->head->data;

    int i, remain = pipe_buf->len - (pipe->base - pipe_buf->buf);
    for (i = 0; i < len && pipe->len; i++, remain--, pipe->len--, pipe->base++) {
        if (!remain) {
            free(pipe_buf->buf);
            free(queuePop(pipe->bufs));
            pipe_buf = pipe->bufs->head->data; 
            pipe->base = pipe_buf->buf;
            remain = pipe_buf->len - (pipe->base - pipe_buf->buf);
        }
        buf[i] = pipe_buf->buf[i];
    }
    context->regs[0] = i;
}

void DoPipeWrite(UserContext *context) {
    TracePrintf(2, "DoPipeWrite\n");
    
    Pipe *pipe = (Pipe *) GetResource(context->regs[0]);
    if (pipe == NULL) {
        TracePrintf(2, "DoPipeWrite: No Pipe found.\n");
        context->regs[0] = ERROR;
        return;
    }

    void *buf = context->regs[1];
    int len = context->regs[2];
    // TODO check integrity of buf pointer?

    PipeBuffer *pipe_buf = malloc(sizeof(PipeBuffer));
    if (pipe_buf == NULL) {
        TracePrint(2, "DoPipeWrite: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }
    pipe_buf->len = len;
    pipe_buf->buf = malloc(len);
    if (pipe_buf->buf == NULL) {
        TracePrint(2, "DoPipeWrite: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }
    memset(pipe_buf->buf, 0, len);
    for (int i = 0; i < len; i++) {
        // TODO deferencing void pointer, probably need to change to char *?
        pipe_buf->buf[i] = buf[i];
    }
    
    if (!pipe->len) {
        pipe->base = pipe_buf->buf;
    }
    pipe->len += len;
    queuePush(pipe->bufs, pipe_buf);

    // Wake up/signal all waiting, same sort of concept as CvarBroadcast
    while (!queueIsEmpty(pipe->waiting)) {
        queuePush(ready_queue, queuePop(pipe->waiting));
    }

    context->regs[0] = len;
}

int ReclaimPipe(Pipe *pipe) {
    // Return error if some proc is blocked on waiting on this pipe
    if (!queueIsEmpty(pipe->waiting)) {
        return ERROR;
    } else {
        queueRemove(pipes, pipe);
        free(pipe->waiting);
        free(pipe);
        return SUCCESS;
    }
}

