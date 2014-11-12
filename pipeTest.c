#include "std.h"

int main(int argc, char **argv) {
    TracePrintf(3, "Starting pipeTest\n");
    int pipe, rc, len;
    rc = PipeInit(&pipe);
    if (rc == ERROR) {
        exit(1);
    }
    void *buf = malloc(33);
    rc = Fork();
    if (rc = 0) {
        while (1) {
            TtyPrintf(1, "PipeReading\n");
            memset(buf, 0, 33);
            len = PipeRead(pipe, buf, 32);
            TtyPrintf(1, "PipeRead len %d\n", len);
            TtyPrintf(1, "%s", buf);
        }
    } else {
        while (1) {
            TtyPrintf(0, "=========>parent %d piping stuff to child\n", GetPid());
            //PipeWrite(pipe, "this is piped to child\n", 32);
            TtyPrintf(0, "=========>parent %d again piping stuff to child\n", GetPid());
            //PipeWrite(pipe, "this is also piped to child\n", 32);
            TtyPrintf(0, "=========>parent %d yet again piping stuff to child\n", GetPid());
            //PipeWrite(pipe, "this is again piped to child\n", 32);
        }
    }
}
