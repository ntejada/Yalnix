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
#define NEW 12


#define BLOCK 1
#define NO_BLOCK 0

typedef struct pcb_t PCB;
typedef struct zcb_t ZCB;


struct pcb_t {
	/* Process identification data */
	int id;             /* Process identifier */
	int status;         /* Running, blocked, or exit status */
	PCB *parent;        /* Pointer to the parent's PCB */
  int clock_count;
	/* Process's page table */
	struct pte pageTable[MAX_PT_LEN];
    
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
};

extern unsigned int pidCount;

extern PCB *current_process;
extern Queue *ready_queue;
extern Queue *delay_queue;
extern Queue *wait_queue; // Need a remove in place function


/* Prototypes of process helper functions */
extern void PCB_Init(PCB *);
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
extern void LoadNextProc(UserContext *, int);

#endif /*!_proc_h*/
