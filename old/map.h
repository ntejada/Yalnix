#ifndef _map_h
#define _map_h

#include "std.h"

#define HASH_SIZE 100

typedef struct _HashMap HashMap;

int hashInit(MapData **);
int hash(int);
MapData *accessMap(int);
int insertMap(MapData *);
int removeFromMap(int);
MapData *createMapData(int, void *, void *);
int addToMap(int, void *, void *);
#endif 


