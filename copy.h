#ifndef _copy_h
#define _copy_h

#include <hardware.h>
#include <yalnix.h>
#include "util/queue.h"
#include "std.h"
#include "trap.h"
#include "proc.h"

extern int CopyStack(PCB *);
extern int CopyRegion1(PCB *);
extern int CoWRegion1(PCB *);
extern int CoW_PreserveGlobal(PCB *);
extern int CopyOnWrite(int, PCB *);

#endif
