#include "./include/hardware.h"


KernelContext *MyKCS(KernelContext *kc_in, void *curr_pcb_p, void *next_pcb_p) 
{
  curr_pcb_p->KernelContext = kc_in;

  // Change kernel stack entries
  reg_one_table = next_pcb_p->ptable_bp;
  WriteRegister(REG_TLB_FLUSH, TLB_FLUSH_0);

  return next_pcb_p->KernelContext;

}
