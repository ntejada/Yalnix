#include "util/queue.h"

extern void *KernelDataStart, *KernelDataEnd, *KernelBrk;
extern struct pte pageTable0[];
extern Queue *freeFrames;
extern int vmFlag;

void InitPageTables(void);
void GetFreeFrames(unsigned int);
void InitPageTables(void);
void Map(int, struct pte[]);
void UnMap(int, struct pte[]);
void TLBFlushAll(void);
void TLBFlush0(void);
void TLBFlush1(void);
void TLBFlushAddr(void *);
int SetKernelBrk(void *);
