/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * TTY implementations, DoTtyRead and DoTtyWrite
 *
 * Authors: Shuo Zheng, Garrett Watumull, Nic Tejada
 *
 * created October 8, 2014  szheng
 */

#ifndef _tty_h
#define _tty_h

#include <hardware.h>
#include "util/list.h"
#include "util/queue.h"
#include "proc.h"
#define READ_LEN 32

typedef struct overflow_buf_t {
    void *addr;
    int len;
} Overflow;

typedef struct tty_t  {
    Queue *readBlocked;     // Blocked queue waiting on TtyReceive
    Queue *writeBlocked;    // Blocked queue waiting to TtyTransmit
    PCB *writePCB;          // Current process writing to terminal
    Queue *overflow;        // Queue of overflow structs
    int totalOverflowLen;
    void *base;             // Base pointer to start of buffer queue
    char *writeBuf;      
    char *writeBase;        // Buff being read from for TtyWrite
    int lenLeftToWrite;
} TTY;

extern TTY ttys[];

extern void InitTTY(void);
extern void DoTtyRead(UserContext *);
extern void DoTtyWrite(UserContext *);
extern void ReadFromBuffer(TTY, void *, int);

#endif
