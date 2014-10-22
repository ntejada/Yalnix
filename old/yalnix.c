#include "../include/yalnix.h"
#include "../include/hardware.h"
#include "mem.h"
#include "proc.h"
#include "trap.h"
#include "std.h"


// Idle process.
void DoIdle(void) {
    for ever {
        TracePrintf(1, "DoIdle\n");
        Pause();
    }
} 

void SetKernelData(void *_KernelDataStart, void *_KernelDataEnd) {
    KernelDataStart = _KernelDataStart;
    KernelDataEnd = _KernelDataEnd;
    KernelBrk = _KernelDataEnd;
}

void KernelStart(char *args[], unsigned int pmemSize, UserContext *context) {
    InitTrapVector();
    GetFreeFrames(pmemSize);
    InitPageTables();
    // Enable virtual memory
    WriteRegister(REG_VM_ENABLE, 1);
    vmFlag = 1;

    // Cook program counter to idle code within kernel code.
    context->pc = &DoIdle;
    // Temporarily putting the stackpointer somewhere in Kernel stack.
    // will fix this later
    context->sp = (void *) (VMEM_0_LIMIT - 128);
}
