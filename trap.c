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
#include "init.h"
#include "tty.h"
#include "lock.h"
#include "cvar.h"
#include "pipe.h"

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
        case YALNIX_CUSTOM_0:
	    DoSpoon(context);
	    break;
	    */
        case YALNIX_CUSTOM_1:
	    DoPS(context);
	    break;
#ifdef LINUX
        case YALNIX_PIPE_INIT:
            DoPipeInit(context);
            break;
        case YALNIX_PIPE_READ:
            DoPipeRead(context);
            break;
        case YALNIX_PIPE_WRITE:
            DoPipeWrite(context);
            break;
        case YALNIX_LOCK_INIT:
            DoLockInit(context);
            break;
        case YALNIX_LOCK_ACQUIRE:
            DoLockAcquire(context);
            break;
        case YALNIX_LOCK_RELEASE:
            DoLockRelease(context);
            break;
        case YALNIX_CVAR_INIT:
            DoCvarInit(context);
            break;
        case YALNIX_CVAR_SIGNAL:
            DoCvarSignal(context);
            break;
        case YALNIX_CVAR_BROADCAST:
            DoCvarBroadcast(context);
            break;
        case YALNIX_CVAR_WAIT:
            DoCvarWait(context);
            break;
        case YALNIX_RECLAIM:
            DoReclaim(context);
            break;
#endif // LINUX
    }
}

void ClockHandler(UserContext *context) {
    TracePrintf(2, "In the ClockHandler\n");
    DelayUpdate();
    DelayPop();
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
    int pageNum = (DOWN_TO_PAGE((int)context->addr - VMEM_1_BASE)>>PAGESHIFT);
    int sp = DOWN_TO_PAGE(current_process->user_context.sp - VMEM_1_BASE)>>PAGESHIFT;
	TracePrintf(2, "TrapMemory: This page has the valid bit set to %d and %d prots and pfn %d\n", current_process->cow.pageTable[pageNum].valid, current_process->cow.pageTable[pageNum].prot, current_process->cow.pageTable[pageNum].pfn);
	TracePrintf(2, "TrapMemory: Read Write is %d and Read is %d\n", (PROT_READ | PROT_WRITE), PROT_READ);
	TracePrintf(2, "TrapMemory: MAPERR = %d, ACCERR = %d, this error = %d\n", YALNIX_MAPERR, YALNIX_ACCERR, context->code);
    TracePrintf(2, "TrapMemory: New stack pointer = %d. Old stack pointer = %d\n", pageNum, sp);	
    TracePrintf(2, "TrapMemory: Context addr: %p\n", context->addr);
    switch (context->code) {
        case YALNIX_MAPERR:

            if (current_process->cow.pageTable[pageNum - 1].valid == 1) {
                TracePrintf(1, "Memory Error: Attempt to extend stack too close to heap\n");

                current_process->status = KILL;
                KillProc(current_process);
                LoadNextProc(context, BLOCK);
            }
	    if (current_process->cow.refCount[pageNum] && 
                current_process->cow.pageTable[pageNum].prot == PROT_READ) {
    			TracePrintf(2, "TrapMemory: YALNIX_MAPERR - copyOnWrite\n");
				copyOnWrite(pageNum, current_process); 
				WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL); 
            	break;
			}
            for (sp; sp <= pageNum; sp++) {
                current_process->cow.pageTable[pageNum].valid = 1;
                current_process->cow.pageTable[pageNum].pfn = getNextFrame();
                current_process->cow.pageTable[pageNum].prot = (PROT_READ | PROT_WRITE);
            }
            break;
        case YALNIX_ACCERR:
            // Check first to see if it was a copy on write issue, ie. attempt to write to a CoW page
            if (current_process->cow.refCount[pageNum] && 
                current_process->cow.pageTable[pageNum].prot == PROT_READ) {
    		 TracePrintf(2, "TrapMemory: YALNIX_ACCERR- copyOnWrite\n");
            	 copyOnWrite(pageNum, current_process); 
            } 
			else {
                // Otherwise it was truly a bad memory access.
                TracePrintf(1, "Memory Error: Tried to access page without correct permissions\n");
                TracePrintf(1, "Memory Error: Killing Current Process\n");

                current_process->status = KILL;
                KillProc(current_process);
                LoadNextProc(context, BLOCK);
            }

            break;
       default:
	   if (current_process->cow.refCount[pageNum] &&
	       current_process->cow.pageTable[pageNum].prot == PROT_READ) {
	       TracePrintf(2, "TrapMemory: YALNIX_ACCERR- copyOnWrite\n");
	       copyOnWrite(pageNum, current_process);
	   }
	   break;
    }
    
	TracePrintf(1, "TrapMemory: pc is %p\n", context->pc);
	TracePrintf(1, "TrapMemory: exiting\n");
}

void TtyReceiveHandler(UserContext *context) {
    // Pass into a buffer that holds input
    TracePrintf(4, "TtyReceiveHandler\n");
    int tty_id = context->code;
    TTY* tty = &(ttys[tty_id]);
    PCB * reading_pcb;
    // Move everything on input line into tty's buffer in memory
    int readLen;
    // void *buf = (void *)malloc(READ_LEN*sizeof(char));
    char *buf = (char *)malloc(READ_LEN*sizeof(char));
    readLen = TtyReceive(tty_id, (void*)buf, READ_LEN); 
    TracePrintf(4, "TtyReceiveHandler: readLen is %d\n", readLen);
    int j;

    
    while(readLen>1) { 
        if(buf[readLen-1] == '\n'){
            TracePrintf(4, "TtyReceiveHandler: getting rid of new line character\n");
            buf[readLen-1]='\0';
            readLen--;
        }   
        Overflow *over = (Overflow *) malloc(sizeof(Overflow));
        over->addr = over->base = buf;
        for(j = 0; j < READ_LEN; j++){
            TracePrintf(4, "TtyReceiveHandler: %d char is %hu or char %c\n", j, buf[j], buf[j]);
        }
        over->len = readLen;
        tty->totalOverflowLen += readLen;
        queuePush(tty->overflow, over);
        buf = (char *)malloc(READ_LEN*sizeof(char));
        //memset(buf, 0, sizeof(buf));
        readLen = TtyReceive(tty_id, (void*)buf, READ_LEN); 
        TracePrintf(4, "TtyReceiveHandler: readLen is %d\n", readLen);
    }

    int len = 0;
    int overFlowLeft = tty->totalOverflowLen;
    int lenReturned;
    while(!queueIsEmpty(tty->readBlocked) && overFlowLeft > 0) {
        reading_pcb = (PCB*)(tty->readBlocked->head->data);
        reading_pcb->user_context.regs[0]=0;
		len = reading_pcb->user_context.regs[2];
        //memset(reading_pcb->readBuf, 0, sizeof(reading_pcb->readBuf));
        lenReturned = ReadFromBuffer(tty, reading_pcb->readBuf, len);
       	reading_pcb->user_context.regs[0] = lenReturned; 
		queuePush(ready_queue, queuePop(tty->readBlocked));
        overFlowLeft -= len;                       
    }    	
    TracePrintf(4, "TtyReceiveHandler: Exiting\n");
}

void TtyTransmitHandler(UserContext *context) {
    // Write out to Tty
    int tty_id = context->code;
    TTY* tty = &(ttys[tty_id]);
    TracePrintf(4, "TtyTransmitHandler: tty_id is %d and has %d left to transmit\n", tty_id, tty->lenLeftToWrite);
    if (tty->lenLeftToWrite == 0) {
        TracePrintf(4, "TtyTransmitHandler: Terminal %d finished writing for PCB->id: %d\n", tty_id, tty->writePCB->id);
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
        TracePrintf(4, "current user context pc is %p\n", context->pc);
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
    InvalidTrapHandler(context);

}

void InvalidTrapHandler(UserContext *context) {
    context->regs[0] = ERROR;
    TracePrintf(1, "Invalid trap occurred from process %d with trap code %d\n",
            current_process->id, context->vector);
    KillProc(current_process);
    LoadNextProc(context, BLOCK);

}



