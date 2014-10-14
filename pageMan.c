



int 
getNextFrame() 
{
  if (0 == number_of_frames) {
    TracePrintf("No available frames, getNextFrame\n");
    return ERROR;
  }
  
  int available_frame = frame_list[0];
  frame_list = &frame_list[1]; // Move array pointer.
  number_of_frames--;
 
  return available_frame;
}

int
addFrame(int frame)
{
  if (frame_list == frame_list_bp) {
    TracePrintf("Trying to add frame to full frame list, addFrame\n");
    return ERROR;
  }

  frame_list = frame_list-1;
  frame_list[0] = frame;
  number_of_frames++;

  return SUCCESS;
}



// Store frame list in kernel heap.                                                                                                                                                                                                          
int
availableFramesListInit()
{
  int top_frame = (PMEM_LIMIT-PMEM_BASE) / PAGESIZE;
  int first_frame = VMEM_0_LIMIT >> PAGESHIFT;

  number_of_frames = top_frame - first_frame;

  frame_list  = (int *)malloc(number_of_frames * sizeof(int));
  frame_list_bp = frame_list;

  if (frame_list == NULL) {
    TracePrintf(1, "Malloc error, availableFramesListInit\n");
    return ERROR;
  }

  int i;
  for (i = 0; i < number_of_frames; i++)
    frame_list[i] = i + first_frame; // Mark all as available                                                                                                                                                                                 
  return SUCCESS;
}
