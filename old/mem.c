#include "../include/hardware.h"
#include "../include/yalnix.h"
#include "util/queue.h"
#include "mem.h"

// Pointers to store necessary information when SetKernelData is called.
void *KernelDataStart, *KernelDataEnd, *KernelBrk;

struct pte pageTable0[MAX_PT_LEN];  // Region 0 page table
struct pte pageTable1[MAX_PT_LEN];  // Region 1 page table, temporary for checkpoint 2
Queue *freeFrames;                  // List of free frames
int vmFlag = 0;

void GetFreeFrames(unsigned int pmemSize) {
    freeFrames = queueNew();
    int pfn;
    for (pfn = UP_TO_PAGE(KernelDataEnd) >> PAGESHIFT;
         pfn < DOWN_TO_PAGE(pmemSize) >> PAGESHIFT;
         pfn++) {
        queuePush(freeFrames, (void *)pfn);
    }
    // TODO need to remove pfn 126 and 127 from free list
    // were allocated to first kernel stack
    for (pfn = (int)queuePop(freeFrames);
         pfn < DOWN_TO_PAGE(KernelBrk) >> PAGESHIFT;
         pfn = (int)queuePop(freeFrames));
}

void InitPageTables() {
    /* 
     * Initialize region 0 page table. Map currently used virtual pages
     * to the same address in physical memory frames.
     */
    int vpn;        // A virtual page number to work with.
    
    // Initialize the kernel instruction virtual memory pages.
    for (vpn = DOWN_TO_PAGE(PMEM_BASE) >> PAGESHIFT; 
         vpn < DOWN_TO_PAGE(KernelDataStart) >> PAGESHIFT;
         vpn++) {
        pageTable0[vpn].valid = 1;
        pageTable0[vpn].prot = (PROT_READ|PROT_EXEC);
        pageTable0[vpn].pfn = vpn;
    }
    // Initialize the kernel global data virtual memory pages.
    for (vpn = DOWN_TO_PAGE(KernelDataStart) >> PAGESHIFT;
         vpn < UP_TO_PAGE(KernelDataEnd) >> PAGESHIFT;
         vpn++) {
        pageTable0[vpn].valid = 1;
        pageTable0[vpn].prot = (PROT_READ|PROT_WRITE);
        pageTable0[vpn].pfn = vpn;
    }

    // As well as stack!
    for (vpn = DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT;
         vpn < UP_TO_PAGE(KERNEL_STACK_LIMIT) >> PAGESHIFT;
         vpn++) {
        pageTable0[vpn].valid = 1;
        pageTable0[vpn].prot = (PROT_READ|PROT_WRITE);
        pageTable0[vpn].pfn = vpn;
    }

    // Initialize the rest of the page table entries to invalid.
    for (vpn = UP_TO_PAGE(KernelDataEnd) >> PAGESHIFT;
         vpn < DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT;
         vpn++) {
        pageTable0[vpn].valid = 0;
        pageTable0[vpn].prot = PROT_NONE;
    }

    // Need to set up first region 1 page table for first process.
    // Temporarily just using one for the checkpoint.
    for (vpn = 0; vpn < MAX_PT_LEN; vpn++) {
        pageTable1[vpn].valid = 0;
        pageTable1[vpn].prot = PROT_NONE;
    }

    TracePrintf(2, "PMEM_BASE=0x%du\n", PMEM_BASE);
    TracePrintf(2, "KernelDataStart=%u, page=%u\n", KernelDataStart, 
            DOWN_TO_PAGE(KernelDataStart) >> PAGESHIFT);
    TracePrintf(2, "KernelDataEnd=%u, page=%u\n", KernelDataEnd, 
            DOWN_TO_PAGE(KernelDataEnd) >> PAGESHIFT);
    TracePrintf(2, "KernelBrk=%u, page=%u\n", KernelBrk, 
            DOWN_TO_PAGE(KernelBrk) >> PAGESHIFT);
    TracePrintf(2, "KERNEL_STACK_BASE=%u, page=%u\n", KERNEL_STACK_BASE, 
            DOWN_TO_PAGE(KERNEL_STACK_BASE) >> PAGESHIFT);
    TracePrintf(2, "KERNEL_STACK_LIMIT=%u, page=%u\n", KERNEL_STACK_LIMIT, 
            DOWN_TO_PAGE(KERNEL_STACK_LIMIT) >> PAGESHIFT);

    for (vpn = 0; vpn < MAX_PT_LEN; vpn++)
        TracePrintf(2, "Virtual page %3d mapped to Physical frame %3d, is valid %d\n",
                vpn, pageTable0[vpn].pfn, pageTable0[vpn].valid);

    WriteRegister(REG_PTBR0, (unsigned int)pageTable0);
    WriteRegister(REG_PTLR0, MAX_PT_LEN);
    WriteRegister(REG_PTBR1, (unsigned int)pageTable1);
    WriteRegister(REG_PTLR1, MAX_PT_LEN);
}

void Map(int vpn, struct pte pageTable[]) {
    pageTable[vpn].pfn = (int) queuePop(freeFrames);
}

void UnMap(int vpn, struct pte pageTable[]) {
    pageTable[vpn].valid = 0;
    queuePush(freeFrames, (void *)pageTable[vpn].pfn);
}


void TLBFlushAll(){
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
}

void TLBFlush0(){
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);
}

void TLBFlush1(){
    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_1);
}

void TLBFlushAddr(void *addr) {
    WriteRegister(REG_TLB_FLUSH, (unsigned int)addr);
}

int SetKernelBrk(void *addr) {
    TracePrintf(2, "NEW ADDR=%u, KernelBrk=%u\n", addr, KernelBrk);
    if ((unsigned int) addr > (unsigned int) KernelBrk)
        KernelBrk = addr;
    if (vmFlag) {
/* 
 If !VM:
  If Curr_max_addr<(int *)addr
  Curr_max_addr=addr
  Return Curr_max_addr
  Else:
  for(i=VMEM_BASE; i<addr,i++)
    if page_table[i] is not valid
    try:
    allocate new frame for page_table[i]
    page_table[i]=1 1 1 0 (4 random bits) (New frame address)
    remove paddr from linked list 
    except:
    return -1
      for(i=addr; i<kernel_stack_base-kernel_stack_max_size; i++)
	if page_table[i] is valid
      make invalid
      add paddr to linked list
      return 0
*/      
    } 
    return 0;
}
