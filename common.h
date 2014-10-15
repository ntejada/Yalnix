#include <stdio.h>
#include <stdlib.h>
#include "./include/hardware.h"
#define SUCCESS 0
#define ERROR (-1) 

/*******************
 * QUEUE/STACK
 ***********************/

// Cvar, Lock, Ready, 
typedef struct {
	void * head;
	void * tail;
} Queue;

// Child processes.
typedef struct {
  void * head;
} Stack;

/*********************
 *  PCB
 **********************/
typedef struct {
  unsigned int state;
  unsigned int exit_state;
  unsigned int pid;
  unsigned int ppid;
  
  UserContext *user_context;
  KernelContext *kernel_context;

	pte * ptable_bp;
	int ptable_limit;
  Stack *child_queue;
  // Page Table
} PCB;

typedef struct {
  unsigned int exit_status;
  unsigned int pid;
} Z_CB; // Zombie Control Block

/**********************
 * SYNCHRONIZATION PRIMITIVES
 **********************/


typedef struct {
  struct Cvar_Node *next;
  PCB *pcb;
  struct Lock *lock;
} Cvar_Node;

typedef struct {
  struct Lock_Node *next;
  PCB *pcb;
} Lock_Node;

typedef struct {
  unsigned int state;
  Queue *lock_queue;
  PCB *pcb;
} Lock;

typedef struct {
  char *buffer;
  int length_buffer;
  int identifier;
} Pipe;

// Prototypes
Cvar_Node *createCvarNode(PCB *, Lock *);
Queue *InitQueue();
int isQueueEmpty(Queue *);
int enqueue(Queue *, void *);
void *dequeue(Queue *);
int dequeueAll(Queue *);

Stack *initStack();
int isStackEmpty(Stack *);
int push(Stack *, void *);
void *pop(Stack *);	

