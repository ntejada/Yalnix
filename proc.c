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
    int pid = 1;    
    PCB *child = (PCB *)malloc(sizeof(PCB));
    
    // If for any reason we can't fork, return ERROR to calling process.
    if (!pid) {
        context->regs[0] = ERROR;
        return;
    }

    // Return 0 to child and save and arm the child for execution.
    child->id = pid;
    context->regs[0] = 0;
    SaveState(child, context);
    Ready(child);

    // Return child's pid to parent and resume execution of the parent.
    context->regs[0] = pid;
}

void DoExec(UserContext *context) {

}

void DoExit(UserContext *context) {
    // Only store exit status if someone (the parent) exists to care.
    if (current_process->parent)
        current_process->status = context->regs[0];

    // Run through children and signal to them that the parent died.
    for(List *child = current_process->children->head; child; child = child->next)
        ((PCB *) child->data)->parent = NULL;

    // free up processes resources here
}

void DoWait(UserContext *context) {
    if (queueIsEmpty(current_process->children)) {
        context->regs[0] = ERROR;
    } else {
        if (queueIsEmpty(current_process->deadChildren)) {
    //     block();
    //     *status_ptr = ((PCB *)queueGetFirst(current->Children))->status;
    //     return childPid;
        } else {
            PCB *child = queuePop(current_process->deadChildren);
            *(int *)context->regs[0] = child->status;
            context->regs[0] = child->id;
        }
    }
}

void DoGetPid(UserContext *context) {
    context->regs[0] = current_process->id;
}

void DoBrk(UserContext *context) {
    // TODO: need to do some stuff here with MMU
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
       current_process->user_context = *context;
	if (block == NO_BLOCK) {
        	queuePush(ready_queue, current_process);
	}
        PCB *next = queuePop(ready_queue);
        TracePrintf(1, "Next Process Id: %d\n", next->id);
        WriteRegister(REG_PTBR1, (unsigned int)&(next->pageTable)); 
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

        KernelContextSwitch(MyKCS, current_process, next);
        TracePrintf(1, "Got past MyKCS\n");
        *context = current_process->user_context;
    }
}
