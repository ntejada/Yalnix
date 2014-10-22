/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Authors: Shuo Zheng and Brian Kim
 *
 * created October 8, 2014  szheng
 */

#include "../include/hardware.h"
#include "proc.h"
#include "common.h"

PCB *current_process;
Queue *ready_queue;

void PCB_Init(PCB *pcb) {
    pcb->children = queueNew();
    pcb->deadChildren = queueNew();
}

void RestoreState(PCB *proc, UserContext *context) {
    *context = proc->context;
}

void SaveState(PCB *proc, UserContext *context) {
    proc->context = *context;
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
    if (current->parent)
        current->status = context->regs[0];

    // Run through children and signal to them that the parent died.
    for(List *child = current->children->head; child; child = child->next)
        ((PCB *) child->data)->parent = NULL;

    // free up processes resources here
}

void DoWait(UserContext *context) {
    if (queueIsEmpty(current->children)) {
        context->regs[0] = ERROR;
    } else {
        if (queueIsEmpty(current->deadChildren)) {
    //     block();
    //     *status_ptr = ((PCB *)queueGetFirst(current->Children))->status;
    //     return childPid;
        } else {
            PCB *child = queuePop(current->deadChildren);
            *(int *)context->regs[0] = child->status;
            context->regs[0] = child->id;
        }
    }
}

void DoGetPid(UserContext *context) {
    context->regs[0] = current->id;
}

void DoBrk(UserContext *context) {
    // TODO: need to do some stuff here with MMU
}

void DoDelay(UserContext *context) {
}

void DoBlock(UserContext *context) {
    // Move current to blocked queue
}
