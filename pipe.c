#include "pipe.h"

Queue *pipes;
unsigned int pipe_count = 0;

void DoPipeInit(UserContext *context) {}
void DoPipeRead(UserContext *context) {}
void DoPipeWrite(UserContext *context) {}
int ReclaimPipe(Pipe *pipe) {}

