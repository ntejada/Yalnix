#ifndef _switch_h
#define _switch_h

#include <hardware.h>

extern KernelContext *MyKCS(KernelContext *, void *, void *);
extern KernelContext *FirstSwitch(KernelContext *, void *, void *);
extern KernelContext *ForkKernel(KernelContext *, void *, void *);
extern KernelContext *SpoonKernel(KernelContext *, void *, void *);


#endif
