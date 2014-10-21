#include "./include/hardware.h"
#include "./kernelInit.h"

KernelContext *MyKCS(KernelContext *kc_in, void *p_curr_pid, void *p_next_pid) 
{
  if (curr_pcb_p == next_pcb_p) {
	
	
    TracePrintf(1, "MyKCS called without switching processes. Will now return current KernelContext\n");
    curr_pcb_p->kernel_context = *kc_in;
    return kc_in;
  }
  TracePrintf("MyKCS called. Will switch processes and reassign page table entries\n");

	int curr_pid = (int) p_curr_pid;
	int next_pid = (int) p_next_pid;

	pZeroTable[KERNEL_STACK_BASE>>PAGESHIFT] = getNextFrame(); pZeroTable[(KERNEL_STACK_BASE>>PAGESHIFT)+1]= getNextFrame();
	` 	


}
