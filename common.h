#include <stdio.h>
#include <stdlib.h>
#include "./include/hardware.h"
#include "./include/yalnix.h"
#define SUCCESS 0
#define ERROR (-1) 
#define KILL 42

/**********************
 * GLOBALS 
 **********************/

int (*vector_table[TRAP_VECTOR_SIZE]) (UserContext *uctxt);

int vmem_on = 0;
void *kernel_extent;
void *kernel_data_start;

PCB *current_process;

struct *pte reg_zero_table;
struct *pte reg_one_table;

int *frame_list;
int *frame_list_bp;
int number_of_frames;





/*******************
 * QUEUE/STACK
 ***********************/
typedef struct PCB PCB;

typedef struct Node Node;


// Cvar, Lock, Ready
typedef struct {
	Node * head;
	Node * tail;
} Queue;

// Child processes.
typedef struct {
  Node * head;
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

  struct pte *kernel_stack_1;
  struct pte *kernel_stack_2;

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

struct Node {
  Node *next;
  (void *) pcb;
  int lock_id; 
} Node;


typedef struct {
  Queue *cvar_queue;
  int cvar_id;
} C_Var;

typedef struct {
  unsigned int state;
  Queue *lock_queue;
  (void *) pcb; // Pointer to PCB that holds lock. Only valid if state == 1;
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

