#ifndef _map_h
#define _map_h

#include <stdio.h>
#include <stdlib.h>
#include "./common.h"

#define HASH_SIZE 100

typedef struct _MapData MapData;

MapData *kernel_stack_table[HASH_SIZE];



struct _MapData {
  MapData *next;
  int pid;
  void *kernelsp_1;
  void *kernelsp_2;
};

int hashInit(MapData **);
int hash(int);
MapData *accessMap(int);
int insertMap(MapData *);
int removeFromMap(int);
MapData *createMapData(int, void *, void*);

#endif 


