#include "common.h"
#include "./include/hardware.h"
//#include "./kernelInit.h"

extern unsigned int pidCount;

KernelContext *MyKCS(KernelContext *kc_in, void *curr_pid, void *next_pid) 
{
  if (curr_pcb_p == next_pcb_p) {
    TracePrintf(1, "MyKCS called without switching processes. Will now return current KernelContext\n");
    curr_pcb_p->kernel_context = *kc_in;
    return kc_in;
  }
  TracePrintf(1, "MyKCS called. Will switch processes and reassign page table entries\n");

  curr_pcb_p->kernel_context = *kc_in;

  // Change kernel stack entries
  // Index into PCB-stack location dictionary
  reg_zero_table[(KERNEL_STACK_BASE>>PAGESIZE)] = next_pcb_p->kernel_stack_1;
  reg_zero_table[(KERNEL_STACK_BASE>>PAGESIZE)+1] = next_pcb_p->kernel_stack_2;
	     		 
  reg_one_table = next_pcb_p->ptable_bp; // Reassign Region 1 page table

  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

  return &(next_pcb_p->kernel_context);

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
