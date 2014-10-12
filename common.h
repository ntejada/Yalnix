#include <stdio.h>
#include <stdlib.h>

#define SUCCESS 1
#define ERROR 0

typedef struct {
	void * head;
	void * tail;
} Queue;


/**********************
 * SYNCHRONIZATION PRIMITIVES
 ********************* */

typedef struct {
  int *PCB;
  Cvar_Node* next;
  int *lock;
} Cvar_Node;

typedef struct {
  unsigned int state;
  Queue *lock_queue;
  PCB *pcb;
} Lock;

typdef struct {
  char *buffer;
  int length_buffer;
  int identifier;
} Pipe;

	
