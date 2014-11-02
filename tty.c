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
    // TODO: Check tty_id is legal
    current_process->user_context = *context;
    
    queuePush(tty[tty_id].readBlocked, current_process);
}

void DoTtyWrite(UserContext *context) {
    int tty_id = context->regs[0];
    // TODO: Check tty_id is legal
    current_process->user_context = *context;
    
    queuePush(tty[tty_id].writeBlocked, current_process);
}
