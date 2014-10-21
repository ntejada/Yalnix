#include <stdio.h>
#include <stdlib.h> 
#include "trapHandlers.h"
#include "common.h"
#include "./include/hardware.h"

/*********PROTOTYPES*********/
int vectorTableInit();
void pageTableInit(PCB* newPcb);
void kernelStart(char * cmd_args[], unsigned int pmem_size, UserContext *uctxt); //add UserContext *uctxt
void DoIdle(void);
void LoadProgramTest();
void pageTableInit();
void SetKernelData(void *_KernelDataStart, void *_KernelDataEnd);
/***************************/
int vmem_on = 0;

void 
SetKernelData(void *_KernelDataStart, void *_KernelDataEnd)
{
	kernel_data_start = _KernelDataStart;
	kernel_extent = _KernelDataEnd;
}

void 
KernelStart(char * cmd_args[], unsigned int pmem_size, UserContext *uctxt)
{

	
	vectorTableInit();
	availableFramesListInit(pmem_size);
	PCB idlePCB;
	PCB_Init(&idlePCB);
	pageTableInit(&idlePCB);
	
	// Cook things so idle process will begin running after return to userland.
	uctxt->pc = DoIdle;
	uctxt->sp = (void *)(VMEM_LIMIT-4);
	idlePCB.user_context = *uctxt;
	// Enable virtual memory.
	WriteRegister(REG_VM_ENABLE, 1);
	WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
	vmem_on = 1;
	TracePrintf(1, "PCB address: %p\n", &idlePCB);	
	TracePrintf(1, "sp: %p\n", uctxt->sp);
	return;
}


int 
SetKernelBrk(void *addr) 
{
	if (vmem_on) {
	        struct pte* ptbr0 = (struct pte*) ReadRegister(REG_PTBR0);
		int ptlr0 = (int) ReadRegister(REG_PTLR0);
		
		//Need to check if addr is outside of region of kernel heap.
		if(((int)addr) > KERNEL_STACK_LIMIT || addr < kernel_data_start){
			TracePrintf(1, "Addr for SetKernelBrk is above stack limit or below the heap\n");	
			return ERROR;
		}		

		//Map pages before addr if they aren't mapped already
		int i = DOWN_TO_PAGE(kernel_extent)>>PAGESHIFT;
		for(i; i < (DOWN_TO_PAGE(addr)>>PAGESHIFT); i++) {
		       if (0 == ptbr0[i].valid) {
				ptbr0[i].pfn = getNextFrame();		
				if (ERROR == ptbr0[i].pfn){
					TracePrintf(1, "Not enough frames left for SetKernelBrk\n");	
					return ERROR;
				}	
				ptbr0[i].valid = 1;
				ptbr0[i].prot = PROT_READ|PROT_WRITE;
			}
		}
		//check if page addr lives in data below it and add it to the page table if so
		if ((int)addr & PAGEOFFSET > 0) {
			int addrPage = DOWN_TO_PAGE(addr)>>PAGESHIFT;
			ptbr0[addrPage].pfn = getNextFrame();
			if (ERROR == ptbr0[addrPage].pfn) {
				TracePrintf(1, "Not enough frames left for SetKernelBrk\n");	
				return ERROR;
			}	
			ptbr0[addrPage].valid = 1;
			ptbr0[addrPage].prot = PROT_READ|PROT_WRITE;
		}
		//unmap pages after addr
		i = (DOWN_TO_PAGE(addr)>>PAGESHIFT);	
		for (i; i<KERNEL_STACK_BASE>>PAGESHIFT; i++) {
			if (1 == ptbr0[i].valid) {
				ptbr0[i].valid = 0;
				ptbr0[i].prot = PROT_NONE;
				if (ERROR == addFrame(ptbr0[i].pfn)) {
					TracePrintf(1, "Too many frames\n");
					return ERROR;
				}
				ptbr0[i].pfn = -1;
			}
		}
		kernel_extent = addr;		
	}
	else {
		if ((int)addr < KERNEL_STACK_LIMIT && addr > kernel_data_start) {
			kernel_extent = addr;
		}
		else {
			TracePrintf(1, "addr for SetKernelBrk is above stack limit or below the heap\n");	
			return ERROR;
		}
	}
	
	TracePrintf(1, "Kernel brk point changed to %p\n", addr);
	return SUCCESS;
}


int 
vectorTableInit()
{
	vector_table[TRAP_KERNEL] = kernelTrap;
	vector_table[TRAP_CLOCK] = clockTrap;
	vector_table[TRAP_ILLEGAL] = illegalTrap;
	vector_table[TRAP_MEMORY] = memoryTrap;
	vector_table[TRAP_MATH] = mathTrap;
	vector_table[TRAP_TTY_RECEIVE] = receiveTrap;
	vector_table[TRAP_TTY_TRANSMIT] = transmitTrap;
	vector_table[TRAP_DISK] = diskTrap;
	int i = TRAP_DISK+1;
	while(i<TRAP_VECTOR_SIZE){
		vector_table[i] = noTrap;
		i++;
	}
	WriteRegister(REG_VECTOR_BASE, (unsigned int) vector_table);
	return SUCCESS;
}

void
pageTableInit(PCB * idlePCB)
{
	//Region zero page table
        unsigned int reg_zero_limit = (VMEM_0_LIMIT-VMEM_0_BASE)>>PAGESHIFT;
	//reg_zero_table = (struct pte*)malloc(sizeof(struct pte)*reg_zero_limit);
	//malloc check
	/*if (reg_zero_table == NULL) {
		TracePrintf(1, "Malloc error, pageTableInit\n");
		return;
	}
*/
	int i;
	for(i = VMEM_0_BASE>>PAGESHIFT; i < (VMEM_0_LIMIT>>PAGESHIFT); i++){
		if (i < DOWN_TO_PAGE(kernel_extent)>>PAGESHIFT || i >= KERNEL_STACK_BASE>>PAGESHIFT) {
			pZeroTable[i].valid = 1;
			//kernel text protections
			if (i < DOWN_TO_PAGE(kernel_data_start)>>PAGESHIFT) {
				pZeroTable[i].prot = (PROT_READ|PROT_EXEC);
				TracePrintf(1, "==========Just created pte for page %d with read and exec rights\n", i);
			}
			//kernel heap and globals protections
			else {
				pZeroTable[i].prot = (PROT_READ|PROT_WRITE);
				TracePrintf(1, "==========Just created pte for page %d with read and write rights\n", i);	
			}
			pZeroTable[i].pfn = i;
		}
		//invalid pages in between kernel stack and heap		else {
		else {
			pZeroTable[i].valid = 0;
			pZeroTable[i].prot = PROT_NONE;
			pZeroTable[i];
			TracePrintf(1, "==========Just created pte for page %d. It is invalid and has no rights\n", i);
		}
	}
	WriteRegister(REG_PTBR0, (unsigned int) pZeroTable);
	WriteRegister(REG_PTLR0, reg_zero_limit);
	
	//region one page table
	unsigned int reg_one_limit = (VMEM_1_LIMIT-VMEM_1_BASE)>>PAGESHIFT;		
	//reg_one_table = (struct pte*)malloc(sizeof(struct pte)*reg_one_limit);
	//malloc check
	/*if (reg_one_table == NULL) {
		TracePrintf(1, "Malloc error, pageTableInit\n");
		return;		
	}*/
	struct pte * reg_one_table = idlePCB->reg_one_table;
	

	// Map invalid pages in Region 1.	
	for(i = VMEM_0_BASE>>PAGESHIFT; i < (VMEM_0_LIMIT>>PAGESHIFT); i++) {
	  // Create valid entry for idle process.
	  if (i == ((VMEM_0_LIMIT>>PAGESHIFT) - 1)) {
	      reg_one_table[i].valid = 1;
	      reg_one_table[i].pfn = (i+(VMEM_0_LIMIT>>PAGESHIFT));
	      reg_one_table[i].prot = (PROT_READ|PROT_WRITE);
	      TracePrintf(1, "====Created valid entry in region 1. Page: %d\n", i);
	  } else {
	    reg_one_table[i].valid = 0;
	    TracePrintf(1, "======Just created invalid page table entry in region 1. Page: %d.\n", i);
	  }
	}
	WriteRegister(REG_PTBR1, (unsigned int) reg_one_table);	WriteRegister(REG_PTLR1, reg_one_limit);
}

void
DoIdle(void) {
  TracePrintf(1, "IN IDLE PROGRAM\n");
  while (1) {
    TracePrintf(1, "DoIdle\n");
    Pause();
  }
}

void 
LoadProgramTest() {
//	LoadProgram("./initIdle", NULL, p_pcb); 

}


