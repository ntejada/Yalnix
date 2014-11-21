#include "init.h"
#include "std.h"
#include "resource.h"
#include "proc.h"
#include "load.h"
#include "frames.h"
#include "switch.h"

int CopyStack(PCB *pcb) {
    pZeroTable[PF_COPIER].valid = 1;
    pZeroTable[PF_COPIER].prot = (PROT_READ | PROT_WRITE);
    TracePrintf(3, "CopyStack\n");
    for (int vpn = KERNEL_STACK_BASE >> PAGESHIFT, ki = 0;
	 vpn < DOWN_TO_PAGE(KERNEL_STACK_LIMIT) >> PAGESHIFT;
	 vpn++, ki++) {
        TracePrintf(4, "CopyStack: vpn: %d, kstackIndex: %d\n", vpn, ki);
        int newPfn = getNextFrame();
        pZeroTable[PF_COPIER].pfn = newPfn;
        WriteRegister(REG_TLB_FLUSH, PF_COPIER << PAGESHIFT);
        memcpy(PF_COPIER << PAGESHIFT, VMEM_0_BASE + (vpn << PAGESHIFT), PAGESIZE);
        TracePrintf(4, "CopyStack: Memcpy to %d from %d of size %d\n", PF_COPIER << PAGESHIFT, VMEM_0_BASE + (vpn << PAGESHIFT), PAGESIZE);
        TracePrintf(4, "CopyStack: Saving physical frame number %d\n", newPfn);
        pcb->kStackPages[ki] = newPfn;

    }

    for (int i = 0; i < 2; i++) {
        TracePrintf(4, "CopyStack: Saved frame number %d\n", pcb->kStackPages[i]);
    }

    pZeroTable[PF_COPIER].prot = PROT_NONE;
    pZeroTable[PF_COPIER].valid = 0;
}

int CopyRegion1(PCB *pcb)
{
    pZeroTable[PF_COPIER].valid = 1;
    pZeroTable[PF_COPIER].prot = (PROT_READ | PROT_WRITE);

    for (int vpn = 0; vpn < MAX_PT_LEN; vpn++) {
        if (current_process->cow.pageTable[vpn].valid) {
            int newPfn = getNextFrame();
            pZeroTable[PF_COPIER].pfn = newPfn;
            WriteRegister(REG_TLB_FLUSH, PF_COPIER << PAGESHIFT);
            memcpy(PF_COPIER << PAGESHIFT, VMEM_1_BASE + (vpn << PAGESHIFT), PAGESIZE);
            pcb->cow.pageTable[vpn].valid = 1;
            pcb->cow.pageTable[vpn].prot = current_process->cow.pageTable[vpn].prot;
            pcb->cow.pageTable[vpn].pfn = newPfn;
        }
    }

    pZeroTable[PF_COPIER].prot = PROT_NONE;
    pZeroTable[PF_COPIER].valid = 0;
}

int CoWRegion1(PCB *pcb) {
    TracePrintf(1, "CoWRegion1: Entering\n");
    int limit = (DOWN_TO_PAGE((int)current_process->user_context.sp)>>PAGESHIFT) - MAX_PT_LEN;
    for (int vpn = 0; vpn < limit; vpn++) {
        // Only do the following for non text frames                                                                          
        if (current_process->cow.pageTable[vpn].prot != (PROT_READ|PROT_EXEC)) {
            current_process->cow.pageTable[vpn].prot = PROT_READ;
        }
        pcb->cow.pageTable[vpn] = current_process->cow.pageTable[vpn];
        if (current_process->cow.pageTable[vpn].valid) {
            if (current_process->cow.refCount[vpn]) {
		pcb->cow.refCount[vpn] = current_process->cow.refCount[vpn];
            } else {
                pcb->cow.refCount[vpn] = current_process->cow.refCount[vpn] =
                    (int *) malloc(sizeof(int));
		*(pcb->cow.refCount[vpn]) = 1;

	    }
            (*(pcb->cow.refCount[vpn]))++;
        }
    }


    // Don't copy on write the stack. Just allocate new frames because the stack will almost certainly change                 
    int vpn = limit;
    TracePrintf(2, "CowRegion1: vpn is set to: %d\n", vpn);

    pZeroTable[PF_COPIER].valid = 1;
    pZeroTable[PF_COPIER].prot = (PROT_READ | PROT_WRITE);

    for (vpn; vpn < MAX_PT_LEN; vpn++) {
        TracePrintf(2, "CowRegion1: vpn: %d. %d.\n", vpn, MAX_PT_LEN);
        if (current_process->cow.pageTable[vpn].valid) {
            int newPfn = getNextFrame();
            TracePrintf(2, "After getNext\n");
            pZeroTable[PF_COPIER].pfn = newPfn;

            WriteRegister(REG_TLB_FLUSH, PF_COPIER << PAGESHIFT);
            memcpy(PF_COPIER << PAGESHIFT, VMEM_1_BASE + (vpn << PAGESHIFT), PAGESIZE);
            TracePrintf(2, "After memcpy\n");
            pcb->cow.pageTable[vpn].valid = 1;
            pcb->cow.pageTable[vpn].prot = current_process->cow.pageTable[vpn].prot;
            pcb->cow.pageTable[vpn].pfn = newPfn;
        }
    }

    pZeroTable[PF_COPIER].prot = PROT_NONE;
    pZeroTable[PF_COPIER].valid = 0;


    TracePrintf(1, "CoWRegion1: Exiting\n");
}

int CoW_PreserveGlobal(PCB *pcb) {

    TracePrintf(2, "CoW_PreserveGlobal: Entering\n");
    TracePrintf(2, "Cowpreserve: DataPageStart: %d. NumberDataPages: %d.\n", current_process->DataPageStart, current_process->NumberDataPages);
    for (int vpn = current_process->DataPageStart; vpn < current_process->NumberDataPages + current_process->DataPageStart; vpn++) {

        current_process->cow.pageTable[vpn].prot = (PROT_READ | PROT_WRITE);
        pcb->cow.pageTable[vpn].prot = (PROT_READ | PROT_WRITE);
        TracePrintf(2, "CoW_PreserveGlobal: Current Page: %d. Current prot: %d.\n", vpn, pcb->cow.pageTable[vpn].prot);
    }

    TracePrintf(2, "CoW_PreserveGlobal: Exiting\n");
    return SUCCESS;

}

int CopyOnWrite(int pageNum, PCB* pcb){
    // Go ahead and change permissions                                                                                            
    TracePrintf(3, "copyOnWrite: called with pcb id %d and pageNum %d and refCount %d and PC %p\n", pcb->id, pageNum, *pcb->cow.refCount[pageNum], pcb->user_context.pc);
    pcb->cow.pageTable[pageNum].prot = (PROT_READ|PROT_WRITE);

    // Check how many processes are referencing that frame                                                                    
    if (pcb->cow.refCount[pageNum] > 1) {
        // Copy over the page and decrement refCount                                                                          
        pZeroTable[PF_COPIER].valid = 1;
        pZeroTable[PF_COPIER].prot = (PROT_READ|PROT_WRITE);

        int newPfn = getNextFrame();
        pZeroTable[PF_COPIER].pfn = newPfn;
        WriteRegister(REG_TLB_FLUSH, PF_COPIER << PAGESHIFT);

        memcpy(PF_COPIER << PAGESHIFT, VMEM_1_BASE + (pageNum << PAGESHIFT), PAGESIZE);
	pcb->cow.pageTable[pageNum].pfn = newPfn;
	TracePrintf(3, "copyOnWrite: copied in page to new pfn %d\n", newPfn);
        (*(pcb->cow.refCount[pageNum]))--;
        pZeroTable[PF_COPIER].prot = PROT_NONE;
        pZeroTable[PF_COPIER].valid = 0;
        pcb->cow.refCount[pageNum]=NULL;
    } else {
        free(pcb->cow.refCount[pageNum]);
        pcb->cow.refCount[pageNum] = NULL;
    }
    TracePrintf(3, "copyOnWrite: Exiting\n");
    return SUCCESS;
}
