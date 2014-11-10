#ifndef _pipe_h
#define _pipe_h

#include <hardware.h>
#include "util/queue.h"

extern unsigned int pipe_count;
extern Queue *pipes;

typedef struct pipe_t {
} Pipe;
    
extern void DoPipeInit(UserContext *);
extern void DoPipeRead(UserContext *);
extern void DoPipeWrite(UserContext *);
extern int ReclaimPipe(Pipe *);

#endif
