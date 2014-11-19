#ifndef _cvar_h
#define _cvar_h

#include <hardware.h>
#include "proc.h"
#include "util/queue.h"

extern unsigned int cvar_count;
extern Queue *cvars;

typedef struct cvar_t {
    int id;
    Queue *waiting;
} Cvar; 

extern void DoCvarInit(UserContext *);
extern void DoCvarSignal(UserContext *);
extern void DoCvarBroadcast(UserContext *);
extern void DoCvarWait(UserContext *);
extern int ReclaimCvar(Cvar *);

#endif

