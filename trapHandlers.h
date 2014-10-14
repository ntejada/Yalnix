/*
 * file: trapHandlers.h
 * authors: Nic Tejada and Garrett Watumull
 * descrip: header for the trapHandlers file
 */

#include "./include/hardware.h"

int kernelTrap(UserContext* uctxt);
int clockTrap(UserContext* uctxt);
int illegalTrap(UserContext* uctxt);
int memoryTrap(UserContext* uctxt);
int mathTrap(UserContext* uctxt);
int receiveTrap(UserContext* uctxt);
int transmitTrap(UserContext* uctxt);
int diskTrap(UserContext* uctxt);
int noTrap(UserContext* uctxt);
