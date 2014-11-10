#ifndef _cvar_h
#define _cvar_h

#include "util/queue.h"
#include "util/list.h"

extern unsigned int cvar_count;
extern Queue *cvar_queue;

typedef struct cvar_t {
    int id;
    Queue *waiting;
} Cvar; 

// Everything waiting on a cvar needs to have a corresponding lock.
// Need a way to keep track of this info.
typedef struct cv_helper_t {
    int lock_id;
    PCB *pcb;
} CV_Helper;


extern void DoCvarInit(UserContext *);
extern void DoCvarSignal(UserContext *);
extern void DoCvarBroadcast(UserContext *);
extern void DoCvarWait(UserContext *);
extern int CvarOntoLockQueue(CV_Helper *);

#endif

