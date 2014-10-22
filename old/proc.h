/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Process control structures - PCB, running process, ready and blocked queues
 *
 * Authors: Shuo Zheng
 *
 * created October 8, 2014  szheng
 */

#ifndef _proc_h
#define _proc_h

#include "../include/yalnix.h"
#include "../include/hardware.h"
#include "util/list.h"
#include "util/queue.h"

typedef struct pcb_t PCB;

struct pcb_t {
    /* Process identification data */
    int id;             /* Process identifier */
    int status;         /* Exit status */
    PCB *parent;        /* Pointer to the parent's PCB */

    /* Process's page table */
    struct pte *pageTable;
    /* Kernel stack pages associated with this process */
    struct pte kStackPages[KERNEL_STACK_MAXSIZE >> PAGESHIFT];

    /* Process control data */
    Queue *children;     /* List of children */ 
    Queue *deadChildren; /* List of zombie/defunct children */

    /* Processor state data */
    UserContext context;
    KernelContext kcontext;
};

extern PCB *current;   /* Currently running process */
extern Queue *ready;   /* Read to run */

// Will probably need multiple blocked queues for disk and tty?
extern Queue *blocked; /* Blocked for whatever reason */

/* Prototypes of process helper functions */
extern void RestoreState(PCB *, UserContext *);
extern void SaveState(PCB *, UserContext *);
extern void Ready(PCB *);

/* Prototypes of process do-functions */
extern void DoFork(UserContext *);
extern void DoExec(UserContext *);
extern void DoExit(UserContext *);
extern void DoWait(UserContext *);
extern void DoGetPid(UserContext *);
extern void DoBrk(UserContext *);
extern void DoDelay(UserContext *);

#endif /*!_proc_h*/