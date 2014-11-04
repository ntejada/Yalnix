/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Authors: Shuo Zheng, Garrett Watamull, Nic Tejada
 *
 * created October 8, 2014  szheng
 */

#include <hardware.h>
#include <yalnix.h>
#include "delay.h"
#include "proc.h"
#include "trap.h"
#include "switch.h"
#include "util/list.h"
#include "std.h"
#include "frames.h"
#include "tty.h"

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
    case YALNIX_TTY_READ:
        DoTtyRead(context);
        break;
    case YALNIX_TTY_WRITE:
        DoTtyWrite(context);
        break;
	/*
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
    DelayUpdate();
    LoadNextProc(context, NO_BLOCK);
}

void IllegalHandler(UserContext *context) {
    TracePrintf(1, "Illegal Instruction. Killing Current Process.\n");
    current_process->status = KILL;
    KillProc(current_process);
    LoadNextProc(context, BLOCK);
}

void MathHandler(UserContext *context) {
    TracePrintf(1, "Math Error. Killing Current Process.\n");
    current_process->status = KILL;
    KillProc(current_process);
    LoadNextProc(context, BLOCK);
}

void MemoryHandler(UserContext *context) {
    TracePrintf(1, "Trap Memory\n");
    int newStackPage = (DOWN_TO_PAGE((int)context->addr - VMEM_1_BASE)>>PAGESHIFT);
    int sp = DOWN_TO_PAGE(current_process->user_context.sp - VMEM_1_BASE)>>PAGESHIFT;
    
    TracePrintf(2, "TrapMemory: New stack page = %d. Old stack = %d\n", newStackPage, sp);	
    TracePrintf(2, "TrapMemory: Context addr: %p\n", context->addr);
    switch (context->code) {
    case YALNIX_MAPERR:

	if (current_process->pageTable[newStackPage - 1].valid == 1) {
	    TracePrintf(1, "Memory Error: Attempt to extend stack too close to heap\n");

	    current_process->status = KILL;
	    KillProc(current_process);
	    LoadNextProc(context, BLOCK);
	}

	for (sp; sp <= newStackPage; sp++) {
	    current_process->pageTable[newStackPage].valid = 1;
	    current_process->pageTable[newStackPage].pfn = getNextFrame();
	    current_process->pageTable[newStackPage].prot = (PROT_READ | PROT_WRITE);
	}
        break;
    case YALNIX_ACCERR:
	TracePrintf(1, "Memory Error: Tried to access page without correct permissions\n");
        TracePrintf(1, "Memory Error: Killing Current Process\n");

        current_process->status = KILL;
        KillProc(current_process);
        LoadNextProc(context, BLOCK);

        break;
    }
}

void TtyReceiveHandler(UserContext *context) {
    // Pass into a buffer that holds input
    TracePrintf(1, "TtyReceiveHandler\n");
    int tty_id = context->code;
    TTY* tty = &(ttys[tty_id]);

    // Move everything on input line into tty's buffer in memory
    int readLen;
    void *buf;
    for (buf = (void*)malloc(READ_LEN); 
       	 readLen = TtyReceive(tty_id, buf, READ_LEN); 
         buf = (void*)malloc(READ_LEN)) { 
        Overflow *over = (Overflow *) malloc(sizeof(Overflow));
        over->addr = over->base = buf;
        over->len = readLen;
        tty->totalOverflowLen += readLen;
        queuePush(tty->overflow, over);
    }

	//get length that process at head of readBlocked queue wants
	//DOESN'T POP IF THERE IS A HEAD THAT HAS LENGTH OF OR TOTALOVERFLOW IS 0
    int len = 0;
    if (!queueIsEmpty(tty->readBlocked)) {
        PCB *reading_pcb = (PCB*)(tty->readBlocked->head->data);
        len = reading_pcb->user_context.regs[0];
    }

    if (len > 0 && tty->totalOverflowLen > 0) {
        ReadFromBuffer(*tty, reading_pcb->readBuf, len);
        queuePush(ready_queue, queuePop(tty->readBlocked));
    }
}

void TtyTransmitHandler(UserContext *context) {
    // Write out to Tty
	int tty_id = context->code;
    TTY* tty = &(ttys[tty_id]);
	TracePrintf(1, "TtyTransmitHandler: tty_id is %d and has %d left to transmit\n", tty_id, tty->lenLeftToWrite);
    if (tty->lenLeftToWrite == 0) {
		TracePrintf(3, "TtyTransmitHandler: Terminal %d finished writing for PCB->id: %d\n", tty_id, tty->writePCB->id);
		free(tty->writeBase);
		queuePush(ready_queue, tty->writePCB);
		tty->writePCB = NULL;
		tty->writeBuf = NULL;
		if(!queueIsEmpty(tty->writeBlocked)){
			PCB* nextPCB = (PCB*)queuePop(tty->writeBlocked);
			queuePush(ready_queue, (void*)nextPCB);
			tty->writePCB = nextPCB;
		}
	} else if (tty->lenLeftToWrite > TEST_LENGTH) {
		TracePrintf(3, "current user context pc is %p\n", context->pc);
		TtyTransmit(tty_id, tty->writeBuf, TEST_LENGTH);
		tty->writeBuf = tty->writeBuf + TEST_LENGTH;
		tty->lenLeftToWrite = tty->lenLeftToWrite - TEST_LENGTH;
	} else {

	    TtyTransmit(tty_id, tty->writeBuf, tty->lenLeftToWrite);
	    tty->lenLeftToWrite = 0;
	    
	}
	current_process->user_context = *context;
}



void DiskHandler(UserContext *context) {
}

void InvalidTrapHandler(UserContext *context) {
    context->regs[0] = ERROR;
    TracePrintf(1, "Invalid trap occurred from process %d with trap code %d\n",
                current_process->id, context->vector);
    KillProc(current_process);
    LoadNextProc(context, BLOCK);

}
