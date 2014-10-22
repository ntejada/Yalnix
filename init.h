#ifndef _kernelInit_h
#define _kernelInit_h

extern int (*vector_table[TRAP_VECTOR_SIZE]) (UserContext *uctxt);

extern void *kernel_extent;
extern void *kernel_data_start;

#endif
