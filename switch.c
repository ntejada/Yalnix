#include "std.h"
#include "proc.h"
#include "frames.h"
#include <hardware.h>
#include "switch.h"
#include "init.h"

KernelContext *MyKCS(KernelContext *kc_in, void *p_curr_pcb, void *p_next_pcb) 
{
    PCB *curr = (PCB *) p_curr_pcb;
    PCB *next = (PCB *) p_next_pcb;

    TracePrintf(1, "MyKCS called.\n");

    curr->kernel_context = *kc_in;

    if (curr != next) {
        for (int vpn = KERNEL_STACK_BASE >> PAGESHIFT, ki = 0; 
                vpn < DOWN_TO_PAGE(KERNEL_STACK_LIMIT) >> PAGESHIFT; 
                vpn++, ki++) {
            TracePrintf(3, "MyKCS: Loading in physical frame number %d\n", next->kStackPages[ki]);
            pZeroTable[vpn].pfn = next->kStackPages[ki];
        }

        current_process = next;

        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
        return &(next->kernel_context);
    } else {
        return kc_in;
    }
}

KernelContext *FirstSwitch(KernelContext *kc_in, void *p_curr_pcb, void *p_next_pcb)
{
    PCB *curr = (PCB *) p_curr_pcb;
    PCB *next = (PCB *) p_next_pcb;

    TracePrintf(1, "First Switch called. \n");

    curr->kernel_context = *kc_in;
    next->kernel_context = *kc_in;

    CopyStack(next);
    return kc_in;
}

KernelContext *ForkKernel(KernelContext *kc_in, void *p_curr_pcb, void *p_next_pcb) {
    PCB *curr = (PCB *) p_curr_pcb;
    PCB *child = (PCB *) p_next_pcb;

    TracePrintf(1, "ForkKernel called. \n");

    child->kernel_context = *kc_in;

    CopyStack(child);
    CopyRegion1(child);

    return kc_in;
}
