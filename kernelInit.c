#include <stdio.h>
#include <stdlib.h> 
#include "trapHandlers.h"
#include "common.h"
#include "./include/hardware.h"


// Global pointer to frame list which resides in kernel heap
int *frame_list;

int vectorTableInit();
int *availableFramesListInit();
void pageTableInit();
void kernelStart(char * cmd_args[], unsigned int pmem_size); //add UserContext *uctxt
void DoIdle(void);


// Initialization of vector table.
void (*vectorTable[TRAP_VECTOR_SIZE]) (UserContext *uctxt);

void 
KernelStart(char * cmd_args[], unsigned int pmem_size)
{
	//loop through cmd_args
	//
	//copy *uctxt for use in init proc
	
	vectorTableInit();
	availableFramesListInit();
	pageTableInit();
}

// Store frame list in kernel heap.
int
availableFramesListInit() {
  int top_frame = (PMEM_LIMIT-PMEM_BASE) / PAGESIZE;
  int first_frame = VMEM_0_LIMIT >> PAGESHIFT;

  int number_of_frames = top_frame - first_frame;

  frame_list  = (int *)malloc(number_of_frames * sizeof(int));

  if (frame_list == NULL) {
    TracePrintf(1, "Malloc error, availableFramesListInit\n");
    return ERROR;
  }

  int i;
  for (i = 0; i < number_of_frames; i++) 
    frame_list[i] = i + first_frame; // Mark all as available

  return SUCCESS;
}




int 
vectorTableInit()
{
	vectorTable[TRAP_KERNEL] = kernelTrap;
	vectorTable[TRAP_CLOCK] = clockTrap;
	vectorTable[TRAP_ILLEGAL] = illegalTrap;
	vectorTable[TRAP_MEMORY] = memoryTrap;
	vectorTable[TRAP_MATH] = mathTrap;
	vectorTable[TRAP_TTY_RECEIVE] = receiveTrap;
	vectorTable[TRAP_TTY_TRANSMIT] = transmitTrap;
	vectorTable[TRAP_DISK] = diskTrap;
	int i = TRAP_DISK+1;
	while(i<TRAP_VECTOR_SIZE){
		vectorTable[i] = noTrap;
		i++;
	}
	return SUCCESS;
}

void
DoIdle(void) {

  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}
