#include "input.h"
#include "proc.h"
#include "std.h"



int 
BufferCheck(void *buf, int len) {

    char *ptr = (char *) buf;
    for (int i = 0; i < len; i++, ptr++) {
		int page = DOWN_TO_PAGE(ptr-VMEM_1_BASE) >> PAGESHIFT;

		if (!(ptr >= VMEM_1_BASE && ptr <= VMEM_1_LIMIT)) {
		    TracePrintf(1, "BufferReadCheck: Address issued not in Region 1\n");
		    return ERROR;
		}

		if (current_process->cow.pageTable[page].prot == PROT_NONE || !current_process->cow.pageTable[page].valid) {
		    TracePrintf(1, "BufferReadCheck: Does not have read permissions for page %d\n", page);
		    return ERROR;
		}
    }
    return SUCCESS;
}


