#include "tty.h"
#include "proc.h"
#include "input.h"
#include "std.h"

TTY ttys[NUM_TERMINALS];

void InitTTY() {
    for (int i = 0; i < NUM_TERMINALS; i++) {
        ttys[i].readBlocked = queueNew();
        ttys[i].writeBlocked = queueNew();
		ttys[i].writePCB = NULL;
        ttys[i].overflow = queueNew();
        ttys[i].totalOverflowLen = 0;
		ttys[i].lenLeftToWrite = 0;
		ttys[i].base = NULL;
		ttys[i].writeBuf = NULL;
    }
}

void DoTtyRead(UserContext *context) {
    int tty_id = context->regs[0];
    
    if (tty_id > NUM_TERMINALS || tty_id < 0) {
        TracePrintf(1, "DoTtyRead: tty_id: %d outside of acceptable range\n", tty_id);
		context->regs[0] = ERROR;
		return;
    } 
	
    current_process->readBuf = (void*)malloc(sizeof(context->regs[1]));
    int len = context->regs[2];
    TTY* tty = &(ttys[tty_id]);
	memset(current_process->readBuf, 0, len);

    // Return if trying to read zero bytes...?
    if (len == 0) {
        return;
    }

    if (tty->totalOverflowLen > 0) {
        ReadFromBuffer(tty, current_process->readBuf, len);
    } else {
        current_process->user_context = *context;
        queuePush(tty->readBlocked, current_process);
		LoadNextProc(context, BLOCK);
		TracePrintf(1, "DoTtyRead: back from readBlockQueue and have first char as %c and is located at %p\n", *((char*)(current_process->readBuf)), current_process->readBuf);	
	}
    	memcpy(context->regs[1], current_process->readBuf, len);
		free(current_process->readBuf);
}

void DoTtyWrite(UserContext *context) {
    int tty_id = context->regs[0];
    char *buf = (char*)(context->regs[1]);
	
    int len = context->regs[2];
    if (tty_id >= NUM_TERMINALS && tty_id < 0) {
		TracePrintf(1, "DoTtyWrite: tty_id: %d outside of acceptable range\n", tty_id);
		context->regs[0] = ERROR;
	return;
    } 
    /*else if (BufferReadCheck(buf, len) == ERROR) {
	TracePrintf(1, "DoTtyWrite: buffer given not valid. Killing current process.\n");
	KillProc(current_process);
	LoadNextProc(context, BLOCK);
    }*/
    TTY* tty = &(ttys[tty_id]); 

    if (tty->writePCB) { // Process currently writing to terminal. So block process.
		TracePrintf(1, "TtyWrite: tty.writePCB is not empty with address %p\n", tty->writePCB);
		queuePush(tty->writeBlocked, current_process);
		LoadNextProc(context, BLOCK); 
		TracePrintf(2, "DoTtyWrite: Back from LoadNextProc after being on writeBlocked queue.\n");

    } 
	
    tty->writeBase = tty->writeBuf = (char*)(malloc(len*sizeof(char)));
    memcpy(tty->writeBase, buf, context->regs[2]);
	TracePrintf(1, "TtyWrite: memcpy to writebase which has address of %p\n", tty->writeBase);
    /*if (len > TERMINAL_MAX_LINE) {
		TtyTransmit(tty_id, tty->writeBuf, TERMINAL_MAX_LINE);
		tty->writeBuf = tty->writeBuf + TERMINAL_MAX_LINE;
		tty->lenLeftToWrite = len - TERMINAL_MAX_LINE;
    } else { 
		tty->lenLeftToWrite = 0;
		TtyTransmit(tty_id, tty->writeBuf, len);
    	TracePrintf(1, "TtyWrite: nothing left to write after transmit\n");
	}*/
	if (len > TEST_LENGTH) {
		TracePrintf(3, "TtyWrite: write length is over MAX_LINE.\n");
		TtyTransmit(tty_id, tty->writeBuf, TEST_LENGTH);
		tty->writeBuf = tty->writeBuf + TEST_LENGTH;
		tty->lenLeftToWrite = len - TEST_LENGTH;
    } else { 
		tty->lenLeftToWrite = 0;
		TtyTransmit(tty_id, tty->writeBuf, len);
    	TracePrintf(1, "TtyWrite: nothing left to write after transmit\n");
	}

    current_process->user_context = *context;	
    tty->writePCB = current_process;
    LoadNextProc(context, BLOCK);

}

void ReadFromBuffer(TTY* tty, void *buf, int len) {
	Queue * overQueue = tty->overflow;
	char *lastWrite = (char*)buf; //pointer to end of last memcpy
	if(queueIsEmpty(overQueue)){
		TracePrintf(1, "ReadFromBuffer: Trying to read from empty buffer!\n");
		return;
	}
	int lenLeft = len;
	while(lenLeft > 0 && tty->totalOverflowLen>0){
		Overflow *over = (Overflow*)(overQueue->head->data);
		if((over->len)<=lenLeft){
			TracePrintf(1, "ReadFromBuffer: lenLeft is %d which is more than the length of the overflow head %d\n", lenLeft, over->len);
			memcpy(lastWrite, over->addr, over->len);
			lastWrite += over->len;
			TracePrintf(1, "ReadFromBuffer: lastWrite = %c\n", *(lastWrite-1));
			TracePrintf(1, "ReadFromBuffer: lastWrite + 1 = %u\n", *(lastWrite));
			lenLeft -= over->len;
			tty->totalOverflowLen -= over->len;
			free(over->base);
			free(queuePop(overQueue));	
		}
		else{
			TracePrintf(1, "ReadFromBuffer: lenLeft is %d which is less than the length of the overflow head %d\n", lenLeft, over->len);
			memcpy(lastWrite, over->addr, lenLeft);
			over->addr += lenLeft;
			over->len -= lenLeft;
			tty->totalOverflowLen -= lenLeft;
			lenLeft = 0;
		} 			
	}	
}
