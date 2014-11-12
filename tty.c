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
	TracePrintf(1, "DoTtyRead: called\n");
    int tty_id = context->regs[0];
    
    if (tty_id >= NUM_TERMINALS || tty_id < 0) {
        TracePrintf(1, "DoTtyRead: tty_id: %d outside of acceptable range\n", tty_id);
		context->regs[0] = ERROR;
		return;
    } 
	
    current_process->readBuf = (char*)malloc(sizeof(char)*context->regs[2]);
    int len = context->regs[2];
    TTY* tty = &(ttys[tty_id]);
	// Return if trying to read zero bytes...?
    if (len == 0) {
        return;
    }

    if (tty->totalOverflowLen > 0) {
        TracePrintf(2, "DoTtyRead: reading from buffer\n");
        ReadFromBuffer(tty, current_process->readBuf, len);
    } else {
        TracePrintf(1, "DoTtyRead: overflow is empty\n");
		current_process->user_context = *context;
        queuePush(tty->readBlocked, current_process);
		LoadNextProc(context, BLOCK);
	}
    strcpy(context->regs[1], current_process->readBuf);
	free(current_process->readBuf);
	TracePrintf(1, "DoTtyRead: exiting\n");
}

void DoTtyWrite(UserContext *context) {
	TracePrintf(1, "DoTtyWrite: called\n");
    int tty_id = context->regs[0];
    char *buf = (char*)(context->regs[1]);
	
    int len = context->regs[2];
    if (tty_id >= NUM_TERMINALS || tty_id < 0) {
		TracePrintf(1, "DoTtyWrite: tty_id: %d outside of acceptable range\n", tty_id);
		context->regs[0] = ERROR;
	return;
    } 
    else if (BufferReadCheck(buf, len) == ERROR) {
	TracePrintf(1, "DoTtyWrite: buffer given not valid. Killing current process.\n");
	KillProc(current_process);
	LoadNextProc(context, BLOCK);
    }
    TTY* tty = &(ttys[tty_id]); 

    if (tty->writePCB) { // Process currently writing to terminal. So block process.
		TracePrintf(1, "TtyWrite: tty.writePCB is not empty with address %p\n", tty->writePCB);
		queuePush(tty->writeBlocked, current_process);
		LoadNextProc(context, BLOCK); 
		TracePrintf(2, "DoTtyWrite: Back from LoadNextProc after being on writeBlocked queue.\n");

    } 
	
    tty->writeBase = tty->writeBuf = (char*)(malloc(len*sizeof(char)));
    strcpy(tty->writeBase, buf);
	TracePrintf(1, "TtyWrite: memcpy to writebase which has address of %p\n", tty->writeBase);
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
	TracePrintf(1, "DoTtyWrite: exiting\n");

}

int ReadFromBuffer(TTY* tty, char *buf, int len) {
	TracePrintf(1, "ReadFromBuffer: called\n");
	Queue * overQueue = tty->overflow;
	char *lastWrite = (char*)buf; //pointer to end of last memcpy
	int returnLen = 0;
	if(queueIsEmpty(overQueue)){
		TracePrintf(1, "ReadFromBuffer: Trying to read from empty buffer!\n");
		return;
	}
	int lenLeft = len;
	while(lenLeft > 0 && tty->totalOverflowLen>0){
		Overflow *over = (Overflow*)(overQueue->head->data);
		if((over->len)<=lenLeft){
			TracePrintf(1, "ReadFromBuffer: lenLeft is %d which is more than the length of the overflow head %d\n", lenLeft, over->len);
			strcpy(lastWrite, over->addr);
			lastWrite += over->len;
			lenLeft -= over->len;
			TracePrintf(1, "returnLen is %d\n", returnLen);	
			returnLen += over->len;
			TracePrintf(1, "returnLen is %d\n", returnLen);	
			tty->totalOverflowLen -= over->len;
			free(over->base);
			free(queuePop(overQueue));	
		}
		else{
			TracePrintf(1, "ReadFromBuffer: lenLeft is %d which is less than the length of the overflow head %d\n", lenLeft, over->len);
			strcpy(lastWrite, over->addr);
			over->addr += lenLeft;
			over->len -= lenLeft;
			returnLen += lenLeft;
			tty->totalOverflowLen -= lenLeft;
			lenLeft = 0;
		} 			
	}
	TracePrintf(1, "returnLen is %d\n", returnLen);	
	TracePrintf(1, "ReadFromBuffer: exiting\n");
	return returnLen;
}
