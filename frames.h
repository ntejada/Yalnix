#ifndef _frames_h
#define _frames_h

#include "./hardware.h"

extern int *frame_list;
extern int *frame_list_bp;
extern int number_of_frames;



int addFrame(int);
int getNextFrame();
int availableFramesListInit(unsigned int pmem_size);

#endif
