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
    int pageNum = (DOWN_TO_PAGE((int)context->addr - VMEM_1_BASE)>>PAGESHIFT);
    int sp = DOWN_TO_PAGE(current_process->user_context.sp - VMEM_1_BASE)>>PAGESHIFT;

    TracePrintf(2, "TrapMemory: New stack page = %d. Old stack = %d\n", pageNum, sp);	
    TracePrintf(2, "TrapMemory: Context addr: %p\n", context->addr);
    switch (context->code) {
        case YALNIX_MAPERR:

            if (current_process->cow.pageTable[pageNum - 1].valid == 1) {
                TracePrintf(1, "Memory Error: Attempt to extend stack too close to heap\n");

                current_process->status = KILL;
                KillProc(current_process);
                LoadNextProc(context, BLOCK);
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
                // Go ahead and change permissions
                current_process->cow.pageTable[pageNum].prot = (PROT_READ|PROT_WRITE);
                // Check how many processes are referencing that frame
                if (current_process->cow.refCount[pageNum] > 1) {
                    // Copy over the page and decrement refCount
                    pZeroTable[PF_COPIER].valid = 1;
                    pZeroTable[PF_COPIER].prot = (PROT_READ|PROT_WRITE);

                    int newPfn = getNextFrame();
                    pZeroTable[PF_COPIER].pfn = newPfn;
                    WriteRegister(REG_TLB_FLUSH, PF_COPIER << PAGESHIFT);
                    memcpy(PF_COPIER << PAGESHIFT, VMEM_1_BASE + (pageNum << PAGESHIFT), PAGESIZE);
                    current_process->cow.pageTable[pageNum].pfn = newPfn;
                    *(current_process->cow.refCount[pageNum])--;

                    pZeroTable[PF_COPIER].prot = PROT_NONE;
                    pZeroTable[PF_COPIER].valid = 0;
                } else {
                    free(current_process->cow.refCount[pageNum]);
                }
            } else {
                // Otherwise it was truly a bad memory access.
                TracePrintf(1, "Memory Error: Tried to access page without correct permissions\n");
                TracePrintf(1, "Memory Error: Killing Current Process\n");

                current_process->status = KILL;
                KillProc(current_process);
                LoadNextProc(context, BLOCK);
            }

            break;
    }
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
    KillProc(current_process);
    LoadNextProc(context, BLOCK);

}
