/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Delays implemenation methods.
 *
 * Authors: Shuo Zheng, Garrett Watumull, Nic Tejada
 *
 * created October 23, 2014 
 */

#ifndef _delay_h
#define _delay_h

#include "proc.h"

void DelayUpdate(void);
void DelayAdd(PCB *);
PCB * DelayPop(void);

#endif
