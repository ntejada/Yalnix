/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Authors: Shuo Zheng, Garrett Watamull, Nic Tejada
 *
 * created October 8, 2014  szheng
 */

#include "../include/hardware.h"
#include "../include/yalnix.h"
#include "proc.h"
#include "trap.h"
#include "switch.h"

void *trapVector[TRAP_VECTOR_SIZE];

void InitTrapVector() {
    // Initialize everything in the trap vector to null.
    for (int i = 0; i < TRAP_VECTOR_SIZE; i++)
        trapVector[i] = &InvalidTrapHandler;

    trapVector[TRAP_KERNEL] = KernelCallHandler;
    trapVector[TRAP_CLOCK] = ClockHandler;
    trapVector[TRAP_ILLEGAL] = IllegalHandler;
    trapVector[TRAP_MEMORY] = MemoryHandler;
    trapVector[TRAP_MATH] = MathHandler;
    trapVector[TRAP_TTY_RECEIVE] = TtyReceiveHandler;
    trapVector[TRAP_TTY_TRANSMIT] = TtyTransmitHandler;
    trapVector[TRAP_DISK] = DiskHandler;

    WriteRegister(REG_VECTOR_BASE, (unsigned int) &trapVector);
}

void KernelCallHandler(UserContext *context) {
    int rc;
    switch (context->code) {
    case YALNIX_FORK:
        DoFork(context);
        break;
    case YALNIX_EXEC:
        DoExec(context);
        break;
    case YALNIX_EXIT:
        DoExit(context);
        break;
    case YALNIX_WAIT:
        DoWait(context);
        break;
    case YALNIX_GETPID:
        DoGetPid(context);
        break;
    case YALNIX_BRK:
        DoBrk(context);
        break;
    case YALNIX_DELAY:
        DoDelay(context);
        break;
	/*
    case YALNIX_TTY_READ:
        doTtyRead(context);
        break;
    case YALNIX_TTY_WRITE:
        doTtyWrite(context);
        break;
#ifdef LINUX
    case YALNIX_PIPE_INIT:
        doPipeInit(context);
        break;
    case YALNIX_PIPE_READ:
        doPipeRead(context);
        break;
    case YALNIX_PIPE_WRITE:
        doPipeWrite(context);
        break;
    case YALNIX_LOCK_INIT:
        doLockInit(context);
        break;
    case YALNIX_LOCK_ACQUIRE:
        doLockAcquire(context);
        break;
    case YALNIX_LOCK_RELEASE:
        doLockRelease(context);
        break;
    case YALNIX_CVAR_INIT:
        doCvarWait(context);
        break;
    case YALNIX_CVAR_SIGNAL:
        doCvarSignal(context);
        break;
    case YALNIX_CVAR_BROADCAST:
        doCvarBroadcast(context);
        break;
    case YALNIX_CVAR_WAIT:
        doCvarWait(context);
        break;
    case YALNIX_RECLAIM:
        doReclaim(context);
        break;
#endif // LINUX
	*/
    }
}

void ClockHandler(UserContext *context) {
    TracePrintf(2, "In the ClockHandler\n");

   // delay_queue = listDelayUpdate(delay_queue);

    
    if (!queueIsEmpty(ready_queue)) {
        queuePush(ready_queue, current_process);
        PCB *next = queuePop(ready_queue);
        KernelContextSwitch(MyKCS, current_process, next);
        WriteRegister(REG_PTBR1, (unsigned int)&(next->pageTable)); 
        WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
        current_process = next;
        *context = next->user_context;
    }
}

void IllegalHandler(UserContext *context) {
    switch (context->code) {
    case YALNIX_MAPERR:
        // Segfault!!!??
        break;
    case YALNIX_ACCERR:
        // 
        break;
    }
}

void MathHandler(UserContext *context) {
    // segfault
    // is there no way to tell which math error it is?
}

void MemoryHandler(UserContext *context) {
    // check if violating address is within process stack
    // if so, allocate memory
}

void TtyReceiveHandler(UserContext *context) {
    // Pass into a buffer that holds input
}

void TtyTransmitHandler(UserContext *context) {
    // Write out to Tty
}

void DiskHandler(UserContext *context) {
}

void InvalidTrapHandler(UserContext *context) {
    context->regs[0] = ERROR;
    TracePrintf(1, "Invalid trap occurred from process %d with trap code %d\n",
                current_process->id, context->vector);
}
