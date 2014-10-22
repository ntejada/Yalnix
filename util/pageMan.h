#include "./hardware.h"

struct pte reg_zero_table[MAX_PT_LEN];

int addFrame(int);
int getNextFrame();
int availableFramesListInit(unsigned int pmem_size);
