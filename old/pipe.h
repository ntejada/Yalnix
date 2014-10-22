/*
 * Interprocess pipes for Yalnix
 *
 * Author: szheng
 * created 10/13/14
 */

#ifndef _pipe_h
#define _pipe_h

#define PIPE 0

typedef struct pipe_t Pipe;

struct pipe_t {
    int id;     /* Pipe identifier */
    void *buf;  /* Buffer to hold contents */
    int len_buf;
    Queue *pipe_queue;
};

extern void doPipeInit(UserContext *);
extern void doPipeRead(UserContext *);
extern void doPipeWrite(UserContext *);
extern Pipe * getPipe(int);
extern int destroyPipe(int);

#endif /*!_pipe_h*/
