#ifndef _switch_h
#define _switch_h

#include "./include/hardware.h"

extern KernelContext *MyKCS(KernelContext *, void *, void *);
extern KernelContext *ForkKernel(KernelContext *, void *, void *);

#endif
