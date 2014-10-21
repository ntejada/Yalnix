#ifndef _common_h
#define _common_h
#include <stdio.h>
#include <stdlib.h>
#include "./include/hardware.h"
#include "./include/yalnix.h"
#define SUCCESS 0
#define ERROR (-1) 
#define KILL 42

//PCB MACROS
#define NEW_PCB (-2)


/**********************
 * GLOBALS 
 **********************/

int pidCount;
typedef struct Queue Queue;
typedef struct PCB PCB;
int (*vector_table[TRAP_VECTOR_SIZE]) (UserContext *uctxt);

void *kernel_extent;
void *kernel_data_start;

PCB *current_process;

//base pointer to region zero page table
struct pte pZeroTable[MAX_PT_LEN];
struct pte *pOneTable;

int *frame_list;
int *frame_list_bp;
int number_of_frames;

Queue *ready_queue;

/*******************
 * QUEUE/STACK
 ***********************/

typedef struct Node Node;


// Cvar, Lock, Ready
struct Queue{
	Node * head;
	Node * tail;
};

// Child processes.
typedef struct {
  Node * head;
} Stack;

// Delay blocked queue
// Instantiate new structure since will be handling different nodes 


/*********************
 *  PCB
 **********************/

typedef struct {
  unsigned int state;
  unsigned int exit_state;
  unsigned int pid;
  unsigned int ppid;
  
  UserContext user_context;
  KernelContext kernel_context;

  struct pte reg_one_table[MAX_PT_LEN];
  int ptable_limit;

  Stack *child_queue;
  // Page Table
} PCB;

typedef struct {
  unsigned int exit_status;
  unsigned int pid;
} Z_CB; // Zombie Control Block


struct Node {
  Node *next;
  void *data; // Void pointer to different nodes
};

typedef struct {
  int pid;
  int clock_count;
} DelayData;

typedef struct {
  int pid;
  int lock_id;
} CvarData;

typedef struct {
  int pid;
} PidData;


/**********************
 * SYNCHRONIZATION PRIMITIVES
 **********************/

// C_Var
typedef struct {
  Queue *cvar_queue;
  int cvar_id;
} C_Var;


// Lock
typedef struct {
  unsigned int state;
  Queue *lock_queue;
//  (void *) pcb; // Pointer to PCB that holds lock. Only valid if state == 1;
  int identifier;
} Lock;

// Pipe
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
#endif
