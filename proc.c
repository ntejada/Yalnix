/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Authors: Shuo Zheng, Garrett Watamull, Nic Tejada
 *
 * created October 8, 2014  szheng
 */

#include <hardware.h>
#include "proc.h"
#include "delay.h"
#include "switch.h"
#include "./util/list.h"
#include "std.h"
#include "frames.h"
PCB *current_process;
Queue *ready_queue;
Queue *delay_queue;
Queue *wait_queue;

void PCB_Init(PCB *pcb) {
    pcb->children = queueNew();
    pcb->deadChildren = queueNew();
    for(int i=0; i<MAX_PT_LEN; i++){
        pcb->pageTable[i].valid=0;
    }

}

unsigned int pidCount = 0;

void RestoreState(PCB *proc, UserContext *context) {
    *context = proc->user_context;
}

void SaveState(PCB *proc, UserContext *context) {
    proc->user_context = *context;
}

void Ready(PCB *proc) {
    queuePush(ready_queue, proc);
}

void DoFork(UserContext *context) {
    // Get an available process id.
    int pid = pidCount++;    
    PCB *child = (PCB *)malloc(sizeof(PCB));

    // If for any reason we can't fork, return ERROR to calling process.
    if (!pid || !child) {
        context->regs[0] = ERROR;
        return;
    }

    PCB_Init(child);
    child->id = pid;
    child->parent = current_process;
    queuePush(child->parent->children, child);
    child->status = RUNNING;
    
    // Return 0 to child and arm the child for execution.
    child->user_context = *context;
    child->user_context.regs[0] = 0;
    queuePush(ready_queue, child);
    KernelContextSwitch(ForkKernel, current_process, child);


    // Return child's pid to parent and resume execution of the parent.
    context->regs[0] = pid;
}

void DoExec(UserContext *context) {
    current_process->user_context = *context;
    int rc = LoadProgram(context->regs[0], context->regs[1], current_process);
    if (rc != SUCCESS) {
        current_process->user_context.regs[0] = ERROR;
    }
    *context = current_process->user_context;
}

void DoExit(UserContext *context) {
    if (current_process->parent) {
        current_process->status = context->regs[0];
    }

    TracePrintf(2, "DoExit\n");

    KillProc(current_process);

    TracePrintf(2, "Finished the murder\n");
    LoadNextProc(context, NO_BLOCK);
}

void DoWait(UserContext *context) {
    if (queueIsEmpty(current_process->children) && queueIsEmpty(current_process->deadChildren)) {
        context->regs[0] = ERROR;
    } else {
        if (queueIsEmpty(current_process->deadChildren)) {
            current_process->status = WAITING;
            queuePush(wait_queue, current_process);
            LoadNextProc(context, BLOCK);
        }

        ZCB *child = queuePop(current_process->deadChildren);
        *((int *)context->regs[0]) = child->status;
        context->regs[0] = child->id;
    }
}

void DoGetPid(UserContext *context) {
    context->regs[0] = current_process->id;
}

void DoBrk(UserContext *context) {
<<<<<<< HEAD
    void * addr = (void*)context->regs[0];
    int newPageBrk = (UP_TO_PAGE(addr-VMEM_1_BASE)>>PAGESHIFT);
    int spPage = (DOWN_TO_PAGE((context->sp)-VMEM_1_BASE)>>PAGESHIFT);
    TracePrintf(2, "DoBrk: Called with brk addr %p, page %d\n", addr, newPageBrk);
    if(newPageBrk>=spPage){
        TracePrintf(1, "User Brk error: addr %p is above the stack\n", addr);
        context->regs[0]=ERROR;
        return;
    }
    TracePrintf(1, "DoBrk: sp page is set at %d\n", spPage); 
    for(int i = 0; i<spPage; i++){
        //map pages before new brk
        if(i<newPageBrk && current_process->pageTable[i].valid==0){
            current_process->pageTable[i].valid=1;
            current_process->pageTable[i].pfn = getNextFrame();
            current_process->pageTable[i].prot = (PROT_READ | PROT_WRITE);
            TracePrintf(2, "===============DoBrk: page %d mapped with pfn %d and read and write prots\n",\
                    i, current_process->pageTable[i].pfn);
        }
        //unmap pages after new brk
        if(i>=newPageBrk && (current_process->pageTable[i].valid == 1)){
            TracePrintf(2, "===============DoBrk: page %d was valid with pfn %d\n", i, current_process->pageTable[i].pfn); 
            current_process->pageTable[i].valid=0;
            if(ERROR == addFrame(current_process->pageTable[i].pfn)){
                TracePrintf(1, "Too Many Frames\n");
                context->regs[0]=ERROR;
                return;
            }
            current_process->pageTable[i].pfn = -1;

            TracePrintf(2, "===============DoBrk: page %d unmapped\n", i);
        }
    }
    context->regs[0]= SUCCESS;
    return;
=======
	// TODO: need to do some stuff here with MMU
	void * addr = (void*)context->regs[0];
	int newPageBrk = (UP_TO_PAGE(addr-VMEM_1_BASE)>>PAGESHIFT);
	int spPage = (DOWN_TO_PAGE((context->sp)-VMEM_1_BASE)>>PAGESHIFT) - 1; // ( - 1 to account for page in between stack and heap)

	TracePrintf(2, "DoBrk: Called with brk addr %p, page %d\n", addr, newPageBrk);
	if(newPageBrk>=spPage){
		TracePrintf(1, "User Brk error: addr %p is above allocatable region - interferes with the stack\n", addr);
		context->regs[0]=ERROR;
		return;
	}
	TracePrintf(1, "DoBrk: sp page is set at %d\n", spPage); 
	for(int i = 0; i<spPage; i++){
		//map pages before new brk
		if(i<newPageBrk && current_process->pageTable[i].valid==0){
			current_process->pageTable[i].valid=1;
			current_process->pageTable[i].pfn = getNextFrame();
			current_process->pageTable[i].prot = (PROT_READ | PROT_WRITE);
			TracePrintf(2, "===============DoBrk: page %d mapped with pfn %d and read and write prots\n",\
					i, current_process->pageTable[i].pfn);
		}
		//unmap pages after new brk
		if(i>=newPageBrk && (current_process->pageTable[i].valid == 1)){
			TracePrintf(2, "===============DoBrk: page %d was valid with pfn %d\n", i, current_process->pageTable[i].pfn); 
			current_process->pageTable[i].valid=0;
			if(ERROR == addFrame(current_process->pageTable[i].pfn)){
				TracePrintf(1, "Too Many Frames\n");
				context->regs[0]=ERROR;
				return;
			}
			current_process->pageTable[i].pfn = -1;

			TracePrintf(2, "===============DoBrk: page %d unmapped\n", i);
		}
	}
	context->regs[0]= SUCCESS;
	return;
>>>>>>> a2438aab56fdc330a00fa944409322a73209585f

}

void DoDelay(UserContext *context) {
    TracePrintf(1, "in DoDelay\n");
    current_process->clock_count = context->regs[0];
    TracePrintf(1, "Delay: %d\n", context->regs[0]);
    DelayAdd(current_process);
    LoadNextProc(context, BLOCK);
}

void LoadNextProc(UserContext *context, int block) {
    DelayPop();
    if (!queueIsEmpty(ready_queue)) {
        if (current_process)  {
            current_process->user_context = *context;
            if (block == NO_BLOCK) {
                queuePush(ready_queue, current_process);
            }
        }

        PCB *next = queuePop(ready_queue);
        TracePrintf(1, "Next Process Id: %d\n", next->id);
        WriteRegister(REG_PTBR1, (unsigned int) &(next->pageTable)); 
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

        KernelContextSwitch(MyKCS, current_process, next);
        TracePrintf(1, "Got past MyKCS\n");
        *context = current_process->user_context;
    }
}

void KillProc(PCB *pcb) {
    TracePrintf(2, "KillProc\n");

    PCB *parent = pcb->parent;
    if (parent) {
        if (parent->status == WAITING) {
            queueRemove(wait_queue, parent);
            queuePush(ready_queue, parent);
        }

        ZCB *zombie = (ZCB *) malloc(sizeof(ZCB));
        zombie->id = pcb->id;
        zombie->status = pcb->status;
        queuePush(parent->deadChildren, zombie);
    }

    TracePrintf(2, "KillProc\n");

    for(List *child = current_process->children->head; child; child = child->next)
        ((PCB *) child->data)->parent = NULL;

    TracePrintf(2, "KillProc\n");

    if (pcb->parent) {
        queueRemove(pcb->parent->children, pcb);
    }

    TracePrintf(2, "KillProc\n");

    FreePCB(pcb);
}

void FreePCB(PCB *pcb) {
    while (!queueIsEmpty(pcb->children)) {
        queuePop(pcb->children);
    }

    while(!queueIsEmpty(pcb->deadChildren)) {
        free(queuePop(pcb->deadChildren));
    }

    free(pcb->children);
    free(pcb->deadChildren);
    free(pcb);
}
