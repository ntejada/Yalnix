/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Authors: Shuo Zheng
 *
 * created October 8, 2014  szheng
 */

#ifndef _trap_h
#define _trap_h

#include "../include/yalnix.h"
#include "../include/hardware.h"

extern void *trapVector[];

/* Trap vector initializer and trap handlers */
extern void InitTrapVector(void);
extern void KernelCallHandler(UserContext *);
extern void ClockHandler(UserContext *);
extern void IllegalHandler(UserContext *);
extern void MemoryHandler(UserContext *);
extern void MathHandler(UserContext *);
extern void TtyReceiveHandler(UserContext *);
extern void TtyTransmitHandler(UserContext *);
extern void DiskHandler(UserContext *);
extern void InvalidTrapHandler(UserContext *);

#endif /*!_trap_h*/
