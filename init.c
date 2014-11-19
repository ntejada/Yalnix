#include "init.h"
#include "switch.h"
#include "proc.h"
#include "frames.h"
#include "load.h"
#include "tty.h"
#include "resource.h"

struct pte pZeroTable[MAX_PT_LEN];
int vmem_on;
void *kernel_extent;
void *kernel_data_start;
struct pte *pOneTable;

void SetKernelData(void *_KernelDataStart, void *_KernelDataEnd)
{
    kernel_data_start = _KernelDataStart;
    kernel_extent = _KernelDataEnd;
}

void KernelStart(char * cmd_args[], unsigned int pmem_size, UserContext *uctxt)
{

    InitTrapVector();
    InitTTY();
    availableFramesListInit(pmem_size);
    //
    // Set up idlePCB                                                                                                 
    PCB *idlePCB = (PCB*) malloc(sizeof(PCB));
    memset(idlePCB, 0, sizeof(PCB));
    PCB_Init(idlePCB);
    idlePCB->status = RUNNING;
    idlePCB->id = pidCount++;
    current_process = idlePCB;

    

    PageTableInit(idlePCB);
    idlePCB->kStackPages[0] = pZeroTable[KERNEL_STACK_BASE>>PAGESHIFT].pfn;
    idlePCB->kStackPages[1] =  pZeroTable[(KERNEL_STACK_BASE>>PAGESHIFT)+1].pfn;
    // Enable virtual memory.                                                                                         
    WriteRegister(REG_VM_ENABLE, 1);
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
    vmem_on = 1;

    // Initialize Queues                                                                                             
    TracePrintf(1, "Initialize queues\n");
    process_queue = queueNew();
    ready_queue = queueNew();
    delay_queue = queueNew();
    wait_queue = queueNew();
    InitResources();
    
    // Set up initPCB
    PCB *initPCB = (PCB*)malloc(sizeof(PCB));
    memset(initPCB, 0, sizeof(PCB));
    PCB_Init(initPCB);
    initPCB->status = RUNNING;
    initPCB->id = pidCount++;
    initPCB->parent = idlePCB;
    queuePush(idlePCB->children, initPCB);
    queuePush(ready_queue, initPCB);
    TracePrintf(1, "Init Process Id: %d\n", initPCB->id);
    TracePrintf(1, "pidCount: %d\n", pidCount);

    // Push first two processes onto overall process queue.
    queuePush(process_queue, idlePCB);
    queuePush(process_queue, initPCB);


    char* args[3];
    args[0]="1";
    args[2]=NULL;

    // Put same kernel context into both PCB's. 
    if (current_process->id == 0) {
        KernelContextSwitch(FirstSwitch, (void *) idlePCB, (void *) initPCB);
    }
    TracePrintf(1, "Current Process Id: %d\n", current_process->id);

    int rc;
    if (current_process->id == 0) {
        args[1] = "idle";
        TracePrintf(1, "idlePCB\n");
        rc = LoadProgram("./initIdle", args, idlePCB);
        TracePrintf(1, "rc: %d, idlePCB pc: %d\n", rc, idlePCB->user_context.pc);

        *uctxt = idlePCB->user_context;

        return;
    } else {
        // Default to init if not starting process given at yalnix command line
        if (!cmd_args[0]) {
            cmd_args[0] = "./initInit";
        }
        args[1]=cmd_args[0];
        TracePrintf(1, "%s\n", cmd_args[0]);
        rc = LoadProgram(cmd_args[0], args, initPCB);
        TracePrintf(1, "rc: %d, %s pc: %d\n", rc, cmd_args[0], initPCB->user_context.pc);

        *uctxt = initPCB->user_context;

        return;
    }
}

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

int copyOnWrite(int pageNum, PCB* pcb){
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
	} 
	else {
        free(pcb->cow.refCount[pageNum]);
    	pcb->cow.refCount[pageNum] = NULL;
	}
	TracePrintf(3, "copyOnWrite: Exiting\n"); 
	return SUCCESS;
}

int SetKernelBrk(void *addr) 
{
    if (vmem_on) {
        struct pte* ptbr0 = (struct pte*) ReadRegister(REG_PTBR0);
        int ptlr0 = (int) ReadRegister(REG_PTLR0);

        //Need to check if addr is outside of region of kernel heap.
        if(((int)addr) > PF_COPIER << PAGESHIFT || addr < kernel_data_start){
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
        for (i; i<PF_COPIER; i++) {
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
    } else {
        if ((int)addr < KERNEL_STACK_LIMIT && addr > kernel_data_start) {
            kernel_extent = addr;
        } else {
            TracePrintf(1, "addr for SetKernelBrk is above stack limit or below the heap\n");	
            return ERROR;
        }
    }

    TracePrintf(1, "Kernel brk point changed to %p\n", addr);
    return SUCCESS;
}


void PageTableInit(PCB * idlePCB)
{
    //Region zero page table
    unsigned int reg_zero_limit = (VMEM_0_LIMIT-VMEM_0_BASE)>>PAGESHIFT;
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
    struct pte * reg_one_table = idlePCB->cow.pageTable;


    /*
    // Map invalid pages in Region 1.	
    for(i = VMEM_0_BASE>>PAGESHIFT; i < (VMEM_0_LIMIT>>PAGESHIFT); i++) {
    // Create valid entry for idle process.
    if (i == ((VMEM_0_LIMIT>>PAGESHIFT) - 1)) {
    reg_one_table[i].valid = 1;
    reg_one_table[i].pfn = getNextFrame(); 
    reg_one_table[i].prot = (PROT_READ|PROT_WRITE);
    TracePrintf(1, "====Created valid entry in region 1. Page: %d\n", i);
    } else {
    reg_one_table[i].valid = 0;
    TracePrintf(1, "======Just created invalid page table entry in region 1. Page: %d.\n", i);
    }
    }
    */
    WriteRegister(REG_PTBR1, (unsigned int) reg_one_table);	
    WriteRegister(REG_PTLR1, reg_one_limit);
    }

    void DoIdle() {
        TracePrintf(1, "IN IDLE PROGRAM\n");
        while (1) {
            TracePrintf(1, "DoIdle\n");
            Pause();
        }
    }

