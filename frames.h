/* 
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Bookkeeping for free frames and giving out and returning frames
 *
 * Authors: Nic Tejada, Garrett Watumull, Shuo Zheng
 *
 * created October 20, 2014
 */

#ifndef _frames_h
#define _frames_h
#include "./hardware.h"

#define PF_COPIER (DOWN_TO_PAGE(KERNEL_STACK_BASE - 1) >> PAGESHIFT)

extern int *frame_list;
extern int *frame_list_bp;
extern int number_of_frames;

int addFrame(int);
int getNextFrame();
int availableFramesListInit(unsigned int pmem_size);
#endif
