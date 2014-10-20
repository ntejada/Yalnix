#include <stdio.h>
#include <stdlib.h>
#include "./include/hardware.h"
#include "./include/yalnix.h"
#define SUCCESS 0
#define ERROR (-1) 
#define KILL 42

/*******************
 * QUEUE/STACK
 ***********************/
typedef struct PCB PCB;

typedef struct Node Node;

// Generalized node to be used by cvars, locks, pipes, and ready queues / child stacks
struct Node{
  Node *next;
  PCB *pcb;
  int lock_id; 
};


// Cvar, Lock, Ready
typedef struct {
	Node * head;
	Node * tail;
} Queue;

// Child processes.
typedef struct {
  void * head;
} Stack;

/*********************
 *  PCB
 **********************/

struct PCB{
  unsigned int state;
  unsigned int exit_state;
  unsigned int pid;
  unsigned int ppid;
  
  UserContext *user_context;
  KernelContext *kernel_context;

        struct pte * ptable_bp;
	int ptable_limit;
  Stack *child_queue;
  // Page Table
};

typedef struct {
  unsigned int exit_status;
  unsigned int pid;
} Z_CB; // Zombie Control Block

/**********************
 * SYNCHRONIZATION PRIMITIVES
 **********************/

typedef struct {
  Queue *cvar_queue;
  int cvar_id;
} C_Var;

typedef struct {
  unsigned int state;
  Queue *lock_queue;
  PCB *pcb; // Pointer to PCB that holds lock. Only valid if state == 1;
  int identifier;
} Lock;

typedef struct {
  char *buffer;
  int length_buffer;
  int identifier;
  Queue *pipe_queue;
} Pipe;

// Prototypes
Node *createNode(PCB *, int);
Queue *InitQueue();
int isQueueEmpty(Queue *);
int enqueue(Queue *, Node *);
void *dequeue(Queue *);
int dequeueAll(Queue *);

Stack *initStack();
int isStackEmpty(Stack *);
int push(Stack *, Node *);
void *pop(Stack *);	

