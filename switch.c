#include "std.h"
#include "proc.h"
#include "./include/hardware.h"
//#include "./kernelInit.h"

extern unsigned int pidCount;

KernelContext *MyKCS(KernelContext *kc_in, void *p_curr_pid, void *p_next_pid) 
{
  if (curr_pcb_p == next_pcb_p) {
	
	
    TracePrintf(1, "MyKCS called without switching processes. Will now return current KernelContext\n");
    curr_pcb_p->kernel_context = *kc_in;
    return kc_in;
  }
  TracePrintf(1, "MyKCS called. Will switch processes and reassign page table entries\n");

	int curr_pid = (int) p_curr_pid;
	int next_pid = (int) p_next_pid;

	pZeroTable[KERNEL_STACK_BASE>>PAGESHIFT] = getNextFrame(); pZeroTable[(KERNEL_STACK_BASE>>PAGESHIFT)+1]= getNextFrame();
}

KernelContext *CreateProcess(KernelContext *kc_in, void *curr_pid, void *next_pid)
{
    int k_stack_0 = getNextFrame();
    int k_stack_1 = getNextFrame();
    reg_zero_table[(KERNEL_STACK_BASE >> PAGESIZE)] = k_stack_0;
    reg_zero_table[(KERNEL_STACK_BASE >> PAGESIZE) + 1] = k_stack_1;
    
    PCB *pcb = (PCB *) KERNEL_STACK_LIMIT - 1;
    pcb->pid = pidCount++;
    pcb->kernel
}

KernelContext *ForkKernel(KernelContext *kc_in, void *curr_pid, void *next_pid)
{
}
