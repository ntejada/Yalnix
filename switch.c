#include "std.h"
#include "proc.h"
#include "frames.h"
#include "./include/hardware.h"
#include "switch.h"
#include "init.h"

KernelContext *MyKCS(KernelContext *kc_in, void *p_curr_pcb, void *p_next_pcb) 
{
    PCB *curr = (PCB *) p_curr_pcb;
    PCB *next = (PCB *) p_next_pcb;

    TracePrintf(1, "MyKCS called. Will switch processes and reassign page table entries\n");


    curr->kernel_context = *kc_in;

    TracePrintf(1, "status: %d\n", next->status);

    if((next->status) == NEW){
      TracePrintf(1, "next given kernel context\n");
      next->kernel_context = *kc_in;
      next->status = RUNNING;
    } 

    pZeroTable[KERNEL_STACK_BASE>>PAGESHIFT].pfn = next->kStackPages[0]; 
    pZeroTable[(KERNEL_STACK_BASE>>PAGESHIFT)+1].pfn = next->kStackPages[1];

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    return &(curr->kernel_context);
}

KernelContext *ForkKernel(KernelContext *kc_in, void *curr_pid, void *next_pid)
{
}
