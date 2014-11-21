/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Initializing the kernel from boot up
 *
 * Authors: Nic Tejada, Garrett Watumull, Shuo Zheng
 *
 * created October 20, 2014
 */

#ifndef _init_h
#define _init_h

#include <hardware.h>
#include <yalnix.h>
#include "util/queue.h"
#include "std.h"
#include "trap.h"
#include "proc.h"

extern int vmem_on;

extern void *kernel_extent;
extern void *kernel_data_start;

extern struct pte pZeroTable[MAX_PT_LEN];
extern struct pte *pOneTable;

extern void SetKernelData(void *, void *);
extern void KernelStart(char * cmd_args[], unsigned int pmem_size, UserContext *uctxt); 
extern int SetKernelBrk(void *);
extern void PageTableInit(PCB*);

#endif
