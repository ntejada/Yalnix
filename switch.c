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
   

    pZeroTable[KERNEL_STACK_BASE>>PAGESHIFT] = next->kStackPages[0]; 
    pZeroTable[(KERNEL_STACK_BASE>>PAGESHIFT)+1] = next->kStackPages[1];
    
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    return &(next->kernel_context);
}

KernelContext *ForkKernel(KernelContext *kc_in, void *curr_pid, void *next_pid)
{
}
