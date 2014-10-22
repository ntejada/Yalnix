#ifndef _init_h
#define _init_h

#include <stdio.h>
#include <stdlib.h>
#include "./include/hardware.h"
#include "./include/yalnix.h"
#include "queue.h"
#include "common.h"
#include "trap.h"

extern int vmem_on;

extern void *kernel_extent;
extern void *kernel_data_start;

extern struct pte pZeroTable[MAX_PT_LEN];
extern struct pte *pOneTable;

void SetKernelData(void *, void *);
void KernelStart(char * cmd_args[], unsigned int pmem_size, UserContext *uctxt); 
int SetKernelBrk(void *);
void PageTableInit(PCB*);
void DoIdle(void);
void LoadProgramTest();

#endif
