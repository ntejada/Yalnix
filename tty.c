#include "tty.h"
#include "proc.h"

TTY tty[NUM_TERMINALS];

void InitTTY() {
    for (int i = 0; i < NUM_TERMINALS; i++) {
        tty[i].readBlocked = queueNew();
        tty[i].writeBlocked = queueNew();
        tty[i].overflow = queueNew();
        tty[i].totalOverflowLen = 0;
    }
}

void DoTtyRead(UserContext *context) {
    int tty_id = context->regs[0];
    
    if (tty_id < NUM_TERMINALS) {
        TracePrintf(1, "DoTtyWrite: tty_id: %d outside of acceptable range\n", tty_id);
	context->regs[0] = ERROR;
	return;

    } else {

	queuePush(tty[tty_id].readBlocked, current_process);
	LoadNextProc(context, BLOCK);
    }
}

void DoTtyWrite(UserContext *context) {
    int tty_id = context->regs[0];
    if (tty_id >= NUM_TERMINALS && tty_id < 0) {
	TracePrintf(1, "DoTtyWrite: tty_id: %d outside of acceptable range\n", tty_id);
	context->regs[0] = ERROR;
	return;
    } else {
	queuePush(tty[tty_id].writeBlocked, current_process);
	LoadNextProc(context, BLOCK);
    }
}
