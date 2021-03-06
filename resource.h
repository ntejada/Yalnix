/*
 * Yalnix for Linux/x86, COSC 058 Fall 2014
 *
 * Shared code for resource management, pipes, lock, cvar
 *
 * Authors: Shuo Zheng, Garrett Watumull, Nic Tejada
 */
#ifndef _resource_h_
#define _resource_h_

#include <hardware.h>
#include "util/queue.h"

#define NUM_RESOURCE_TYPES 4
#define LOCK 0
#define CVAR 1
#define SEM  2
#define PIPE 3

typedef struct resource_t {
    int id;
} Resource;

extern void InitResources(void);
extern void *GetResource(int);
extern void DoReclaim(UserContext *context);

#endif
