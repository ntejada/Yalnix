#include <stdio.h>
#include <stdlib.h> 
#include "trapHandlers.h"

int vectorTableInit();
void availableFramesListInit();
void pageTableInit();
void kernelStart(char * cmd_args[], unsigned int pmem_size); //add UserContext *uctxt

void (*vectorTable[TRAP_VECTOR_SIZE]) (UserContext uctxt);

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
