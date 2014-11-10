/*
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Shared code for resource management, pipes, lock, cvar
 *
 * Authors: Shuo Zheng, Garrett Watumull, Nic Tejada
 */
#ifndef _resource_h_
#define _resource_h_

#include "util/queue.h"

#define NUM_RESOURCE_TYPES 3
#define LOCK 0
#define CVAR 1
#define PIPE 2

typedef struct resource_t {
    int id;
} Resource;

extern void *GetResource(Queue *, int);
extern void DoReclaim(UserContext *context);

#endif
