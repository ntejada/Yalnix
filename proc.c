/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Authors: Shuo Zheng and Brian Kim
 *
 * created October 8, 2014  szheng
 */

#include "../include/hardware.h"
#include "proc.h"
#include "switch.h"
#include "./util/list.h"

PCB *current_process;
Queue *ready_queue;

void PCB_Init(PCB *pcb) {
    pcb->children = queueNew();
    pcb->deadChildren = queueNew();
	for(int i=0; i<MAX_PT_LEN; i++){
		pcb->pageTable[i].valid=0;
	}

}

List *delay_queue;
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
}

void DoDelay(UserContext *context) {
    TracePrintf(1, "in DoDelay\n");
    int delay = context->regs[0];
    delay_queue = listAppendInPlace(delay_queue, current_process);
    LoadNextProc(context);
}

void DoBlock(UserContext *context) {
    // Move current to blocked queue
}

void LoadNextProc(UserContext *context) {
    delay_queue = DelayPop(delay_queue);
    if (!queueIsEmpty(ready_queue)) {
        current_process->user_context = *context;
        queuePush(ready_queue, current_process);
        PCB *next = queuePop(ready_queue);
        TracePrintf(1, "Next Process Id: %d\n", next->id);
        WriteRegister(REG_PTBR1, (unsigned int)&(next->pageTable)); 
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);

        KernelContextSwitch(MyKCS, current_process, next);
        TracePrintf(1, "Got past MyKCS\n");
        *context = current_process->user_context;
    }
}
