#ifndef _sem_h
#define _sem_h

#include <hardware.h>
#include "proc.h"
#include "util/queue.h"

extern unsigned int sem_count;
extern Queue *sems;

typedef struct sem_t {
    int id;
    int max;
    int count;
    Queue *owners;
    Queue *waiting;
} Sem; 

extern void DoSemInit(UserContext *);
extern void DoSemUp(UserContext *);
extern void DoSemDown(UserContext *);
extern int ReclaimSem(Sem *);

#endif

