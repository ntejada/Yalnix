#include "resource.h"
#include "pipe.h"
#include "proc.h"
#include "std.h"
#include "input.h"

Queue *pipes;
unsigned int pipe_count = 0;

void DoPipeInit(UserContext *context) {

    if (BufferCheck(context->regs[0], INT_LENGTH) == ERROR || BufferWriteCheck(context->regs[0], INT_LENGTH) == ERROR) {
        TracePrintf(1, "DoPipeInit: Pointer given not valid. Returning Error\n");
        context->regs[0] = ERROR;
        return;
    }

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

    pipe->bufs = queueNew();
    if (pipe->bufs == NULL) {
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


    if (BufferCheck(buf, len) == ERROR || BufferWriteCheck(buf, len) == ERROR) {
	TracePrintf(1, "DoPipeRead: buffer given not valid. Returning with an error. SEAN SMITH!\n");
	context->regs[0] = ERROR;
	return;
    }

    while (!pipe->len) {
        queuePush(pipe->waiting, current_process);
        LoadNextProc(context, BLOCK);
    }

    PipeBuffer *pipe_buf = (PipeBuffer *)pipe->bufs->head->data;

    int read, remain = pipe_buf->len - (pipe->base - pipe_buf->buf);
    for (read = 0; read < len && pipe->len > 0; read++, remain--, pipe->len--, pipe->base++, buf++) {
        if (remain <= 0) {
            free(pipe_buf->buf);
            free(queuePop(pipe->bufs));
            pipe_buf = pipe->bufs->head->data; 
            pipe->base = pipe_buf->buf;
            remain = pipe_buf->len;

        }
	TracePrintf(2, "DoPipeRead: len: %d, remain %d, char %u\n", pipe->len, remain, *((char *)pipe->base));
        *((char *)buf) = *((char *)pipe->base);
    }
    context->regs[0] = read;
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
    if (!len) {
        context->regs[0] = ERROR;
        return;
    }

    if (BufferCheck(buf, len) == ERROR) {
        TracePrintf(1, "DoPipeRead: buffer given not valid. Returning with an error. Hi Sean.\n");
	context->regs[0] = ERROR;
	return;
    }


    PipeBuffer *pipe_buf = malloc(sizeof(PipeBuffer));
    if (pipe_buf == NULL) {
        TracePrintf(2, "DoPipeWrite: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }
    pipe_buf->len = len;
    pipe_buf->buf = malloc(len);
    if (pipe_buf->buf == NULL) {
        TracePrintf(2, "DoPipeWrite: malloc error\n");
        context->regs[0] = ERROR;
        return;
    }

    TracePrintf(3, "DoPipeWrite: copying into buffer with len %d\n", len);
    memset(pipe_buf->buf, 0, len);
    for (int i = 0; i < len; i++) {
        ((char *)pipe_buf->buf)[i] = ((char *)buf)[i];
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
    TracePrintf(3, "DoPipeWrite: finished\n");
}

int ReclaimPipe(Pipe *pipe) {
    // Return error if some proc is blocked on waiting on this pipe
    if (!queueIsEmpty(pipe->waiting)) {
        return ERROR;
    } else {
        // Clear and free all buffers in pipe
        PipeBuffer *pipe_buf;
        while (!queueIsEmpty(pipe->bufs)) {
            pipe_buf = (queuePop(pipe->bufs));
            free(pipe_buf->buf);
            free(pipe_buf);
        }
        queueRemove(pipes, pipe);
        free(pipe->waiting);
        free(pipe);
        return SUCCESS;
    }
}

