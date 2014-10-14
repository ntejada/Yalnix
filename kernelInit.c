#include <stdio.h>
#include <stdlib.h> 
#include "trapHandlers.h"
#include "common.h"
#include "./include/hardware.h"

/*********GLOBALS*********/
// Global pointer to frame list which resides in kernel heap
int *frame_list;
// Initialization of vector table
void (*vectorTable[TRAP_VECTOR_SIZE]) (UserContext *uctxt);
//flag for whether virtual memory is on
int vmem_on = 0;
void *kernel_extent;
void *kernel_data_start;
/************************/

/*********PROTOTYPES*********/
int vectorTableInit();
int availableFramesListInit();
void pageTableInit();
void kernelStart(char * cmd_args[], unsigned int pmem_size); //add UserContext *uctxt
void DoIdle(void);
void pageTableInit();
void SetKernelData(void *_KernelDataStart, void *_KernelDataEnd);
/***************************/


void 
SetKernelData(void *_KernelDataStart, void *_KernelDataEnd)
{
	kernel_data_start = _KernelDataStart;
	kernel_extent = _KernelDataEnd;
}

void 
KernelStart(char * cmd_args[], unsigned int pmem_size)
{
	//loop through cmd_args
	//
	//copy *uctxt for use in init proc
	
	vectorTableInit();
	availableFramesListInit();
	pageTableInit();
	WriteRegister(REG_VM_ENABLE, 1);
	vmem_on = 1;
}

//TEST IF ERROR WORKS
int 
SetKernelBrk(void *addr) 
{
	if(vmem_on){
	        pte* ptbr0 = (int) ReadRegister(REG_PTBR0);
		int ptlr0 = (int) ReadRegister(REG_PTLR0);
		
		//need to check if addr is outside of region
		if(addr>KERNEL_STACK_LIMIT || addr<kernel_data_start){
			TracePrintf(1, "addr for SetKernelBrk is above stack limit or below the heap\n");	
			return ERROR;
		}		

		//map pages before addr if they aren't mapped already
		for(int i = kernel_extent>>PAGESHIFT; i < (addr>>PAGESHIFT); i++) {
			if(0==ptbr0[i].valid){
				ptbr0[i].pfn=getNextFrame();		
				if(ERROR == ptbr0[i].pfn){
					TracePrintf(1, "not enough frames left for SetKernelBrk\n");	
					return ERROR;
				}	
				ptbr0[i].valid=1;
				ptbr0[i].prot=PROT_READ|PROT_WRITE;
			}
		}
		//check if page addr lives in data below it and add it to the page table if so
		if(addr & PAGEOFFSET > 0) {
			int addrPage = addr>>PAGESHIFT;
			ptbr0[addrPage].pfn=getNextFrame();
			if(ERROR == ptbr0[addrPage].pfn){
				TracePrintf(1, "not enough frame left for SetKernelBrk\n");	
				return ERROR;
			}	
			ptbr0[addrPage].valid=1;
			ptbr0[addrPage].prot=PROT_READ|PROT_WRITE;
		}
		//unmap pages after addr
		for(i = (addr>>PAGESHIFT)+1){
			if(1==ptbr0[i].valid){
				ptbr0[i].valid = 0;
				ptbr0[i].prot = PROT_NONE;
				addFrame(ptbr0[i].pfn);
				ptbr0[i].pfn = -1;
		}
		kernel_extent = addr;		
	}
	else {
		if(addr<KERNEL_STACK_LIMIT && addr>kernel_data_start){
			kernel_extent = addr;
		}
		else{
			TracePrintf(1, "addr for SetKernelBrk is above stack limit or below the heap\n");	
			return ERROR;
		}
	}
	return SUCCESS;
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
	WriteRegister(REG_VECTOR_BASE, vectorTable);
	return SUCCESS;
}

void
pageTableInit()
{
	//region zero page table
        unsigned int reg_zero_limit = (VMEM_0_LIMIT-VMEM_0_BASE)>>PAGESHIFT;
	pte* reg_zero_table = (pte*)malloc(sizeof(pte)*reg_zero_limit);
	//malloc check
	if (reg_zero_table == NULL) {
		TracePrintf(1, "Malloc error, pageTableInit\n");
		return;
	}

	int i;
	for(i = VMEM_0_BASE>>PAGESHIFT; i<(VMEM_0_LIMIT>>PAGESHIFT); i++){
		if(i<kernel_extent>>PAGESHIFT || i>KERNEL_STACK_BASE>>PAGESHIFT){
			reg_zero_table[i].valid = 1;
			//kernel text protections
			if(i<kernel_data_start){
				reg_zero_table[i].prot = (PROT_READ|PROT_EXEC);
			}
			//kernel heap and globals protections
			else{
				reg_zero_table[i].prot = (PROT_READ|PROT_WRITE);	
			}
			reg_zero_table[i].pfn=i;
		}
		//invalid pages in between kernel stack and heap
		else{
			reg_zero_table[i].valid = 0;
			reg_zero_table[i].prot = PROT_NONE;
			reg_zero_table[i]
		}
	}
	WriteRegister(REG_PTBR0, (unsigned int) reg_zero_table);
	WriteRegister(REG_PTLR0, reg_zero_limit);
	
	//region one page table
	unsigned int reg_one_limit = (VMEM_1_LIMIT-VMEM_1_BASE)>>PAGESHIFT;		
	pte* reg_one_table = (pte*)malloc(sizeof(pte)*reg_one_limit);
	//malloc check
	if(reg_one_table == NULL) {
		TracePrintf(1, "Malloc error, pageTableInit\n");
		return;		
	}
	
	//isn't all of region 1 invalid at this point?
	for(i=VMEM_1_BASE>>PAGESHIFT; i<(VMEM_1_LIMIT>>PAGESHIFT); i++) {
		reg_one_table[i].valid = 0;
	}
	WriteRegister(REG_PTBR1, (unsigned int) reg_one_table);
	WriteRegister(REG_PTLR1, reg_zero_limit);
}

void
DoIdle(void) {

  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}



