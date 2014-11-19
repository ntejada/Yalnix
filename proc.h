/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Process control structures - PCB, running process, ready 
 * and blocked queues for various reasons
 *
 * Authors: Shuo Zheng, Garrett Watumull, Nic Tejada
 *
 * created October 8, 2014  szheng
 */

#ifndef _proc_h
#define _proc_h

#include <yalnix.h>
#include <hardware.h>
#include "util/list.h"
#include "util/queue.h"

/*****STATES*****/

#define RUNNING 1
#define BLOCKED 2
#define WAITING 3
#define DEAD 4
#define NEW 12



#define BLOCK 1
#define NO_BLOCK 0

typedef struct pcb_t PCB;
typedef struct zcb_t ZCB;
typedef struct cow_page_table COWPageTable;

struct cow_page_table {
    struct pte pageTable[MAX_PT_LEN];
    int *refCount[MAX_PT_LEN];      // Pointers to integers representing a reference count
                                        // to corresponding page - ie. the number of processes
                                        // seeing this page as read only reference.
};

struct pcb_t {
    /* Process identification data */
    int id;             /* Process identifier */
    int status;         /* Running, blocked, or exit status */
    PCB *parent;        /* Pointer to the parent's PCB */
    int clock_count;    /* Remain time to wait after a Delay() call */
	char *readBuf;
    /* Process's page table */
    COWPageTable cow;

    /* Kernel stack pfns associated with this process */
    int kStackPages[KERNEL_STACK_MAXSIZE >> PAGESHIFT];

    /* Process control data */
    Queue *children;     /* List of children */  // PCBs
    Queue *deadChildren; /* List of zombie/defunct children */ // ZCBs

    /* Processor state data */
    UserContext user_context;
    KernelContext kernel_context;
};

struct zcb_t {
    int id;
    int status;
    int exit_status;
};

extern unsigned int pidCount; // Counter for next PID

extern PCB *current_process;
extern Queue *ready_queue;
extern Queue *delay_queue;
extern Queue *wait_queue;
extern Queue *process_queue;

/* Prototypes of process helper functions */
extern void PCB_Init(PCB *);
extern void RestoreState(PCB *, UserContext *);
extern void SaveState(PCB *, UserContext *);
extern void Ready(PCB *);

/* Prototypes of process do-functions */
extern void DoFork(UserContext *);
//extern void DoSpoon(UserContext *);
extern void DoExec(UserContext *);
extern void DoExit(UserContext *);
extern void DoWait(UserContext *);
extern void DoGetPid(UserContext *);
extern void DoBrk(UserContext *);
extern void DoPS(UserContext *);
extern void DoDelay(UserContext *);
extern void LoadNextProc(UserContext *, int);
extern void KillProc(PCB *);
extern void FreePCB(PCB *);

#endif /*!_proc_h*/
