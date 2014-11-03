#include "tty.h"
#include "proc.h"

TTY ttys[NUM_TERMINALS];

void InitTTY() {
    for (int i = 0; i < NUM_TERMINALS; i++) {
        ttys[i].readBlocked = queueNew();
        ttys[i].writeBlocked = queueNew();
        ttys[i].overflow = queueNew();
        ttys[i].totalOverflowLen = 0;
    }
}

void DoTtyRead(UserContext *context) {
    int tty_id = context->regs[0];
    // TODO: Check tty_id is legal
    void *buf = context->regs[1];
    int len = context->regs[2];
    TTY tty = ttys[tty_id];

    // Return if trying to read zero bytes...?
    if (len == 0) {
        return;
    }

    if (tty.totalOverflowLen > 0) {
        ReadFromBuffer(tty, buf, len);
    } else {
        current_process->user_context = *context;
        queuePush(tty.readBlocked, current_process);
    }
}

void DoTtyWrite(UserContext *context) {
    int tty_id = context->regs[0];
    // TODO: Check tty_id is legal
    TTY tty = ttys[tty_id];
    current_process->user_context = *context;
    queuePush(tty.writeBlocked, current_process);
}

void ReadFromBuffer(TTY tty, void *buf, int len) {

}
