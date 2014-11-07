#include "input.h"
#include "proc.h"
#include "std.h"



int 
BufferReadCheck(void *buf, int len) {

    char *ptr = (char *) buf;
    for (int i = 0; i < len; i++, ptr++) {
		int page = DOWN_TO_PAGE(ptr-VMEM_1_BASE) >> PAGESHIFT;
		// Has no read permissions
		char* buffer = (char*) buf;
		if (current_process->pageTable[page].prot == PROT_NONE || !current_process->pageTable[page].valid) {
		    TracePrintf(1, "BufferReadCheck: Does not have read permissions for page %d\n", page);
		    return ERROR;
		}
    }
    return SUCCESS;
}


int 
BufferWriteCheck(void *buf, int len) {

    char *ptr = (char *) buf;
    for (int i = 0; i < len; i++, ptr++) {
        int page = DOWN_TO_PAGE(ptr) >> PAGESHIFT;
        // Has no read permissions                                                                                                          
        if (current_process->pageTable[page].prot != PROT_READ | PROT_WRITE) {
            TracePrintf(1, "BufferReadCheck: Does not have write permission for page %d\n", page);
            return ERROR;
        }
    }
    return SUCCESS;
}
