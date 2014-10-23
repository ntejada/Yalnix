 void
 KernelStart(char * cmd_args[], unsigned int pmem_size, UserContext *uctxt)
 {

   PCB *idlePCB = (PCB*)malloc(sizeof(PCB));
   memset(idlePCB, 0, sizeof(PCB));

   InitTrapVector();
   availableFramesListInit(pmem_size);
   PCB_Init(idlePCB);
   PageTableInit(idlePCB);


   pidCount = 0;
   // Cook things so idle process will begin running after return to userland.                                      
   // Enable virtual memory.                                                                                           WriteRegister(REG_VM_ENABLE, 1);
   WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);
   vmem_on = 1;

   // Initialize Queues                                                                                             
   ready_queue = queueNew();
   delay_queue = listAllocate();
   //set up idlePCB                                                                                                 

   char* args[3];

   idlePCB->status = NEW;
   idlePCB->id = pidCount++;
   idlePCB->kStackPages[0] = pZeroTable[KERNEL_STACK_BASE>>PAGESHIFT].pfn;
   idlePCB->kStackPages[1] =  pZeroTable[(KERNEL_STACK_BASE>>PAGESHIFT)+1].pfn;


   PCB *initPCB = (PCB*)malloc(sizeof(PCB));
   memset(initPCB, 0, sizeof(PCB));
   PCB_Init(initPCB);
   initPCB->id = pidCount++;
   initPCB->status = NEW;

   args[0]="1";
   args[2]=NULL;

   queuePush(ready_queue, initPCB);
   current_process = idlePCB;
   
   CopyStack(initPCB);
   // Put same kernel context into both PCB's. 
   KernelContextSwitch(MyKCS, (void *) initPCB, (void *) idlePCB);

   if (current_process->id == 0) {
     args[1] = "idle";

     int rc = LoadProgram("./initIdle", args, idlePCB);
     TracePrintf(1, "rc: %d, idlePCB pc: %d\n", rc, idlePCB->user_context.pc);

     *uctxt = idlePCB->user_context;

     return;

   } else {
     args[1]="init";

     rc = LoadProgram("./initIdle", args, initPCB);
     TracePrintf(1, "rc: %d, initPCB pc: %d\n", rc, initPCB->user_context.pc);

     *uctxt = initPCB->user_context;
     
     return;
   }
 }

KernelContext *MyKCS(KernelContext *kc_in, void *p_curr_pcb, void *p_next_pcb)
{
  PCB *curr = (PCB *) p_curr_pcb;
  PCB *next = (PCB *) p_next_pcb;

  TracePrintf(1, "MyKCS called. Will switch processes and reassign page table entries\n");

  // For KernelStart to execute
  if (curr->status == NEW && next->status == NEW) {
    curr->status = RUNNING;
    next->status = RUNNING;

    curr->kernel_context = *kc_in;
    next->kernel_context = *kc_in;

    return kc_in;
  }

  curr->kernel_context = *kc_in;

  if (p_curr_pcb != p_next_pcb) {

    for (int vpn = KERNEL_STACK_BASE >> PAGESHIFT, ki = 0;
	 vpn < DOWN_TO_PAGE(KERNEL_STACK_LIMIT) >> PAGESHIFT;
	 vpn++, ki++) {
      pZeroTable[vpn].pfn = next->kStackPages[ki];
    }

    WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_ALL);

    return &(next->kernel_context);
  } else {
    return kc_in;
  }
}

